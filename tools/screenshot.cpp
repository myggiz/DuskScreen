/*
 * Copyright (C) Christian Kaiser
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QFileDialog>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include <QScreen>
#include <QStringBuilder>
#include <QDebug>

#include <tools/screenshot.h>
#include <tools/screenshotmanager.h>
#include <tools/windowpicker.h>
#include <dialogs/areadialog.h>

#include <tools/os.h>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

#ifdef Q_OS_LINUX
    #include <QtGui/qguiapplication_platform.h>
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #undef Success
#endif

Screenshot::Screenshot(QObject *parent, Screenshot::Options options):
    QObject(parent),
    mOptions(std::move(options)),
    mPixmapDelay(false),
    mCancelled(false)
{
    if (mOptions.format == Screenshot::PNG) {
        // For PNG the "quality" argument is a compression level, not a quality
        // level, and -1 asks the writer for its default. Forcing 80 produced
        // barely-compressed files, while the options dialog disables the slider
        // for PNG on the grounds that it does not apply.
        mOptions.quality = -1;
    }
}

QString Screenshot::getName(const NamingOptions &options, const QString &prefix, const QDir &directory)
{
    QString naming;
    int naming_largest = 0;

    if (options.flip) {
        naming = "%1" % prefix;
    } else {
        naming = prefix % "%1";
    }

    switch (options.naming) {
    case Screenshot::Numeric: // Numeric
        // Iterating through the folder to find the largest numeric naming.
        for (auto file : directory.entryList(QDir::Files)) {
            if (file.contains(prefix)) {
                file.chop(file.size() - file.lastIndexOf("."));
                file.remove(prefix);

                if (file.toInt() > naming_largest) {
                    naming_largest = file.toInt();
                }
            }
        }

        if (options.leadingZeros > 0) {
            //Pretty, huh?
            QString format;
            QTextStream(&format) << "%0" << (options.leadingZeros + 1) << "d";

            naming = naming.arg(QString::asprintf(format.toLatin1(), naming_largest + 1));
        } else {
            naming = naming.arg(naming_largest + 1);
        }
        break;
    case  Screenshot::Date: // Date
        naming = naming.arg(QLocale().toString(QDateTime::currentDateTime(), options.dateFormat));
        break;
    case  Screenshot::Timestamp: // Timestamp
        naming = naming.arg(QDateTime::currentDateTime().toSecsSinceEpoch());
        break;
    case  Screenshot::Empty:
        naming = naming.arg("");
        break;
    }

    return naming;
}

const Screenshot::Options &Screenshot::options()
{
    return mOptions;
}

QPixmap &Screenshot::pixmap()
{
    return mPixmap;
}

//

void Screenshot::confirm(bool result)
{
    if (result) {
        // save() may finish on another thread; it emits cleanup() itself once
        // the result is known, so nothing may follow it here.
        save();
        return;
    }

    mOptions.result = Screenshot::Cancel;
    emit finished();
    emit cleanup();

    mPixmap = QPixmap();
}

void Screenshot::discard()
{
    confirm(false);
}

void Screenshot::optimize()
{
    QProcess *process = new QProcess(this);

    // Delete the QProcess once it's done.
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this   , SLOT(optimizationDone()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));

    QString optiPNG;

#ifdef Q_OS_UNIX
    optiPNG = QStandardPaths::findExecutable("optipng");
#else
    optiPNG = qApp->applicationDirPath() % QDir::separator() % "optipng.exe";
#endif

    if (optiPNG.isEmpty() || !QFile::exists(optiPNG)) {
        optimizationDone();
        process->deleteLater();
        return;
    }

    process->start(optiPNG, QStringList() << mOptions.fileName);

    if (process->state() == QProcess::NotRunning) {
        optimizationDone();
        process->deleteLater();
    }
}

void Screenshot::optimizationDone()
{
    emit finished();
}

void Screenshot::save()
{
    QString name = "";
    QString fileName = "";
    Screenshot::Result result = Screenshot::Failure;

    if (mOptions.file && !mOptions.saveAs)  {
        name = newFileName();
    } else if (mOptions.file && mOptions.saveAs) {
        name = QFileDialog::getSaveFileName(nullptr, tr("Save as.."), newFileName(), "*" % extension());

        // The native dialog appends the filter's extension to the suggested
        // (extension-less) name. Everything below works on a bare name — the
        // duplicate-name scan and the final append — so strip it back off
        // instead of ending up with "screenshot.1.png.png".
        if (name.endsWith(extension(), Qt::CaseInsensitive)) {
            name.chop(extension().size());
        }
    }

    if (!mOptions.replace && QFile::exists(name % extension())) {
        // Probe for the first free " (n)" instead of listing the directory and
        // parsing names back apart. The old string surgery stripped the base
        // name and then read whatever digits remained, so an unrelated file that
        // merely started with the same text — "shot.7.png" beside "shot." — was
        // read as suffix 7 and pushed the next capture to " (8)".
        int count = 1;

        while (QFile::exists(name % " (" % QString::number(count) % ")" % extension())) {
            ++count;
        }

        name = name % " (" % QString::number(count) % ")";
    }

    if (mOptions.clipboard) {
        QApplication::clipboard()->setPixmap(mPixmap, QClipboard::Clipboard);

        if (!mOptions.file) {
            result = Screenshot::Success;
        }
    }

    if (!mOptions.file) {
        saveFinished(result, fileName);
        return;
    }

    if (name.isEmpty()) {
        // Save As dismissed: leave fileName empty rather than reporting a
        // bare ".png", which the optimize step below would then run on.
        saveFinished(Screenshot::Cancel, fileName);
        return;
    }

    fileName = name % extension();

    // Claim the name before handing the encode off. The duplicate-name probe
    // above only sees files that exist, and encoding no longer creates the file
    // before the next capture resolves its own name — so without this, every
    // capture started inside one encode window picks the same name and they
    // overwrite each other. Creating it empty is enough: the probe and the
    // numeric scan both work on existence.
    QFile reservation(fileName);

    if (!reservation.open(QIODevice::WriteOnly)) {
        saveFinished(Screenshot::Failure, fileName);
        return;
    }

    reservation.close();

    // Encoding a full-desktop capture costs tens of milliseconds and used to
    // run here, on the UI thread, freezing the application until it finished.
    // QImage rather than QPixmap because a pixmap may not be touched outside
    // the GUI thread; both share their data, so this is not an extra copy of
    // the image — QPixmap::save() converts internally anyway.
    const QImage image = mPixmap.toImage();
    const int quality = mOptions.quality;

    auto *watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, fileName]() {
        const bool ok = watcher->result();
        watcher->deleteLater();
        saveFinished(ok ? Screenshot::Success : Screenshot::Failure, fileName);
    });

    watcher->setFuture(QtConcurrent::run([image, fileName, quality]() {
        return image.save(fileName, nullptr, quality);
    }));
}

void Screenshot::saveFinished(Result result, const QString &fileName)
{
    mOptions.fileName = fileName;
    mOptions.result   = result;

    if (!mOptions.result) {
        // Take the reserved name back out of the way, so a failed capture does
        // not leave an empty file behind holding a number.
        if (!fileName.isEmpty()) {
            QFile::remove(fileName);
        }

        // Failure: finish here. Falling through emitted finished() a second
        // time, or started optimize() on a file that was never written.
        emit finished();
    } else if (mOptions.format == Screenshot::PNG && mOptions.optimize && mOptions.file) {
        optimize();
    } else {
        emit finished();
    }

    // Deferred until the result is known: the window restore and the
    // notification both read options.result, and the notification names the
    // file that was written.
    emit cleanup();

    mPixmap = QPixmap();
}

void Screenshot::setPixmap(const QPixmap &pixmap)
{
    mPixmap = pixmap;

    if (mPixmap.isNull()) {
        confirm(false);
    } else {
        confirm(true);
    }
}

void Screenshot::take()
{
    switch (mOptions.mode) {
    case Screenshot::WholeScreen:
        wholeScreen();
        break;

    case Screenshot::SelectedArea:
        selectedArea();
        break;

    case Screenshot::ActiveWindow:
        activeWindow();
        break;

    case Screenshot::SelectedWindow:
        selectedWindow();
        break;
    }

    if (mPixmapDelay) {
        return;
    }

    if (!mPixmap.isNull()) {
        confirm(true);
        return;
    }

    if (mCancelled) {
        confirm(false);
        return;
    }

    // The grab produced nothing and the user didn't cancel: the capture failed
    // (the compositor refused it, the target window went away). Reporting that
    // as Cancel sends it down the one path that stays deliberately quiet, so
    // the user is told nothing at all.
    mOptions.result = Screenshot::Failure;
    emit finished();
    emit cleanup();
}

void Screenshot::refresh()
{
    // Only reached from the area selector (F5), so keep it cursor-free too.
    grabDesktop(false);
}

//

void Screenshot::activeWindow()
{
#ifdef Q_OS_WIN
    HWND fWindow = GetForegroundWindow();

    if (fWindow == NULL) {
        return;
    }

    if (fWindow == GetDesktopWindow()) {
        wholeScreen();
        return;
    }

    mPixmap = os::grabWindow((WId)GetForegroundWindow());
#endif

#if defined(Q_OS_LINUX)
    auto *x11app = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();

    if (x11app) {
        Window focus;
        int revert;

        XGetInputFocus(x11app->display(), &focus, &revert);

        mPixmap = QGuiApplication::primaryScreen()->grabWindow(focus);
    }
    // else: non-X11 (Wayland) — capture path is a separate future effort; leave mPixmap empty.
#endif
}

const QString Screenshot::extension() const
{
    switch (mOptions.format) {
    case Screenshot::PNG:
        return QStringLiteral(".png");
        break;
    case Screenshot::BMP:
        return QStringLiteral(".bmp");
        break;
    case Screenshot::WEBP:
        return QStringLiteral(".webp");
        break;
    case Screenshot::JPEG:
        return QStringLiteral(".jpg");
        break;
    }

    return QStringLiteral(".png");
}

void Screenshot::grabDesktop(bool includeCursor)
{
    QRect geometry;
    QPoint cursorPosition = QCursor::pos();

    if (mOptions.currentMonitor) {
        QScreen *currentScreen = QGuiApplication::screenAt(cursorPosition);

        if (!currentScreen) {
            currentScreen = QGuiApplication::primaryScreen();
        }

        geometry = currentScreen->geometry();
        cursorPosition = cursorPosition - geometry.topLeft();
    } else {
        int top = 0;

        for (QScreen *screen : QGuiApplication::screens()) {
            auto screenRect = screen->geometry();

            if (screenRect.top() < 0) {
                top += screenRect.top() * -1;
            }

            if (screenRect.left() < 0) {
                cursorPosition.setX(cursorPosition.x() + screenRect.width()); //= localCursorPos + screenRect.normalized().topLeft();
            }

            geometry = geometry.united(screenRect);
        }

        cursorPosition.setY(cursorPosition.y() + top);
    }

    mPixmap = QApplication::primaryScreen()->grabWindow(0, geometry.x(), geometry.y(), geometry.width(), geometry.height());
    mPixmap.setDevicePixelRatio(QApplication::primaryScreen()->devicePixelRatio());

    if (mOptions.cursor && includeCursor && !mPixmap.isNull()) {
        QPainter painter(&mPixmap);
        auto cursorInfo = os::cursor();
        auto cursorPixmap = cursorInfo.first;
        cursorPixmap.setDevicePixelRatio(QApplication::primaryScreen()->devicePixelRatio());

#if 0 // Debug cursor position helper
        painter.setBrush(QBrush(Qt::darkRed));
        painter.setPen(QPen(QBrush(Qt::red), 5));
        QRectF rect;
        rect.setSize(QSizeF(100, 100));
        rect.moveCenter(cursorPosition);
        painter.drawRoundRect(rect, rect.size().height()*2, rect.size().height()*2);
#endif

        painter.drawPixmap(cursorPosition-cursorInfo.second, cursorPixmap);
    }
}

const QString Screenshot::newFileName() const
{
    if (!mOptions.directory.exists() && !mOptions.directory.mkpath(mOptions.directory.path())) {
        // The save below will fail and be reported as a failure; name the
        // directory here so the cause is diagnosable rather than just "an error
        // occurred" — an unwritable or stale target is the usual reason.
        qWarning() << "Could not create the screenshot directory:" << mOptions.directory.path();
    }

    QString naming = Screenshot::getName(mOptions.namingOptions, mOptions.prefix, mOptions.directory);
    QString path   = QDir::toNativeSeparators(mOptions.directory.path());

    // Cleanup
    if (!path.isEmpty() && path.at(path.size() - 1) != QDir::separator()) {
        path.append(QDir::separator());
    }

    QString fileName;
    fileName.append(path);
    fileName.append(naming);

    return fileName;
}

void Screenshot::selectedArea()
{
    grabDesktop(false);

    if (mPixmap.isNull()) {
        return;
    }

    AreaDialog selector(this);
    int result = selector.exec();

    if (result == QDialog::Accepted) {
        mPixmap = mPixmap.copy(selector.resultRect());
    } else {
        mCancelled = true;
        mPixmap = QPixmap();
    }
}

void Screenshot::selectedWindow()
{
    WindowPicker *windowPicker = new WindowPicker;
    mPixmapDelay = true;

    connect(windowPicker, SIGNAL(pixmap(QPixmap)), this, SLOT(setPixmap(QPixmap)));
}

void Screenshot::wholeScreen()
{
    grabDesktop();
}

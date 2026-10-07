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
#include <QGuiApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QRubberBand>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>

#include <tools/windowpicker.h>
#include <tools/os.h>

#include <QImage>

#if defined(Q_OS_WIN)
    #include <windows.h>

#endif

WindowPicker::WindowPicker() : QWidget(nullptr), mCrosshair(":/icons/picker"), mWindowLabel(nullptr), mCurrentWindow(0), mTaken(false)
{
#if defined(Q_OS_WIN)
    setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
#endif

    setWindowTitle(tr("DuskScreen Window Picker"));
    setStyleSheet("QWidget { color: #000; } #frame { padding: 7px 10px; border: 4px solid #232323; background-color: rgba(250, 250, 250, 255); }");

    QLabel *helpLabel = new QLabel(tr("Grab the window picker by clicking and holding down the mouse button, then drag it to the window of your choice and release it to capture."), this);
    helpLabel->setMinimumWidth(400);
    helpLabel->setMaximumWidth(400);
    helpLabel->setWordWrap(true);

    mWindowIcon = new QLabel(this);
    mWindowIcon->setMinimumSize(22, 22);
    mWindowIcon->setMaximumSize(22, 22);
    mWindowIcon->setScaledContents(true);

    mWindowLabel = new QLabel(tr(" - Start dragging to select windows"), this);
    mWindowLabel->setStyleSheet("font-weight: bold");

    mCrosshairLabel = new QLabel(this);
    mCrosshairLabel->setAlignment(Qt::AlignHCenter);
    mCrosshairLabel->setPixmap(mCrosshair);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &WindowPicker::close);

    QHBoxLayout *windowLayout = new QHBoxLayout;
    windowLayout->addWidget(mWindowIcon);
    windowLayout->addWidget(mWindowLabel);
    windowLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(0);
    buttonLayout->addWidget(closeButton);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *crosshairLayout = new QHBoxLayout;
    crosshairLayout->addStretch(0);
    crosshairLayout->addWidget(mCrosshairLabel);
    crosshairLayout->addStretch(0);
    crosshairLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *fl = new QVBoxLayout;
    fl->addWidget(helpLabel);
    fl->addLayout(windowLayout);
    fl->addLayout(crosshairLayout);
    fl->addLayout(buttonLayout);
    fl->setContentsMargins(0, 0, 0, 0);

    QFrame *frame = new QFrame(this);
    frame->setObjectName("frame");
    frame->setLayout(fl);

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(frame);

    setLayout(l);

    resize(sizeHint());

    // screenAt() returns null when the cursor sits on a coordinate no screen
    // covers — a monitor just unplugged, or a gap between differently-sized
    // ones. Screenshot::grabDesktop() already falls back the same way.
    const QScreen *cursorScreen = QGuiApplication::screenAt(QCursor::pos());

    if (!cursorScreen) {
        cursorScreen = QGuiApplication::primaryScreen();
    }

    if (cursorScreen) {
        move(cursorScreen->geometry().center() - QPoint(width() / 2, height() / 2));
    }

    show();
}

WindowPicker::~WindowPicker()
{
    qApp->restoreOverrideCursor();
}

void WindowPicker::cancel()
{
    mWindowIcon->setPixmap(QPixmap());
    mCrosshairLabel->setPixmap(mCrosshair);
    qApp->restoreOverrideCursor();
}

void WindowPicker::closeEvent(QCloseEvent *)
{
    if (!mTaken) {
        emit pixmap(QPixmap());
    }

    qApp->restoreOverrideCursor();
    deleteLater();
}

void WindowPicker::mouseMoveEvent(QMouseEvent *event)
{
    QString windowName;

#if defined(Q_OS_WIN)
    POINT mousePos;
    const QPoint globalPos = event->globalPosition().toPoint();
    mousePos.x = globalPos.x();
    mousePos.y = globalPos.y();

    HWND cWindow = GetAncestor(WindowFromPoint(mousePos), GA_ROOT);

    mCurrentWindow = (WId) cWindow;

    if (mCurrentWindow == winId()) {
        mWindowIcon->setPixmap(QPixmap());
        mWindowLabel->setText("");
        return;
    }

    // Text
    WCHAR str[256];
    HICON icon;

    ::GetWindowText((HWND)mCurrentWindow, str, 256);
    windowName = QString::fromWCharArray(str);
    ///

    // Retrieving the application icon
    // GetClassLongPtr is the correct API for handle-sized class data; on 32-bit
    // it is defined to GetClassLong. Not a live bug (the HICONs Windows hands
    // out here fit in 32 bits), but the previous GetClassLong + GCLP_HICON
    // pairing was only correct by accident.
    icon = (HICON)::GetClassLongPtr((HWND)mCurrentWindow, GCLP_HICON);

    if (icon != NULL) {
        mWindowIcon->setPixmap(QPixmap::fromImage(QImage::fromHICON(icon)));
    } else {
        mWindowIcon->setPixmap(QPixmap());
    }
#endif

    if (windowName.isEmpty()) {
        mWindowLabel->setText("");
        return;
    }

    const int maxTitleLength = 60;

    if (windowName.length() > maxTitleLength) {
        windowName = windowName.left(maxTitleLength) + "...";
    }

    if (mWindowIcon->pixmap().isNull()) {
        mWindowLabel->setText(QString(" - %1").arg(windowName));
    } else {
        mWindowLabel->setText(windowName);
    }
}

void WindowPicker::mousePressEvent(QMouseEvent *event)
{
    qApp->setOverrideCursor(QCursor(mCrosshair));
    mCrosshairLabel->setMinimumWidth(mCrosshairLabel->width());
    mCrosshairLabel->setMinimumHeight(mCrosshairLabel->height());
    mCrosshairLabel->setPixmap(QPixmap());
    QWidget::mousePressEvent(event);
}

void WindowPicker::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Resolve the window once, here, and use that same handle for both the
        // validity check and the grab. The Linux path used to check the window
        // resolved at release but capture mCurrentWindow, which mouseMoveEvent
        // had written — they disagree whenever move events stop arriving before
        // the release, capturing a window the user wasn't pointing at.
#if defined(Q_OS_WIN)
        POINT mousePos;
        const QPoint globalPos = event->globalPosition().toPoint();
        mousePos.x = globalPos.x();
        mousePos.y = globalPos.y();

        WId nativeWindow = (WId)GetAncestor(WindowFromPoint(mousePos), GA_ROOT);
#else
        WId nativeWindow = 0;
#endif

        // A zero handle means nothing pickable was under the cursor. Qt reads it
        // as "grab the entire screen", so the picker would silently return a
        // full-desktop capture instead of a window.
        if (nativeWindow == 0 || nativeWindow == winId()) {
            cancel();
            return;
        }

        mTaken = true;

        setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
        close();

        emit pixmap(os::grabWindow(nativeWindow));

        return;
    }

    close();
}


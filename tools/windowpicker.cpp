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

#elif defined(Q_OS_LINUX)
    #include <QtGui/qguiapplication_platform.h>
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xatom.h>

// X11 display handle for the current QGuiApplication, or nullptr when the
// platform plugin isn't xcb (e.g. Wayland). Mirrors os.cpp's helper; this
// file is X11-only pre-existing behavior (used the now-removed Qt5
// X11Extras display accessor).
static Display *x11Display()
{
    auto *x11app = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11app ? x11app->display() : nullptr;
}

// Bounds on what we accept from _NET_WM_ICON, which any window on the display
// can set: enough elements for a generous icon set, and a per-edge cap so a
// bogus size can't be used to compute an enormous allocation.
const long kMaxIconElements = 1024 * 1024;
const unsigned long kMaxIconEdge = 4096;
#endif

WindowPicker::WindowPicker() : QWidget(nullptr), mCrosshair(":/icons/picker"), mWindowLabel(nullptr), mCurrentWindow(0), mTaken(false)
{
#if defined(Q_OS_WIN)
    setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
#elif defined(Q_OS_LINUX)
    setWindowFlags(Qt::WindowStaysOnTopHint);
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
    mousePos.x = event->globalX();
    mousePos.y = event->globalY();

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
#elif defined(Q_OS_LINUX)
    if (!x11Display()) {
        // non-X11 (e.g. Wayland): window picking is unavailable — don't touch Xlib.
        mWindowIcon->setPixmap(QPixmap());
        mWindowLabel->setText("");
        return;
    }

    Q_UNUSED(event) // the X11 path resolves the window from the pointer, not the event

    Window cWindow = os::windowUnderCursor(false);

    if (cWindow == mCurrentWindow) {
        return;
    }

    mCurrentWindow = cWindow;

    if (mCurrentWindow == None || mCurrentWindow == winId()) {
        mWindowIcon->setPixmap(QPixmap());
        mWindowLabel->setText("");
        return;
    }

    // Getting the window name property.
    XTextProperty tp;
    char **text;
    int count;

    if (XGetTextProperty(x11Display(), cWindow, &tp, XA_WM_NAME) != 0 && tp.value != nullptr) {
        if (tp.encoding == XA_STRING) {
            windowName = QString::fromLocal8Bit((const char *) tp.value);
        } else if (XmbTextPropertyToTextList(x11Display(), &tp, &text, &count) == Success &&
                   text != nullptr && count > 0) {
            windowName = QString::fromLocal8Bit(text[0]);
            XFreeStringList(text);
        }

        XFree(tp.value);
    }

    // _NET_WM_ICON holds a sequence of icons, each an ARGB image preceded by its
    // width and height. Fetched in a single request: the previous code issued
    // three (two of them one-element reads just for the dimensions), so the
    // property could change between them, and it read those dimensions as bytes.
    // A format-32 property arrives as an array of long — eight bytes per element
    // on 64-bit — so that yielded only the low byte of the width, turning a
    // 300px icon into 44px, and the pixels were then handed to QImage as if they
    // were packed 32-bit values.
    Atom type_ret = None;
    int format = 0;
    unsigned long n = 0;
    unsigned long extra = 0;
    unsigned char *iconData = nullptr;

    const Atom _net_wm_icon = XInternAtom(x11Display(), "_NET_WM_ICON", False);

    mWindowIcon->setPixmap(QPixmap());

    if (XGetWindowProperty(x11Display(), cWindow, _net_wm_icon, 0, kMaxIconElements, False,
                           XA_CARDINAL, &type_ret, &format, &n, &extra, &iconData) == Success
            && iconData) {

        if (format == 32 && type_ret == XA_CARDINAL) {
            const unsigned long *elements = reinterpret_cast<const unsigned long *>(iconData);

            unsigned long bestOffset = 0;
            int bestWidth  = 0;
            int bestHeight = 0;

            // Walk the icons, keeping the largest that actually fits inside what
            // the server returned. Any window on the display can set this
            // property, so the advertised sizes are not to be trusted.
            for (unsigned long i = 0; i + 2 <= n;) {
                const unsigned long iconWidth  = elements[i];
                const unsigned long iconHeight = elements[i + 1];

                if (iconWidth == 0 || iconHeight == 0
                        || iconWidth > kMaxIconEdge || iconHeight > kMaxIconEdge) {
                    break;
                }

                if (i + 2 + iconWidth * iconHeight > n) {
                    break;    // advertised larger than delivered — stop, don't over-read
                }

                if (static_cast<int>(iconWidth) > bestWidth) {
                    bestWidth  = static_cast<int>(iconWidth);
                    bestHeight = static_cast<int>(iconHeight);
                    bestOffset = i + 2;
                }

                i += 2 + iconWidth * iconHeight;
            }

            if (bestWidth > 0) {
                // One pixel per element, in its low 32 bits, so they have to be
                // repacked rather than reinterpreted.
                QImage icon(bestWidth, bestHeight, QImage::Format_ARGB32);

                for (int y = 0; y < bestHeight; ++y) {
                    QRgb *line = reinterpret_cast<QRgb *>(icon.scanLine(y));

                    for (int x = 0; x < bestWidth; ++x) {
                        line[x] = static_cast<QRgb>(
                                      elements[bestOffset + static_cast<unsigned long>(y) * bestWidth + x] & 0xffffffffUL);
                    }
                }

                mWindowIcon->setPixmap(QPixmap::fromImage(icon));
            }
        }

        XFree(iconData);
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
        mousePos.x = event->globalX();
        mousePos.y = event->globalY();

        WId nativeWindow = (WId)GetAncestor(WindowFromPoint(mousePos), GA_ROOT);
#elif defined(Q_OS_LINUX)
        WId nativeWindow = (WId)os::windowUnderCursor(false);
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

#ifdef Q_OS_LINUX
        emit pixmap(QGuiApplication::primaryScreen()->grabWindow(nativeWindow));
#else
        emit pixmap(os::grabWindow(nativeWindow));
#endif

        return;
    }

    close();
}


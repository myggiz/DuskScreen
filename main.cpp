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
#include <QLocale>
#include <QTimer>
#include <QDataStream>
#include <QByteArray>

#include <tools/os.h>
#include "tools/SingleApplication/singleapplication.h"

#include <duskscreenwindow.h>

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    qSetMessagePattern("%{message} @%{line}[%{function}()]");
#endif

    QApplication::setOrganizationName("Myggiz");
    QApplication::setApplicationName("DuskScreen");
    QApplication::setApplicationVersion(APP_VERSION);

    // allowSecondary so the constructor returns in a second instance instead of
    // exiting inside itself, which is what lets the arguments be forwarded below.
    SingleApplication application(argc, argv, true);

    if (application.isSecondary()) {
        // Hand our command line to the running instance and get out of the way.
        // Upstream has no equivalent of the old fork's instanceArguments signal,
        // so the arguments travel as an explicit message.
        QByteArray payload;
        QDataStream stream(&payload, QIODevice::WriteOnly);

        stream.setVersion(QDataStream::Qt_6_0);
        stream << QApplication::arguments();

        application.sendMessage(payload);
        return 0;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    DuskScreenWindow lightscreen;

    if (QApplication::arguments().size() > 1) {
        // Deferred until exec() is running. QCoreApplication::quit() does nothing
        // when there is no event loop, so dispatching here directly meant that
        // "--quit" on an instance that wasn't already running *started* the
        // application instead of stopping it. The forwarded-argument path from a
        // second instance always ran inside the loop and was unaffected.
        const QStringList arguments = QApplication::arguments();

        QTimer::singleShot(0, &lightscreen, [&lightscreen, arguments] {
            lightscreen.executeArguments(arguments);
        });
    } else {
        lightscreen.show();
    }

    QObject::connect(&application, &SingleApplication::receivedMessage, &lightscreen,
    [&lightscreen](quint32, const QByteArray & message) {
        QStringList arguments;
        QDataStream stream(message);

        stream.setVersion(QDataStream::Qt_6_0);
        stream >> arguments;

        if (stream.status() == QDataStream::Ok && !arguments.isEmpty()) {
            lightscreen.executeArguments(arguments);
        }
    });

    QObject::connect(&lightscreen, &DuskScreenWindow::finished, &application, &SingleApplication::quit);

    return application.exec();
}

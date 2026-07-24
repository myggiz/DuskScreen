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
#include <QByteArray>
#include <QDataStream>
#include <QStringList>

#include "singleapplication.h"

#include <duskscreenwindow.h>

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    qSetMessagePattern("%{message} @%{line}[%{function}()]");
#endif

    QApplication::setOrganizationName("Myggiz");
    QApplication::setApplicationName("DuskScreen");
    QApplication::setApplicationVersion(APP_VERSION);

    // allowSecondary = true so the constructor RETURNS on a second launch (letting
    // us forward args below) instead of calling ::exit() internally, which would
    // make the isSecondary() block below dead code.
    SingleApplication application(argc, argv, true);

    // A second launch forwards its command-line arguments to the already-running
    // primary instance and then exits. (itay-grudev SingleApplication replaces the
    // old fork's automatic instanceArguments() signal with explicit
    // sendMessage()/receivedMessage() over its inter-instance channel.)
    if (application.isSecondary()) {
        QByteArray payload;
        QDataStream out(&payload, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << QApplication::arguments();
        application.sendMessage(payload);
        return 0;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    DuskScreenWindow lightscreen;

    if (QApplication::arguments().size() > 1) {
        lightscreen.executeArguments(QApplication::arguments());
    } else {
        lightscreen.show();
    }

    QObject::connect(&application, &SingleApplication::receivedMessage, &lightscreen,
        [&lightscreen](quint32 /*instanceId*/, const QByteArray &message) {
            QDataStream in(message);
            in.setVersion(QDataStream::Qt_6_0);
            QStringList arguments;
            in >> arguments;
            if (in.status() != QDataStream::Ok) {
                return;  // ignore a malformed/truncated inter-instance message
            }
            lightscreen.executeArguments(arguments);
        });
    QObject::connect(&lightscreen, &DuskScreenWindow::finished, &application, &SingleApplication::quit);

    return QApplication::exec();
}

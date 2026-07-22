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

    SingleApplication application(argc, argv);

    application.setQuitOnLastWindowClosed(false);

    DuskScreenWindow lightscreen;

    if (application.arguments().size() > 1) {
        lightscreen.executeArguments(application.arguments());
    } else {
        lightscreen.show();
    }

    QObject::connect(&application, &SingleApplication::instanceArguments, &lightscreen, &DuskScreenWindow::executeArguments);
    QObject::connect(&lightscreen, &DuskScreenWindow::finished, &application, &SingleApplication::quit);

    return application.exec();
}

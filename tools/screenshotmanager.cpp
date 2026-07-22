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
#include <tools/screenshotmanager.h>
#include <tools/screenshot.h>
#include <tools/os.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>

ScreenshotManager::ScreenshotManager(QObject *parent) : QObject(parent)
{
    if (QFile::exists(qApp->applicationDirPath() + QDir::separator() + "config.ini")) {
        mSettings     = new QSettings(qApp->applicationDirPath() + QDir::separator() + "config.ini", QSettings::IniFormat, this);
        mPortableMode = true;
    } else {
        mSettings     = new QSettings(this);
        mPortableMode = false;
    }
}

int ScreenshotManager::activeCount() const
{
    return mScreenshots.count();
}

bool ScreenshotManager::portableMode()
{
    return mPortableMode;
}

//

void ScreenshotManager::askConfirmation()
{
    Screenshot *s = qobject_cast<Screenshot *>(sender());
    emit confirm(s);
}

void ScreenshotManager::cleanup()
{
    Screenshot *screenshot = qobject_cast<Screenshot *>(sender());
    emit windowCleanup(screenshot->options());
}

void ScreenshotManager::finished()
{
    Screenshot *screenshot = qobject_cast<Screenshot *>(sender());
    mScreenshots.removeOne(screenshot);
    emit activeCountChange();
    screenshot->deleteLater();
}

void ScreenshotManager::take(Screenshot::Options &options)
{
    Screenshot *newScreenshot = new Screenshot(this, options);
    mScreenshots.append(newScreenshot);

    connect(newScreenshot, &Screenshot::askConfirmation, this, &ScreenshotManager::askConfirmation);
    connect(newScreenshot, &Screenshot::cleanup        , this, &ScreenshotManager::cleanup);
    connect(newScreenshot, &Screenshot::finished       , this, &ScreenshotManager::finished);

    newScreenshot->take();
}

// Singleton
ScreenshotManager *ScreenshotManager::mInstance = nullptr;

ScreenshotManager *ScreenshotManager::instance()
{
    if (!mInstance) {
        mInstance = new ScreenshotManager();
    }

    return mInstance;
}

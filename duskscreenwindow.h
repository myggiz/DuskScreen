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
#ifndef DUSKSCREENWINDOW_H
#define DUSKSCREENWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QSystemTrayIcon>

#include <updater/updater.h>
#include <tools/screenshot.h>

#include "ui_duskscreenwindow.h"

class Updater;
class QSettings;
class QProgressBar;
class UGlobalHotkeys;
class DuskScreenWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Action {
        ShowMainWindow = 5,
        OpenScreenshotFolder = 6
    };
    Q_ENUM(Action)

    DuskScreenWindow(QWidget *parent = nullptr);
    ~DuskScreenWindow();

public slots:
    void action(int mode = 3);
    void areaHotkey();
    void checkForUpdates();
    void cleanup(const Screenshot::Options &options);
    void closeToTrayWarning();
    bool closingWithoutTray();
    void goToFolder();
    void messageClicked();
    void executeArgument(const QString &message);
    void executeArguments(const QStringList &arguments);
    void notify(const Screenshot::Result &result);
    void quit();
    void restoreNotification();
    void setStatus(QString status = "");
    void screenshotAction(Screenshot::Mode mode = Screenshot::None, bool delayed = false);
    void screenshotActionTriggered(QAction *action);
    void screenHotkey();
    void showHotkeyError(const QStringList &hotkeys);
    void showOptions();
    void showScreenshotMessage(const Screenshot::Result &result, const QString &fileName);
    void toggleVisibility();
    void updateStatus();
    void updaterDone(bool available, const QString &version, const QString &url);
    void windowPickerHotkey();

private slots:
    void applySettings();

signals:
    void finished();

private:
    void connectHotkeys();
    void createTrayIcon();

    // Convenience function
    QSettings *settings() const;

protected:
    bool event(QEvent *event) override;

private:
    bool mDoCache;
    bool mHideTrigger;
    bool mWasVisible;
    int  mLastMessage;
    QString mLastScreenshot;
    QPointer<QSystemTrayIcon> mTrayIcon;
    QPointer<Updater> mUpdater;
    Ui::DuskScreenWindowClass ui;

    QPointer<UGlobalHotkeys> mGlobalHotkeys;

};

#endif // DUSKSCREENWINDOW_H


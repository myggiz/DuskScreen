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
#include <QActionGroup>
#include <QDate>
#include <QDesktopServices>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QScreen>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QToolTip>
#include <QUrl>
#include <QSoundEffect>
#include <QKeyEvent>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

//
//DuskScreen includes
//
#include <duskscreenwindow.h>
#include <dialogs/optionsdialog.h>

#include <tools/os.h>
#include <tools/screenshot.h>
#include <tools/screenshotmanager.h>
#include <tools/UGlobalHotkey/uglobalhotkeys.h>

#include <updater/updater.h>

// Qt 6 removed QSound; QSoundEffect is the WAV-playback replacement. Fire-and-forget:
// the effect deletes itself once playback finishes.
static void playSoundFile(const QString &fileName)
{
    auto *effect = new QSoundEffect;
    effect->setSource(QUrl::fromLocalFile(fileName));
    QObject::connect(effect, &QSoundEffect::playingChanged, effect, [effect] {
        if (!effect->isPlaying()) {
            effect->deleteLater();
        }
    });
    effect->play();
}

DuskScreenWindow::DuskScreenWindow(QWidget *parent) :
    QMainWindow(parent),
    mDoCache(false),
    mHideTrigger(false),
    mReviveMain(false),
    mWasVisible(true),
    mLastMessage(0),
    mLastMode(Screenshot::None),
    mLastScreenshot(),
    mHasTaskbarButton(false)
{
    ui.setupUi(this);

    ui.screenPushButton->setIcon(os::icon("screen.big"));
    ui.areaPushButton->setIcon(os::icon("area.big"));
    ui.windowPushButton->setIcon(os::icon("pickWindow.big"));

    ui.optionsPushButton->setIcon(os::icon("configure"));
    ui.folderPushButton->setIcon(os::icon("folder"));

    setMinimumSize(size());
    setWindowFlags(windowFlags() ^ Qt::WindowMaximizeButtonHint);

    // Actions
    connect(ui.screenPushButton, &QPushButton::clicked, this, &DuskScreenWindow::screenHotkey);
    connect(ui.areaPushButton  , &QPushButton::clicked, this, &DuskScreenWindow::areaHotkey);
    connect(ui.windowPushButton, &QPushButton::clicked, this, &DuskScreenWindow::windowPickerHotkey);

    connect(ui.optionsPushButton, &QPushButton::clicked, this, &DuskScreenWindow::showOptions);
    connect(ui.folderPushButton , &QPushButton::clicked, this, &DuskScreenWindow::goToFolder);

    // Shortcuts
    mGlobalHotkeys = new UGlobalHotkeys(this);

    connect(mGlobalHotkeys, &UGlobalHotkeys::activated, [&](size_t id) {
        action(id);
    });

    // Manager
    connect(ScreenshotManager::instance(), &ScreenshotManager::confirm,           this, &DuskScreenWindow::preview);
    connect(ScreenshotManager::instance(), &ScreenshotManager::windowCleanup,     this, &DuskScreenWindow::cleanup);
    connect(ScreenshotManager::instance(), &ScreenshotManager::activeCountChange, this, &DuskScreenWindow::updateStatus);

    if (!settings()->contains("file/format")) {
        showOptions();  // There are no options (or the options config is invalid or incomplete)
    } else {
        QTimer::singleShot(0   , this, &DuskScreenWindow::applySettings);
        QTimer::singleShot(5000, this, &DuskScreenWindow::checkForUpdates);
    }
}

DuskScreenWindow::~DuskScreenWindow()
{
    settings()->setValue("lastScreenshot", mLastScreenshot);
    settings()->sync();
    mGlobalHotkeys->unregisterAllHotkeys();
}

void DuskScreenWindow::action(int mode)
{
    if (mode <= Screenshot::SelectedWindow) {
        screenshotAction((Screenshot::Mode)mode);
    } else if (mode == ShowMainWindow) {
        show();
    } else if (mode == OpenScreenshotFolder) {
        goToFolder();
    } else {
        qWarning() << "Unknown hotkey ID: " << mode;
    }
}

void DuskScreenWindow::areaHotkey()
{
    screenshotAction(Screenshot::SelectedArea);
}

void DuskScreenWindow::checkForUpdates()
{
    if (settings()->value("options/disableUpdater", false).toBool()) {
        return;
    }

    if (settings()->value("lastUpdateCheck").toInt() + 7
            > QDate::currentDate().dayOfYear()) {
        return;    // If 7 days have not passed since the last update check.
    }

    mUpdater = new Updater(this);

    connect(mUpdater, &Updater::done, this, &DuskScreenWindow::updaterDone);
    mUpdater->check();
}

void DuskScreenWindow::cleanup(const Screenshot::Options &options)
{
    // Reversing settings
    if (settings()->value("options/hide").toBool()) {
#ifndef Q_OS_LINUX // X is not quick enough and the notification ends up everywhere but in the icon
        if (settings()->value("options/tray").toBool() && mTrayIcon) {
            mTrayIcon->show();
        }
#endif

        if (mWasVisible) {
            show();
        }

        mHideTrigger = false;
    }

    if (settings()->value("options/tray").toBool() && mTrayIcon) {
        notify(options.result);

        if (settings()->value("options/message").toBool() && options.file) {
            showScreenshotMessage(options.result, options.fileName);
        }
    }

    if (settings()->value("options/playSound", false).toBool()) {
        if (options.result == Screenshot::Success) {
            playSoundFile("sounds/ls.screenshot.wav");
        } else {
            playSoundFile("sound/ls.error.wav");
        }
    }

    updateStatus();

    if (options.result != Screenshot::Success) {
        return;
    }

    mLastScreenshot = options.fileName;
}

void DuskScreenWindow::closeToTrayWarning()
{
    if (!settings()->value("options/closeToTrayWarning", true).toBool()) {
        return;
    }

    mLastMessage = 3;
    mTrayIcon->showMessage(tr("Closed to tray"), tr("DuskScreen will keep running, you can disable this in the options menu."));
    settings()->setValue("options/closeToTrayWarning", false);
}

bool DuskScreenWindow::closingWithoutTray()
{
    if (settings()->value("options/disableHideAlert", false).toBool()) {
        return false;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("DuskScreen"));
    msgBox.setText(tr("You have chosen to hide DuskScreen when there's no system tray icon, so you will not be able to access the program <b>unless you have selected a hotkey to do so</b>.<br>What do you want to do?"));
    msgBox.setIcon(QMessageBox::Warning);

    msgBox.setStyleSheet("QPushButton { padding: 4px 8px; }");

    auto enableButton      = msgBox.addButton(tr("Hide but enable tray"), QMessageBox::ActionRole);
    auto enableQuietButton = msgBox.addButton(tr("Hide and don't warn"), QMessageBox::ActionRole);
    auto hideButton        = msgBox.addButton(tr("Just hide"), QMessageBox::ActionRole);
    auto abortButton       = msgBox.addButton(QMessageBox::Cancel);

    Q_UNUSED(abortButton);

    msgBox.exec();

    if (msgBox.clickedButton() == hideButton) {
        return true;
    } else if (msgBox.clickedButton() == enableQuietButton) {
        settings()->setValue("options/disableHideAlert", true);
        applySettings();
        return true;
    } else if (msgBox.clickedButton() == enableButton) {
        settings()->setValue("options/tray", true);
        applySettings();
        return true;
    }

    return false; // Cancel.
}

void DuskScreenWindow::goToFolder()
{
#ifdef Q_OS_WIN
    if (!mLastScreenshot.isEmpty() && QFile::exists(mLastScreenshot)) {
        // Reveal the file in Explorer with it selected. The "/select," switch and the
        // path must be a SINGLE argument (comma glued to a native-separator path);
        // Qt 6's deprecated string-splitting startDetached() would break them apart,
        // making Explorer just open the file in the default image viewer instead.
        QProcess::startDetached("explorer.exe", { "/select," + QDir::toNativeSeparators(mLastScreenshot) });
    } else {
#endif
        QDir path(settings()->value("file/target").toString());

        // We might want to go to the folder without it having been created by taking a screenshot yet.
        if (!path.exists()) {
            path.mkpath(path.absolutePath());
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(path.absolutePath() + QDir::separator()));
#ifdef Q_OS_WIN
    }
#endif
}

void DuskScreenWindow::messageClicked()
{
    if (mLastMessage == 1) {
        goToFolder();
    } else if (mLastMessage == 3) {
        QTimer::singleShot(0, this, &DuskScreenWindow::showOptions);
    }
}

void DuskScreenWindow::executeArgument(const QString &message)
{
    if (message == "--wake") {
        show();
        os::setForegroundWindow(this);
        qApp->alert(this, 2000);
    } else if (message == "--screen") {
        screenshotAction(Screenshot::WholeScreen);
    } else if (message == "--area") {
        screenshotAction(Screenshot::SelectedArea);
    } else if (message == "--activewindow") {
        screenshotAction(Screenshot::ActiveWindow);
    } else if (message == "--pickwindow") {
        screenshotAction(Screenshot::SelectedWindow);
    } else if (message == "--folder") {
        action(OpenScreenshotFolder);
    } else if (message == "--options") {
        showOptions();
    } else if (message == "--quit") {
        qApp->quit();
    }
}

void DuskScreenWindow::executeArguments(const QStringList &arguments)
{
    // If we just have the default argument, call "--wake"
    if (arguments.count() == 1 && (arguments.at(0) == qApp->arguments().at(0) || arguments.at(0).contains(QFileInfo(qApp->applicationFilePath()).fileName()))) {
        executeArgument("--wake");
        return;
    }

    for (auto argument : arguments) {
        executeArgument(argument);
    }
}

void DuskScreenWindow::notify(const Screenshot::Result &result)
{
    switch (result) {
    case Screenshot::Success:
        mTrayIcon->setIcon(QIcon(":/icons/lightscreen.yes"));

        if (mHasTaskbarButton) {
            mTaskbarButton->setOverlayIcon(os::icon("yes"));
        }

        setWindowTitle(tr("Success!"));
        break;
    case Screenshot::Failure:
        mTrayIcon->setIcon(QIcon(":/icons/lightscreen.no"));
        setWindowTitle(tr("Failed!"));

        if (mHasTaskbarButton) {
            mTaskbarButton->setOverlayIcon(os::icon("no"));
        }

        break;
    case Screenshot::Cancel:
        setWindowTitle(tr("Cancelled!"));
        break;
    }

    QTimer::singleShot(2000, this, &DuskScreenWindow::restoreNotification);
}

void DuskScreenWindow::preview(Screenshot *screenshot)
{
    screenshot->confirm(true);
}

void DuskScreenWindow::quit()
{
    settings()->setValue("position", pos());

    int answer = 0;
    QString doing;

    if (ScreenshotManager::instance()->activeCount() > 0) {
        doing = tr("processing");
    }

    if (!doing.isEmpty()) {
        answer = QMessageBox::question(this,
                                       tr("Are you sure you want to quit?"),
                                       tr("DuskScreen is currently %1 screenshots. Are you sure you want to quit?").arg(doing),
                                       tr("Quit"),
                                       tr("Don't Quit"));
    }

    if (answer == 0) {
        emit finished();
    }
}

void DuskScreenWindow::restoreNotification()
{
    if (mTrayIcon) {
        mTrayIcon->setIcon(QIcon(":/icons/lightscreen.small"));
    }

    if (mHasTaskbarButton) {
        mTaskbarButton->clearOverlayIcon();
        mTaskbarButton->progress()->setVisible(false);
        mTaskbarButton->progress()->stop();
        mTaskbarButton->progress()->reset();
    }

    updateStatus();
}

void DuskScreenWindow::screenshotAction(Screenshot::Mode mode)
{
    int delayms = -1;

    bool optionsHide = settings()->value("options/hide").toBool(); // Option cache, used a couple of times.

    if (!mHideTrigger) {
        mWasVisible = isVisible();
        mHideTrigger = true;
    }

    // Applying pre-screenshot settings
    if (optionsHide) {
        hide();

#ifndef Q_OS_LINUX // X is not quick enough and the notification ends up everywhere but in the icon
        if (mTrayIcon) {
            mTrayIcon->hide();
        }
#endif
    }

    // Screenshot delay
    delayms = settings()->value("options/delay", 0).toInt();
    delayms = delayms * 1000; // Converting the delay to milliseconds.

    delayms += 400;

    // The delayed functions works using the static variable lastMode
    // which keeps the argument so a QTimer can call this function again.
    if (delayms > 0) {
        if (mLastMode == Screenshot::None) {
            mLastMode = mode;

            QTimer::singleShot(delayms, this, [&] {
                screenshotAction(mLastMode);
            });
            return;
        } else {
            mode = mLastMode;
            mLastMode = Screenshot::None;
        }
    }

    static Screenshot::Options options;

    if (!mDoCache) {
        // Populating the option object that will then be passed to the screenshot engine (sounds fancy huh?)
        options.file           = settings()->value("file/enabled").toBool();
        options.format         = (Screenshot::Format) settings()->value("file/format").toInt();
        options.prefix         = settings()->value("file/prefix").toString();

        QDir dir(settings()->value("file/target").toString());
        dir.makeAbsolute();

        options.directory      = dir;

        options.quality        = settings()->value("options/quality", 100).toInt();
        options.currentMonitor = settings()->value("options/currentMonitor", false).toBool();
        options.clipboard      = settings()->value("options/clipboard",      true).toBool();
        options.magnify        = settings()->value("options/magnify",        false).toBool();
        options.cursor         = settings()->value("options/cursor",         true).toBool();
        options.saveAs         = settings()->value("options/saveAs",         false).toBool();
        options.animations     = settings()->value("options/animations",     true)  .toBool();
        options.replace        = settings()->value("options/replace",        false).toBool();
        options.optimize       = settings()->value("options/optimize",       false).toBool();

        Screenshot::NamingOptions namingOptions;
        namingOptions.naming       = (Screenshot::Naming) settings()->value("file/naming").toInt();
        namingOptions.leadingZeros = settings()->value("options/naming/leadingZeros", 0).toInt();
        namingOptions.flip         = settings()->value("options/flip", false).toBool();
        namingOptions.dateFormat   = settings()->value("options/naming/dateFormat", "yyyy-MM-dd").toString();

        options.namingOptions = namingOptions;

        mDoCache = true;
    }

    options.mode = mode;

    ScreenshotManager::instance()->take(options);
}

void DuskScreenWindow::screenshotActionTriggered(QAction *action)
{
    screenshotAction(action->data().value<Screenshot::Mode>());
}

void DuskScreenWindow::screenHotkey()
{
    screenshotAction(Screenshot::WholeScreen);
}

void DuskScreenWindow::showHotkeyError(const QStringList &hotkeys)
{
    static bool dontShow = false;

    if (dontShow) {
        return;
    }

    QString messageText;

    messageText = tr("Some hotkeys could not be registered, they might already be in use");

    if (hotkeys.count() > 1) {
        messageText += tr("<br>The failed hotkeys are the following:") + "<ul>";

        for (auto hotkey : hotkeys) {
            messageText += QString("%1%2%3").arg("<li><b>").arg(hotkey).arg("</b></li>");
        }

        messageText += "</ul>";
    } else {
        messageText += tr("<br>The failed hotkey is <b>%1</b>").arg(hotkeys[0]);
    }

    messageText += tr("<br><i>What do you want to do?</i>");

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("DuskScreen"));
    msgBox.setText(messageText);

    QPushButton *changeButton  = msgBox.addButton(tr("Change") , QMessageBox::ActionRole);
    QPushButton *disableButton = msgBox.addButton(tr("Disable"), QMessageBox::ActionRole);
    QPushButton *exitButton    = msgBox.addButton(tr("Quit")   , QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == exitButton) {
        dontShow = true;
        QTimer::singleShot(10, this, &DuskScreenWindow::quit);
    } else if (msgBox.clickedButton() == changeButton) {
        showOptions();
    } else if (msgBox.clickedButton() == disableButton) {
        for (auto hotkey : hotkeys) {
            settings()->setValue(QString("actions/%1/enabled").arg(hotkey), false);
        }
    }
}

void DuskScreenWindow::showOptions()
{
    mGlobalHotkeys->unregisterAllHotkeys();
    QPointer<OptionsDialog> optionsDialog = new OptionsDialog(this);

    optionsDialog->exec();
    optionsDialog->deleteLater();

    applySettings();
}

void DuskScreenWindow::showScreenshotMessage(const Screenshot::Result &result, const QString &fileName)
{
    if (result == Screenshot::Cancel) {
        return;
    }

    // Showing message.
    QString title;
    QString message;

    if (result == Screenshot::Success) {
        title = QFileInfo(fileName).fileName();

        if (settings()->value("file/target").toString().isEmpty()) {
            message = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
        } else {
            message = tr("Saved to \"%1\"").arg(settings()->value("file/target").toString());
        }
    } else {
        title   = tr("The screenshot was not taken");
        message = tr("An error occurred.");
    }

    mLastMessage = 1;
    mTrayIcon->showMessage(title, message);
}

void DuskScreenWindow::toggleVisibility()
{
    if (isVisible()) {
        hide();
    } else {
        show();
        os::setForegroundWindow(this);
    }
}

void DuskScreenWindow::updateStatus()
{
    int activeCount = ScreenshotManager::instance()->activeCount();

    if (mHasTaskbarButton) {
        mTaskbarButton->progress()->setPaused(true);
        mTaskbarButton->progress()->setVisible(true);
    }

    if (activeCount > 1) {
        setStatus(tr("%1 processing").arg(activeCount));
    } else if (activeCount == 1) {
        setStatus(tr("processing"));
    } else {
        setStatus();

        if (mHasTaskbarButton) {
            mTaskbarButton->progress()->setVisible(false);
        }
    }
}

void DuskScreenWindow::updaterDone(bool result)
{
    mUpdater->deleteLater();

    settings()->setValue("lastUpdateCheck", QDate::currentDate().dayOfYear());

    if (!result) {
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("DuskScreen"));
    msgBox.setText(tr("There's a new version of DuskScreen available.<br>Would you like to see more information?<br>(<em>You can turn this notification off</em>)"));
    msgBox.setIcon(QMessageBox::Information);

    QPushButton *yesButton     = msgBox.addButton(QMessageBox::Yes);
    QPushButton *turnOffButton = msgBox.addButton(tr("Turn Off"), QMessageBox::ActionRole);
    QPushButton *remindButton  = msgBox.addButton(tr("Remind Me Later"), QMessageBox::RejectRole);

    Q_UNUSED(remindButton);

    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        QDesktopServices::openUrl(QUrl(QString(APP_URL "/whatsnew?from=") + qApp->applicationVersion()));
    } else if (msgBox.clickedButton() == turnOffButton) {
        settings()->setValue("options/disableUpdater", true);
    }
}

void DuskScreenWindow::windowHotkey()
{
    screenshotAction(Screenshot::ActiveWindow);
}

void DuskScreenWindow::windowPickerHotkey()
{
    screenshotAction(Screenshot::SelectedWindow);
}

void DuskScreenWindow::applySettings()
{
    bool tray = settings()->value("options/tray", true).toBool();

    if (tray && !mTrayIcon) {
        createTrayIcon();
        mTrayIcon->setVisible(true);
    } else if (!tray && mTrayIcon) {
        mTrayIcon->setVisible(false);
    }

    connectHotkeys();

    mDoCache = false;

    if (settings()->value("lastScreenshot").isValid() && mLastScreenshot.isEmpty()) {
        mLastScreenshot = settings()->value("lastScreenshot").toString();
    }

    os::setStartup(settings()->value("options/startup").toBool(), settings()->value("options/startupHide").toBool());
}

void DuskScreenWindow::connectHotkeys()
{
    const QStringList actions = {"screen", "window", "area", "windowPicker", "open", "directory"};
    QStringList failed;
    size_t id = Screenshot::WholeScreen;

    for (auto action : actions) {
        if (settings()->value("actions/" + action + "/enabled").toBool()) {
            if (!mGlobalHotkeys->registerHotkey(settings()->value("actions/" + action + "/hotkey").toString(), id)) {
                failed << action;
            }
        }

        id++;
    }

    if (!failed.isEmpty()) {
        showHotkeyError(failed);
    }
}

void DuskScreenWindow::createTrayIcon()
{
    mTrayIcon = new QSystemTrayIcon(QIcon(":/icons/lightscreen.small"), this);
    updateStatus();

    connect(mTrayIcon, &QSystemTrayIcon::messageClicked, this, &DuskScreenWindow::messageClicked);
    connect(mTrayIcon, &QSystemTrayIcon::activated     , this, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::DoubleClick) return;
        toggleVisibility();
    });

    auto hideAction = new QAction(QIcon(":/icons/lightscreen.small"), tr("Show&/Hide"), mTrayIcon);
    connect(hideAction, &QAction::triggered, this, &DuskScreenWindow::toggleVisibility);

    auto screenAction = new QAction(os::icon("screen"), tr("&Screen"), mTrayIcon);
    screenAction->setData(QVariant::fromValue<Screenshot::Mode>(Screenshot::WholeScreen));

    auto windowAction = new QAction(os::icon("window"), tr("Active &Window"), this);
    windowAction->setData(QVariant::fromValue<Screenshot::Mode>(Screenshot::ActiveWindow));

    auto windowPickerAction = new QAction(os::icon("pickWindow"), tr("&Pick Window"), this);
    windowPickerAction->setData(QVariant::fromValue<Screenshot::Mode>(Screenshot::SelectedWindow));

    auto areaAction = new QAction(os::icon("area"), tr("&Area"), mTrayIcon);
    areaAction->setData(QVariant::fromValue<Screenshot::Mode>(Screenshot::SelectedArea));

    auto screenshotGroup = new QActionGroup(mTrayIcon);
    screenshotGroup->addAction(screenAction);
    screenshotGroup->addAction(areaAction);
    screenshotGroup->addAction(windowAction);
    screenshotGroup->addAction(windowPickerAction);

    connect(screenshotGroup, &QActionGroup::triggered, this, &DuskScreenWindow::screenshotActionTriggered);

    auto optionsAction = new QAction(os::icon("configure"), tr("View &Options"), mTrayIcon);
    connect(optionsAction, &QAction::triggered, this, &DuskScreenWindow::showOptions);

    auto goAction = new QAction(os::icon("folder"), tr("&Go to Folder"), mTrayIcon);
    connect(goAction, &QAction::triggered, this, &DuskScreenWindow::goToFolder);

    auto quitAction = new QAction(tr("&Quit"), mTrayIcon);
    connect(quitAction, &QAction::triggered, this, &DuskScreenWindow::quit);

    auto screenshotMenu = new QMenu(tr("Screenshot"));
    screenshotMenu->addAction(screenAction);
    screenshotMenu->addAction(areaAction);
    screenshotMenu->addAction(windowAction);
    screenshotMenu->addAction(windowPickerAction);

    auto trayIconMenu = new QMenu;
    trayIconMenu->addAction(hideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addMenu(screenshotMenu);
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(goAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    mTrayIcon->setContextMenu(trayIconMenu);
}

void DuskScreenWindow::setStatus(QString status)
{
    if (status.isEmpty()) {
        status = tr("DuskScreen");
    } else {
        status += tr(" - DuskScreen");
    }

    if (mTrayIcon) {
        mTrayIcon->setToolTip(status);
    }

    setWindowTitle(status);
}

QSettings *DuskScreenWindow::settings() const
{
    return ScreenshotManager::instance()->settings();
}

// Event handling
bool DuskScreenWindow::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        QPoint savedPosition = settings()->value("position").toPoint();

        if (!savedPosition.isNull() && screen()->availableVirtualGeometry().contains(QRect(savedPosition, size()))) {
            move(savedPosition);
        }

        if (mHasTaskbarButton) {
            mTaskbarButton->setWindow(windowHandle());
        }
    } else if (event->type() == QEvent::Hide) {
        settings()->setValue("position", pos());
    } else if (event->type() == QEvent::Close) {
        if (settings()->value("options/tray").toBool() && settings()->value("options/closeHide").toBool()) {
            closeToTrayWarning();
            hide();
        } else if (settings()->value("options/closeHide").toBool()) {
            if (closingWithoutTray()) {
                hide();
            }
        } else {
            quit();
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
#ifdef Q_WS_MAC
        if (keyEvent->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period) {
            keyEvent->ignore();

            if (isVisible()) {
                toggleVisibility();
            }

            return false;
        } else
#endif
            if (!keyEvent->modifiers() && keyEvent->key() == Qt::Key_Escape) {
                keyEvent->ignore();

                if (isVisible()) {
                    toggleVisibility();
                }

                return false;
            }
    } else if (event->type() == QEvent::LanguageChange) {
        ui.retranslateUi(this);
        resize(minimumSizeHint());
    }


    return QMainWindow::event(event);
}

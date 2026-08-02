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
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QTimer>
#include <updater/updater.h>
#include "ui_optionsdialog.h"

class QSettings;
class QAbstractButton;
class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(QWidget *parent = nullptr);

public slots:
    void accepted();
    void checkUpdatesNow();
    void exportSettings();
    void importSettings();
    void loadSettings();
    void openUrl(QString url);
    void restoreDefaults();
    void saveSettings();
    void updatePreview();
    void schedulePreviewUpdate();

protected:
    bool event(QEvent *event) override;

#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);
#endif

private slots:
    void browse();
    void dialogButtonClicked(QAbstractButton *button);
    void flipToggled(bool checked);
    void init();
    void namingOptions();

private:
    bool hotkeyCollision();
    QSettings *settings() const;

private:
    Ui::OptionsDialog ui;

    // Coalesces preview refreshes: each one lists the whole screenshot folder.
    QTimer mPreviewTimer;
};

#endif // OPTIONSDIALOG_H

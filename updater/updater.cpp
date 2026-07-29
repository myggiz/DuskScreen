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
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QVersionNumber>

#include <updater/updater.h>
#include <dialogs/updaterdialog.h>

namespace {

// The GitHub Releases API is the source of truth for what has shipped, so there
// is no update server to run. It returns the newest non-draft, non-prerelease
// release, including its tag and the page a user should be sent to.
const QLatin1String kLatestReleaseUrl(
    "https://api.github.com/repos/myggiz/DuskScreen/releases/latest");

// Unauthenticated GitHub API calls are rate limited per IP, which is irrelevant
// at one request per launch, but the request should not hang if the API is
// unreachable — without this the reply never finishes and the Updater (and its
// QNetworkAccessManager) survive for the whole session.
const int kTimeoutMs = 15000;

}

Updater::Updater(QObject *parent) :
    QObject(parent)
{
    connect(&mNetwork, &QNetworkAccessManager::finished, this, &Updater::finished);
}

void Updater::check()
{
    QNetworkRequest request{QUrl(kLatestReleaseUrl)};

    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QString("DuskScreen/%1").arg(qApp->applicationVersion()));
    request.setTransferTimeout(kTimeoutMs);

    mNetwork.get(request);
}

void Updater::checkWithFeedback()
{
    UpdaterDialog updaterDialog;
    connect(this, &Updater::done, &updaterDialog, &UpdaterDialog::updateDone);

    check();
    updaterDialog.exec();
}

void Updater::finished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit done(false, QString(), QString());
        return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

    if (!document.isObject()) {
        emit done(false, QString(), QString());
        return;
    }

    const QJsonObject release = document.object();

    QString version = release.value("tag_name").toString();
    const QString url = release.value("html_url").toString();

    // Tags are published as "v1.0.6"; QVersionNumber wants bare digits.
    if (version.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        version.remove(0, 1);
    }

    const auto remoteVersion  = QVersionNumber::fromString(version).normalized();
    const auto currentVersion = QVersionNumber::fromString(qApp->applicationVersion()).normalized();

    if (remoteVersion.isNull() || url.isEmpty() || remoteVersion <= currentVersion) {
        emit done(false, QString(), QString());
        return;
    }

    emit done(true, version, url);
}

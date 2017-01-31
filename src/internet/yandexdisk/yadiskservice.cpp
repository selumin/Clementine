/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <QNetworkReply>
#include <QNetworkRequest>

#include <qjson/parser.h>

#include "core/application.h"
#include "core/player.h"
#include "core/waitforsignal.h"
#include "internet/core/oauthenticator.h"
#include "internet/yandexdisk/yadiskservice.h"
#include "internet/yandexdisk/yadiskurlhandler.h"
#include "ui/iconloader.h"

const char* YandexDiskService::kServiceName = "YandexDisk";
const char* YandexDiskService::kSettingsGroup = "YandexDisk";

namespace {
static const char* kOAuthEndpoint =
    "https://oauth.yandex.ru/authorize";
static const char* kOAuthClientId = "36f9cfadbf8c4f87ae9fbead439a8a6d";
static const char* kOAuthClientSecret = "afb61b309d144fdab1c49fd2014c60b1";
static const char* kOAuthTokenEndpoint =
    "https://oauth.yandex.ru/token";
static const char* kOAuthScope = "";

static const char* kYandexDiskBase =
    "https://cloud-api.yandex.net:443/v1/disk/resources?";

static const char* kYandexDiskDownload =
    "https://cloud-api.yandex.net:443/v1/disk/resources/download?";

static const char* kRootFolder = "/";
}


YandexDiskService::YandexDiskService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kSettingsGroup,
                       IconLoader::Load("yandexdisk", IconLoader::Provider),
                       SettingsDialog::Page_YandexDisk) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  access_token_ = s.value("access_token").toString();
  expiry_time_ = s.value("expiry_time").toDateTime();

  app->player()->RegisterUrlHandler(new YandexDiskUrlHandler(this, this));
}

YandexDiskService::~YandexDiskService() {}

void YandexDiskService::StartAuthentification() {
  OAuthenticator* oauth =
      new OAuthenticator(kOAuthClientId, kOAuthClientSecret,
                         OAuthenticator::RedirectStyle::REMOTE_WITH_STATE,
                         this);

  oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint,
                                      kOAuthScope);

  NewClosure(oauth, SIGNAL(Finished()), this,
             SLOT(AuthenticationFinished(OAuthenticator*)), oauth);
}

void YandexDiskService::Connect() {
  if (!has_credentials()) {
    ShowSettingsDialog();
  } else {
    GetFileList(kRootFolder);
  }
}

void YandexDiskService::ForgetCredentials() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.remove("access_token");
  s.remove("expiry_time");
  s.remove("name");

  access_token_.clear();
}

void YandexDiskService::AuthenticationFinished(OAuthenticator* oauth) {
  oauth->deleteLater();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();

  s.setValue("access_token", oauth->access_token());
  s.setValue("expiry_time", oauth->expiry_time());

  emit Connected();

  GetFileList(kRootFolder);
}

void YandexDiskService::GetFileList(const QString& folder) {
  QUrl url(kYandexDiskBase);
  url.addQueryItem("path", folder);

  QNetworkRequest request(url);
  AddAuthorizationHeader(request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(GetFileListFinished(QNetworkReply*)), reply);
}

void YandexDiskService::GetFileListFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJson::Parser parser;

  QByteArray data = reply->readAll();
  QVariantMap response = parser.parse(data).toMap();

  QVariantList files = response["_embedded"].toMap()["items"].toList();
  for (const QVariant& f : files) {
    QVariantMap file = f.toMap();
    if (file["type"].toString() == "file" &&
        file["media_type"].toString() == "audio") {
      QString mime_type = GuessMimeTypeForFile(file["name"].toString());

      QUrl url;
      url.setPath(file["path"].toString());

      Song song;
      song.set_url(url);
      song.set_ctime(file["created"].toDateTime().toTime_t());
      song.set_mtime(file["modified"].toDateTime().toTime_t());
      song.set_filesize(file["size"].toInt());
      song.set_title(file["name"].toString());

      QUrl download_url = GetDownloadUrlForFile(file["path"].toString());

      MaybeAddFileToDatabase(song, mime_type, download_url, QString::null);
    } else if (file["type"].toString() == "dir") {
      GetFileList(file["path"].toString());
    }
  }
}

void YandexDiskService::AddAuthorizationHeader(QNetworkRequest& request) {
  request.setRawHeader("Authorization",
                        QString("OAuth %1").arg(access_token_).toUtf8());
}

bool YandexDiskService::has_credentials() const {
  return !access_token_.isEmpty() && expiry_time_.isValid() &&
      QDateTime::currentDateTime().secsTo(expiry_time_) > 0;
}

QUrl YandexDiskService::GetDownloadUrlForFile(const QString& file) {
  EnsureConnected();

  QUrl url(kYandexDiskDownload);
  url.addQueryItem("path", file);

  QNetworkRequest request(url);
  AddAuthorizationHeader(request);

  std::unique_ptr<QNetworkReply> reply(network_->get(request));
  WaitForSignal(reply.get(), SIGNAL(finished()));

  QJson::Parser parser;
  QVariantMap response = parser.parse(reply->readAll()).toMap();

  return QUrl::fromEncoded(response["href"].toByteArray());
}

void YandexDiskService::EnsureConnected() {
  if (!access_token_.isEmpty()) {
    return;
  }

  StartAuthentification();
  WaitForSignal(this, SIGNAL(Connected()));
}

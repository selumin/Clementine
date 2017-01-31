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

#ifndef INTERNET_YANDEXDISK_YADISKSERVICE_H_
#define INTERNET_YANDEXDISK_YADISKSERVICE_H_

#include "internet/core/cloudfileservice.h"

class OAuthenticator;
class QNetworkReply;
class QNetworkRequest;
class QUrl;

class YandexDiskService : public CloudFileService {
  Q_OBJECT

public:
  YandexDiskService(Application* name, InternetModel* model);
  ~YandexDiskService();

  static const char* kServiceName;
  static const char* kSettingsGroup;

  bool has_credentials() const override;

  QUrl GetDownloadUrlForFile(const QString& file);

  void StartAuthentification();

signals:
  void Connected();

public slots:
  void Connect() override;
  void ForgetCredentials();

private slots:
  void AuthenticationFinished(OAuthenticator* oauth);
  void GetFileListFinished(QNetworkReply* reply);

private:
  void AddAuthorizationHeader(QNetworkRequest& request);
  void GetFileList(const QString& folder);
  void EnsureConnected();

  QString     access_token_;
  QDateTime   expiry_time_;

};

#endif // INTERNET_YANDEXDISK_YADISKSERVICE_H_

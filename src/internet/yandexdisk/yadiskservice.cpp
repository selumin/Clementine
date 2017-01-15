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

#include "yadiskservice.h"
#include "ui/iconloader.h"

const char* YandexDiskService::kServiceName = "Yandex.Disk";
const char* YandexDiskService::kSettingsGroup = "Yandex.Disk";

//static const char* kAuthTokenUrl = "https://oauth.yandex.ru/authorize?";

YandexDiskService::YandexDiskService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kSettingsGroup,
                       IconLoader::Load("yandexdisk", IconLoader::Provider),
                       SettingsDialog::Page_YandexDisk) {
  QSettings s;
  access_token_ = s.value("access_token").toString();
}

YandexDiskService::~YandexDiskService() {}

void YandexDiskService::Connect() {
  if (has_credentials()) {
    GetFileList();
  } else {
    ShowSettingsDialog();
  }
}

void YandexDiskService::ForgetCredentials() {
  access_token_.clear();
}

void YandexDiskService::GetFileList() {

}

bool YandexDiskService::has_credentials() const {
  return !access_token_.isEmpty();
}

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

#include "yadiskurlhandler.h"
#include "yadiskservice.h"

YandexDiskUrlHandler::YandexDiskUrlHandler(YandexDiskService* service,
                                       QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult YandexDiskUrlHandler::StartLoading(const QUrl& url) {
  QString file_id(url.path());
  qDebug() << file_id;
  QUrl real_url = service_->GetDownloadUrlForFile(file_id);
  return LoadResult(url, LoadResult::TrackAvailable, real_url);
}

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

#include "yadisksettingspage.h"

#include <QSortFilterProxyModel>

#include "ui_yadisksettingspage.h"
#include "core/application.h"
#include "internet/yandexdisk/yadiskservice.h"
#include "internet/core/internetmodel.h"
#include "ui/settingsdialog.h"
#include "ui/iconloader.h"

YandexDiskSettingsPage::YandexDiskSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::YandexDiskSettingsPage),
      service_(dialog()->app()->internet_model()->Service<YandexDiskService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("yandexdisk", IconLoader::Provider));

  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(service_, SIGNAL(Connected()), SLOT(Connected()));

  dialog()->installEventFilter(this);
}

YandexDiskSettingsPage::~YandexDiskSettingsPage() { delete ui_; }

void YandexDiskSettingsPage::Load() {
  QSettings s;
  s.beginGroup(YandexDiskService::kSettingsGroup);

  const QString name = s.value("name").toString();

  if (service_->has_credentials() && !name.isEmpty()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, name);
  }
}

void YandexDiskSettingsPage::Save() {
  QSettings s;
  s.beginGroup(YandexDiskService::kSettingsGroup);
}

void YandexDiskSettingsPage::LoginClicked() {
  service_->Connect();
  ui_->login_button->setEnabled(false);
}

bool YandexDiskSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void YandexDiskSettingsPage::LogoutClicked() {
  service_->ForgetCredentials();
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
}

void YandexDiskSettingsPage::Connected() {
  QSettings s;
  s.beginGroup(YandexDiskService::kSettingsGroup);

  const QString name = s.value("name").toString();

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, name);
}

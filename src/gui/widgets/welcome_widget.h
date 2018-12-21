/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "gui/widgets/base_widget.h"

#include <common/error.h>

class QToolBar;
class QLabel;
class QPushButton;
class QHBoxLayout;

namespace fastonosql {
namespace gui {

class WelcomeWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  static const QSize kIconSize;
  enum { min_width = 640, min_height = 480 };

 protected:
  explicit WelcomeWidget(QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;
  void showEvent(QShowEvent* ev) override;

 private Q_SLOTS:
  void pageLoad(common::Error err, const QString& content);
  void openGithub() const;
  void openTwitter() const;
  void openFacebook() const;
  void openYoutube() const;
  void openInstagram() const;
  void openGetStartedNow() const;
  void openEmail() const;
  void openHomePage() const;

 private:
  void loadPage();
  void setHtml(const QString& html);
  QHBoxLayout* createSocialButtons();

  QLabel* page_label_;
  QLabel* help_title_;
  QLabel* help_description_;
  QPushButton* get_started_now_button_;
  QLabel* social_title_;

  QPushButton* open_github_action_;
  QPushButton* open_twitter_action_;
  QPushButton* open_facebook_action_;
  QPushButton* open_youtube_action_;
  QPushButton* open_instagram_action_;
  QPushButton* open_email_action_;
  QPushButton* open_home_page_action_;
};

}  // namespace gui
}  // namespace fastonosql

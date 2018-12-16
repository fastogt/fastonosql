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

#include <QWidget>

class QToolBar;
class QLabel;
class QPushButton;
class QHBoxLayout;

namespace fastonosql {
namespace gui {

class WelcomeWidget : public QWidget {
  Q_OBJECT

 public:
  typedef QWidget base_class;
  static const QSize kIconSize;
  enum { min_width = 640, min_height = 480 };
  explicit WelcomeWidget(QWidget* parent = Q_NULLPTR);

 protected:
  void showEvent(QShowEvent* ev) override;
  void changeEvent(QEvent* ev) override;

 private Q_SLOTS:
  void pageLoad(const QString& content, const QString& error_message);
  void openGithub() const;
  void openTwitter() const;
  void openFacebook() const;
  void openYoutube() const;
  void openInstagram() const;
  void openGetStartedNow() const;
  void openEmail() const;
  void openHomePage() const;

 private:
  void retranslateUi();
  void loadPage();
  void setHtml(const QString& html);
  QHBoxLayout* createToolBar();

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

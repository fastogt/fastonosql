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

#include "gui/widgets/welcome_widget.h"

#include <QDesktopServices>
#include <QEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QThread>
#include <QToolBar>
#include <QUrl>

#include <common/macros.h>

#include "gui/gui_factory.h"
#include "gui/workers/load_welcome_page.h"

namespace {
const QString trDontKnowHowToUse = QObject::tr("<b>Don't know how to use?</b>");
const QString trLearHowTo = QObject::tr("Learn how to use <b>" PROJECT_NAME_TITLE "</b>.");
const QString trGetStartedNow = QObject::tr("Get Started Now");
const QString trSocial = QObject::tr("<b>Do you want to know more about us?</b>");
}  // namespace

namespace fastonosql {
namespace gui {

WelcomeWidget::WelcomeWidget(QWidget* parent) : base_class(parent) {
  page_label_ = new QLabel;
  page_label_->setTextFormat(Qt::RichText);
  page_label_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  page_label_->setOpenExternalLinks(true);

  help_title_ = new QLabel;
  help_description_ = new QLabel;

  get_started_now_button_ = new QPushButton;
  VERIFY(connect(get_started_now_button_, &QPushButton::clicked, this, &WelcomeWidget::openGetStartedNow));

  social_title_ = new QLabel;

  QVBoxLayout* help_panel = new QVBoxLayout;
  help_panel->addWidget(help_title_);
  help_panel->addWidget(help_description_);
  help_panel->addWidget(get_started_now_button_);
  QSplitter* hs = new QSplitter(Qt::Vertical);
  help_panel->addWidget(hs);
  help_panel->addWidget(social_title_);
  QToolBar* help_bar = createToolBar();
  help_panel->addWidget(help_bar);

  QHBoxLayout* main_layout = new QHBoxLayout;
  main_layout->addWidget(page_label_, 1);
  main_layout->addLayout(help_panel, 0);

  setLayout(main_layout);
  setMinimumSize(QSize(min_width, min_height));
  retranslateUi();
}

void WelcomeWidget::showEvent(QShowEvent* ev) {
  base_class::showEvent(ev);
  static bool page_loaded = false;
  if (!page_loaded) {
    loadPage();
    page_loaded = true;
  }
}

void WelcomeWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  base_class::changeEvent(ev);
}

void WelcomeWidget::retranslateUi() {
  help_title_->setText(trDontKnowHowToUse);
  help_description_->setText(trLearHowTo);
  get_started_now_button_->setText(trGetStartedNow);
  social_title_->setText(trSocial);
}

void WelcomeWidget::pageLoad(const QString& content, const QString& error_message) {
  if (!error_message.isEmpty()) {
    QFile file(":" PROJECT_NAME_LOWERCASE "/welcome.html");
    file.open(QFile::ReadOnly);
    const QString file_content = file.readAll();
    setHtml(file_content);
    return;
  }

  setHtml(content);
}

void WelcomeWidget::openGithub() const {
  QDesktopServices::openUrl(QUrl(PROJECT_GITHUB_URL));
}

void WelcomeWidget::openTwitter() const {
  QDesktopServices::openUrl(QUrl(PROJECT_TWITTER_URL));
}

void WelcomeWidget::openFacebook() const {
  QDesktopServices::openUrl(QUrl(PROJECT_FACEBOOK_URL));
}

void WelcomeWidget::openYoutube() const {
  QDesktopServices::openUrl(QUrl(PROJECT_YOUTUBE_URL));
}

void WelcomeWidget::openInstagram() const {
  QDesktopServices::openUrl(QUrl(PROJECT_INSTAGRAM_URL));
}

void WelcomeWidget::openGetStartedNow() const {
  QDesktopServices::openUrl(QUrl(PROJECT_HOW_TO_USE_URL));
}

void WelcomeWidget::openEmail() const {
  QDesktopServices::openUrl(QUrl(PROJECT_EMAIL_URL));
}

void WelcomeWidget::openHomePage() const {
  QDesktopServices::openUrl(QUrl(PROJECT_DOMAIN));
}

void WelcomeWidget::loadPage() {
  QThread* th = new QThread;
  LoadWelcomePage* sender = new LoadWelcomePage;
  sender->moveToThread(th);
  VERIFY(connect(th, &QThread::started, sender, &LoadWelcomePage::routine));
  VERIFY(connect(sender, &LoadWelcomePage::pageLoaded, this, &WelcomeWidget::pageLoad));
  VERIFY(connect(sender, &LoadWelcomePage::pageLoaded, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, sender, &LoadWelcomePage::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

void WelcomeWidget::setHtml(const QString& html) {
  page_label_->setText(html);
}

QToolBar* WelcomeWidget::createToolBar() {
  QToolBar* help = new QToolBar;
  open_facebook_action_ = new QAction(this);
  open_facebook_action_->setIcon(gui::GuiFactory::GetInstance().facebookIcon());
  VERIFY(connect(open_facebook_action_, &QAction::triggered, this, &WelcomeWidget::openFacebook));
  help->addAction(open_facebook_action_);

  open_github_action_ = new QAction(this);
  open_github_action_->setIcon(gui::GuiFactory::GetInstance().githubIcon());
  VERIFY(connect(open_github_action_, &QAction::triggered, this, &WelcomeWidget::openGithub));
  help->addAction(open_github_action_);

  open_twitter_action_ = new QAction(this);
  open_twitter_action_->setIcon(gui::GuiFactory::GetInstance().twitterIcon());
  VERIFY(connect(open_twitter_action_, &QAction::triggered, this, &WelcomeWidget::openTwitter));
  help->addAction(open_twitter_action_);

  open_home_page_action_ = new QAction(this);
  open_home_page_action_->setIcon(gui::GuiFactory::GetInstance().homePageIcon());
  VERIFY(connect(open_home_page_action_, &QAction::triggered, this, &WelcomeWidget::openHomePage));
  help->addAction(open_home_page_action_);

  open_email_action_ = new QAction(this);
  open_email_action_->setIcon(gui::GuiFactory::GetInstance().emailIcon());
  VERIFY(connect(open_email_action_, &QAction::triggered, this, &WelcomeWidget::openEmail));
  help->addAction(open_email_action_);

  open_youtube_action_ = new QAction(this);
  open_youtube_action_->setIcon(gui::GuiFactory::GetInstance().youtubeIcon());
  VERIFY(connect(open_youtube_action_, &QAction::triggered, this, &WelcomeWidget::openYoutube));
  help->addAction(open_youtube_action_);

  open_instagram_action_ = new QAction(this);
  open_instagram_action_->setIcon(gui::GuiFactory::GetInstance().instagramIcon());
  VERIFY(connect(open_instagram_action_, &QAction::triggered, this, &WelcomeWidget::openInstagram));
  help->addAction(open_instagram_action_);
  return help;
}

}  // namespace gui
}  // namespace fastonosql

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
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QThread>
#include <QToolBar>
#include <QUrl>

#include <common/macros.h>

#include "gui/gui_factory.h"
#include "gui/workers/load_welcome_page.h"

namespace {
const QString trDontKnowHowToUse = QObject::tr("<b>Don't know how to use?</b>");
}

namespace fastonosql {
namespace gui {

WelcomeWidget::WelcomeWidget(QWidget* parent) : base_class(parent) {
  page_label_ = new QLabel;
  page_label_->setTextFormat(Qt::RichText);
  page_label_->setTextInteractionFlags(Qt::TextBrowserInteraction);
  page_label_->setOpenExternalLinks(true);

  help_title_ = new QLabel;
  help_title_->setFixedWidth(min_width / 4);
  help_title_->setAlignment(Qt::AlignHCenter);

  QVBoxLayout* help_panel = new QVBoxLayout;
  help_panel->addWidget(help_title_);
  QSplitter* hs = new QSplitter(Qt::Vertical);
  help_panel->addWidget(hs);
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
}

void WelcomeWidget::pageLoad(const QString& content, const QString& error_message) {
  if (!error_message.isEmpty()) {
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

/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/view_keys_dialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QEvent>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QScrollBar>
#include <QSplitter>
#include <QStyledItemDelegate>

#include "common/qt/convert_string.h"
#include "common/logger.h"

#include "core/iserver.h"
#include "core/idatabase.h"

#include "gui/keys_table_model.h"
#include "gui/fasto_table_view.h"
#include "gui/gui_factory.h"

#include "translations/global.h"

namespace {

QPushButton *createButtonWithIcon(const QIcon &icon) {
  QPushButton *button = new QPushButton;
  button->setIcon(icon);
  button->setFixedSize(24, 24);
  button->setFlat(true);
  return button;
}

class NumericDelegate
  : public QStyledItemDelegate
{
public:
explicit NumericDelegate(QObject *parent = 0)
  : QStyledItemDelegate(parent) {
}

QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  QSpinBox* editor = new QSpinBox(parent);
  editor->setRange(-1, INT32_MAX);
  editor->setValue(-1);
  return editor;
}

void setEditorData(QWidget *editor, const QModelIndex &index) const {
  int value = index.model()->data(index, Qt::EditRole).toInt();

  QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
  spinBox->setValue(value);
}

void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
  spinBox->interpretText();
  int value = spinBox->value();

  model->setData(index, value, Qt::EditRole);
}

void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                          const QModelIndex &) const {
  editor->setGeometry(option.rect);
}
};

}

namespace fastonosql {

ViewKeysDialog::ViewKeysDialog(const QString &title, IDatabaseSPtr db, QWidget* parent)
  : QDialog(parent), db_(db), cursorStack_(), curPos_(0) {
  DCHECK(db_);
  if(db_){
      IServerSPtr serv = db_->server();
      VERIFY(connect(serv.get(), &IServer::startedLoadDataBaseContent,
                     this, &ViewKeysDialog::startLoadDatabaseContent));
      VERIFY(connect(serv.get(), &IServer::finishedLoadDatabaseContent,
                     this, &ViewKeysDialog::finishLoadDatabaseContent));
  }

  setWindowTitle(title);

  // main layout
  QVBoxLayout *mainlayout = new QVBoxLayout;

  QHBoxLayout* searchLayout = new QHBoxLayout;
  searchBox_ = new QLineEdit;
  searchBox_->setText("*");
  VERIFY(connect(searchBox_, &QLineEdit::textChanged, this, &ViewKeysDialog::searchLineChanged));
  searchLayout->addWidget(searchBox_);

  countSpinEdit_ = new QSpinBox;
  countSpinEdit_->setRange(min_key_on_page, max_key_on_page);
  countSpinEdit_->setSingleStep(step_keys_on_page);
  countSpinEdit_->setValue(defaults_key);

  keyCountLabel_ = new QLabel;

  searchLayout->addWidget(keyCountLabel_);
  searchLayout->addWidget(countSpinEdit_);

  searchButton_ = new QPushButton;
  VERIFY(connect(searchButton_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
  searchLayout->addWidget(searchButton_);

  keysModel_ = new KeysTableModel(this);
  VERIFY(connect(keysModel_, &KeysTableModel::changedValue, this, &ViewKeysDialog::executeCommand, Qt::DirectConnection));
  if(db_){
      IServerSPtr serv = db_->server();
      VERIFY(connect(serv.get(), &IServer::startedExecuteCommand, this, &ViewKeysDialog::startExecuteCommand, Qt::DirectConnection));
      VERIFY(connect(serv.get(), &IServer::finishedExecuteCommand, this, &ViewKeysDialog::finishExecuteCommand, Qt::DirectConnection));
  }
  keysTable_ = new FastoTableView;
  keysTable_->setModel(keysModel_);
  keysTable_->setItemDelegateForColumn(KeyTableItem::kTTL, new NumericDelegate(this));

  QDialogButtonBox* buttonBox = new QDialogButtonBox;
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ViewKeysDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ViewKeysDialog::reject));
  mainlayout->addLayout(searchLayout);
  mainlayout->addWidget(keysTable_);

  leftButtonList_ = createButtonWithIcon(GuiFactory::instance().leftIcon());
  rightButtonList_ = createButtonWithIcon(GuiFactory::instance().rightIcon());
  VERIFY(connect(leftButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::leftPageClicked));
  VERIFY(connect(rightButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
  QHBoxLayout* pagingLayout = new QHBoxLayout;
  pagingLayout->addWidget(leftButtonList_);
  DataBaseInfoSPtr inf = db_->info();
  size_t sizeKey = inf->sizeDB();
  currentKey_ = new QSpinBox;
  currentKey_->setEnabled(false);
  currentKey_->setValue(0);
  currentKey_->setMinimum(0);
  currentKey_->setMaximum(sizeKey);
  countKey_ = new QSpinBox;
  countKey_->setEnabled(false);
  countKey_->setValue(sizeKey);
  pagingLayout->addWidget(new QSplitter(Qt::Horizontal));
  pagingLayout->addWidget(currentKey_);
  pagingLayout->addWidget(countKey_);
  pagingLayout->addWidget(new QSplitter(Qt::Horizontal));
  pagingLayout->addWidget(rightButtonList_);

  mainlayout->addLayout(pagingLayout);
  mainlayout->addWidget(buttonBox);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainlayout);

  updateControls();
  retranslateUi();
}

void ViewKeysDialog::startLoadDatabaseContent(const EventsInfo::LoadDatabaseContentRequest& req) {
  keysModel_->clear();
}

void ViewKeysDialog::finishLoadDatabaseContent(const EventsInfo::LoadDatabaseContentResponce& res) {
  common::Error er = res.errorInfo();
  if(er && er->isError()){
    return;
  }

  if (!keysModel_) {
    return;
  }

  EventsInfo::LoadDatabaseContentResponce::keys_cont_type keys = res.keys_;

  size_t size = keys.size();
  for(size_t i = 0; i < size; ++i){
      NDbKValue key = keys[i];
      keysModel_->insertItem(new KeyTableItem(key));
  }

  int curv = currentKey_->value();
  if (cursorStack_.size() == curPos_) {
      cursorStack_.push_back(res.cursorOut_);
      currentKey_->setValue(curv + size);
  } else {
    currentKey_->setValue(curv - size);
  }

  updateControls();
}

void ViewKeysDialog::executeCommand(CommandKeySPtr cmd) {
  if(db_){
    EventsInfo::CommandRequest req(this, db_->info(), cmd);
    db_->executeCommand(req);
  }
}

void ViewKeysDialog::startExecuteCommand(const EventsInfo::CommandRequest& req) {
}

void ViewKeysDialog::finishExecuteCommand(const EventsInfo::CommandResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  if (res.initiator() != this) {
    DEBUG_MSG_FORMAT<512>(common::logging::L_DEBUG, "Skipped event in file: %s, function: %s", __FILE__, __FUNCTION__);
    return;
  }

  CommandKeySPtr key = res.cmd_;
  if (key->type() == CommandKey::C_CHANGE_TTL) {
    CommandChangeTTL * cttl = dynamic_cast<CommandChangeTTL*>(key.get());
    if (cttl) {
      NDbKValue dbv = cttl->newKey();
      keysModel_->changeValue(dbv);
    }
  }
}

void ViewKeysDialog::search(bool forward) {
  if (!db_) {
    return;
  }

  QString pattern = searchBox_->text();
  if (pattern.isEmpty()) {
    return;
  }

  if(cursorStack_.empty()){
    cursorStack_.push_back(0);
  }

  DCHECK(cursorStack_[0] == 0);
  if (forward) {
    EventsInfo::LoadDatabaseContentRequest req(this, db_->info(),
                                               common::convertToString(pattern), countSpinEdit_->value(), cursorStack_[curPos_]);
    db_->loadContent(req);
    ++curPos_;
  } else {
    if(curPos_ > 0){
      EventsInfo::LoadDatabaseContentRequest req(this, db_->info(),
                                                 common::convertToString(pattern), countSpinEdit_->value(), cursorStack_[--curPos_]);
      db_->loadContent(req);
    }
  }
}

void ViewKeysDialog::searchLineChanged(const QString& text) {
  cursorStack_.clear();
  curPos_ = 0;
  currentKey_->setValue(0);
  updateControls();
}

void ViewKeysDialog::leftPageClicked() {
  search(false);
}

void ViewKeysDialog::rightPageClicked() {
  search(true);
}

void ViewKeysDialog::changeEvent(QEvent* e) {
  if(e->type() == QEvent::LanguageChange){
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void ViewKeysDialog::retranslateUi() {
  using namespace translations;
  keyCountLabel_->setText(trKeyCountOnThePage);
  searchButton_->setText(trSearch);
}

void ViewKeysDialog::updateControls() {
  bool isEmptyDb = keysCount() == 0;
  bool isEndSearch = curPos_ ? (cursorStack_[curPos_] == 0) : false;

  leftButtonList_->setEnabled(curPos_ > 0);
  rightButtonList_->setEnabled(!isEmptyDb && !isEndSearch);
  searchButton_->setEnabled(!isEmptyDb && !isEndSearch);
}

size_t ViewKeysDialog::keysCount() const {
  return countKey_->value();
}

}

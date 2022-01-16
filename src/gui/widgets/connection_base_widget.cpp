/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/widgets/connection_base_widget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>

#include "gui/gui_factory.h"

#include "translations/global.h"

namespace {
const char* kDefaultNameConnectionFolder = "/";
const QString trLoggingToolTip = QObject::tr("INFO command timeout in msec for history statistic.");

QString toRawCommandLine(QString input) {
  return input.replace("\\n", "\n").replace("\\r", "\r");
}

QString fromRawCommandLine(QString input) {
  return input.replace("\n", "\\n").replace("\r", "\\r");
}

const QStringList kSeparators = {":", ";", ",", "[", "]"};
const QStringList kDelimiters = {"\\n", "\\r\\n"};

class UniqueCharValidator : public QValidator {
 public:
  explicit UniqueCharValidator(QObject* parent = Q_NULLPTR) : QValidator(parent) {}

  virtual State validate(QString& val, int& pos) const override {
    UNUSED(pos);
    if (val.isEmpty()) {
      return Invalid;
    }

    for (int i = 0; i < val.length(); i++) {
      for (int j = i + 1; j < val.length(); j++) {
        if (val[i] == val[j]) {
          return Invalid;
        }
      }
    }

    return Acceptable;
  }
};
}  // namespace

namespace fastonosql {
namespace gui {

ConnectionBaseWidget::ConnectionBaseWidget(QWidget* parent) : base_class(parent) {
  connection_name_ = new QLineEdit;
  connection_name_label_ = new QLabel;

  namespace_separator_label_ = new QLabel;
  namespace_separator_ = new QComboBox;
  namespace_separator_->addItems(kSeparators);
  namespace_separator_->setEditable(true);
  namespace_separator_->setValidator(new UniqueCharValidator(this));
  namespace_displaying_strategy_label_ = new QLabel;
  namespace_displaying_strategy_ = new QComboBox;
  for (uint32_t i = 0; i < proxy::g_display_strategy_types.size(); ++i) {
    namespace_displaying_strategy_->addItem(proxy::g_display_strategy_types[i], i);
  }
  delimiter_label_ = new QLabel;
  delimiter_ = new QComboBox;
  delimiter_->addItems(kDelimiters);

  connection_folder_ = new QLineEdit;
  QRegExp rxf("^/[A-z0-9]+/$");
  connection_folder_->setValidator(new QRegExpValidator(rxf, this));
  folder_label_ = new QLabel;

  logging_ = new QCheckBox;
  logging_msec_ = new QSpinBox;
  logging_msec_->setRange(0, INT32_MAX);
  logging_msec_->setSingleStep(1000);
  VERIFY(connect(logging_, &QCheckBox::stateChanged, this, &ConnectionBaseWidget::loggingStateChange));
  logging_msec_->setEnabled(false);

  QHBoxLayout* connection_name_layout = new QHBoxLayout;
  connection_name_layout->addWidget(connection_name_label_);
  connection_name_layout->addWidget(connection_name_);

  // ns
  QHBoxLayout* namespace_separator_layout = new QHBoxLayout;
  namespace_separator_layout->addWidget(namespace_separator_label_);
  namespace_separator_layout->addWidget(namespace_separator_);

  // ns strategy
  QHBoxLayout* namespace_strategy_layout = new QHBoxLayout;
  namespace_strategy_layout->addWidget(namespace_displaying_strategy_label_);
  namespace_strategy_layout->addWidget(namespace_displaying_strategy_);

  QVBoxLayout* namespace_start_layout = new QVBoxLayout;
  namespace_start_layout->addLayout(namespace_separator_layout);
  namespace_start_layout->addLayout(namespace_strategy_layout);

  QHBoxLayout* namespace_delimiter_layout = new QHBoxLayout;
  namespace_delimiter_layout->addWidget(delimiter_label_);
  namespace_delimiter_layout->addWidget(delimiter_);

  // namesapce
  QHBoxLayout* namespace_layout = new QHBoxLayout;
  namespace_layout->addLayout(namespace_start_layout);
  namespace_layout->addLayout(namespace_delimiter_layout);

  QHBoxLayout* folderLayout = new QHBoxLayout;
  folderLayout->addWidget(folder_label_);
  folderLayout->addWidget(connection_folder_);

  QHBoxLayout* logging_layout = new QHBoxLayout;
  logging_layout->addWidget(logging_);
  QHBoxLayout* logging_vlayout = new QHBoxLayout;
  logging_vlayout->addWidget(new QLabel(QObject::tr("msec:")));
  logging_vlayout->addWidget(logging_msec_);
  logging_layout->addWidget(new QSplitter(Qt::Horizontal));
  logging_layout->addLayout(logging_vlayout);

  QVBoxLayout* main = new QVBoxLayout;
  main->addLayout(connection_name_layout);
  main->addLayout(namespace_layout);
  main->addLayout(folderLayout);
  main->addLayout(logging_layout);
  setLayout(main);
}

void ConnectionBaseWidget::addWidget(QWidget* widget) {
  QVBoxLayout* mainLayout = static_cast<QVBoxLayout*>(layout());
  mainLayout->addWidget(widget);
}

void ConnectionBaseWidget::addLayout(QLayout* l) {
  QVBoxLayout* mainLayout = static_cast<QVBoxLayout*>(layout());
  mainLayout->addLayout(l);
}

QString ConnectionBaseWidget::connectionName() const {
  return connection_name_->text();
}

proxy::IConnectionSettingsBase* ConnectionBaseWidget::createConnection() const {
  std::string conName = common::ConvertToString(connectionName());
  std::string conFolder = common::ConvertToString(UIFolderText());
  if (conFolder.empty()) {
    conFolder = kDefaultNameConnectionFolder;
  }

  proxy::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  proxy::IConnectionSettingsBase* conn = createConnectionImpl(path);
  conn->SetNsSeparator(common::ConvertToString(namespace_separator_->currentText()));
  conn->SetNsDisplayStrategy(static_cast<proxy::NsDisplayStrategy>(namespace_displaying_strategy_->currentIndex()));
  QString raw_delemiter = toRawCommandLine(delimiter_->currentText());
  conn->SetDelimiter(common::ConvertToString(raw_delemiter));
  if (isLogging()) {
    conn->SetLoggingMsTimeInterval(loggingInterval());
  }

  return conn;
}

void ConnectionBaseWidget::setConnectionName(const QString& name) {
  connection_name_->setText(name);
}

void ConnectionBaseWidget::syncControls(proxy::IConnectionSettingsBase* connection) {
  if (connection) {
    proxy::connection_path_t path = connection->GetPath();
    QString qname;
    if (common::ConvertFromString(path.GetName(), &qname)) {
      setConnectionName(qname);
    }

    std::string ns_separator = connection->GetNsSeparator();
    std::string delemiter = connection->GetDelimiter();
    QString qns_separator;
    common::ConvertFromString(ns_separator, &qns_separator);
    namespace_separator_->setCurrentText(qns_separator);
    namespace_displaying_strategy_->setCurrentIndex(connection->GetNsDisplayStrategy());
    QString qdelemiter;
    common::ConvertFromString(delemiter, &qdelemiter);
    delimiter_->setCurrentText(fromRawCommandLine(qdelemiter));

    QString qdir;
    common::ConvertFromString(path.GetDirectory(), &qdir);

    setUIFolderText(qdir);
    setLogging(connection->IsHistoryEnabled());
    setLoggingInterval(connection->GetLoggingMsTimeInterval());
  }
}

void ConnectionBaseWidget::retranslateUi() {
  connection_name_label_->setText(translations::trName + ":");
  namespace_separator_label_->setText(tr("Ns separator:"));
  namespace_displaying_strategy_label_->setText(tr("Ns display:"));
  delimiter_label_->setText(tr("Delimiter:"));
  folder_label_->setText(tr("UI Folder:"));
  logging_->setText(translations::trLoggingEnabled);
  logging_msec_->setToolTip(trLoggingToolTip);
}

bool ConnectionBaseWidget::validated() const {
  return true;
}

QString ConnectionBaseWidget::UIFolderText() const {
  return connection_folder_->text();
}

void ConnectionBaseWidget::setUIFolderText(const QString& text) {
  connection_folder_->setText(text);
}

void ConnectionBaseWidget::setUIFolderEnabled(bool val) {
  connection_folder_->setEnabled(val);
}

bool ConnectionBaseWidget::isLogging() const {
  return logging_->isChecked();
}

void ConnectionBaseWidget::setLogging(bool logging) {
  logging_->setChecked(logging);
}

int ConnectionBaseWidget::loggingInterval() const {
  return logging_msec_->value();
}

void ConnectionBaseWidget::setLoggingInterval(int val) {
  logging_msec_->setValue(val);
}

void ConnectionBaseWidget::loggingStateChange(int value) {
  logging_msec_->setEnabled(value);
}

}  // namespace gui
}  // namespace fastonosql

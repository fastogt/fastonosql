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

#include <fastonosql/core/connection_types.h>  // for ConnectionType
#include "proxy/proxy_fwd.h"                   // for IServerSPtr

class QGroupBox;

namespace fastonosql {
namespace gui {
class OutputWidget;
class BaseShellWidget;

class QueryWidget : public QWidget {
  Q_OBJECT
 public:
  enum { min_width = 640, min_height = 480 };
  explicit QueryWidget(proxy::IServerSPtr server, QWidget* parent = Q_NULLPTR);

  QueryWidget* clone(const QString& text);
  core::ConnectionType connectionType() const;
  QString inputText() const;
  void setInputText(const QString& text);

 public Q_SLOTS:
  void execute(const QString& text);
  void executeArgs(const QString& text, int repeat, int interval, bool history);
  void reload();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  QGroupBox* console_gb_;
  QGroupBox* output_gb_;

  BaseShellWidget* shell_widget_;
  OutputWidget* output_widget_;
  const proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql

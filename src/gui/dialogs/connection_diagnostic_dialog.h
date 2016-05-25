/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <QDialog>

#include "core/connection_settings.h"

class QMovie;
class QLabel;

namespace fasto {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}

namespace fastonosql {
namespace gui {

class TestConnection
  : public QObject {
  Q_OBJECT
 public:
  explicit TestConnection(core::IConnectionSettingsBaseSPtr conn, QObject* parent = 0);

 Q_SIGNALS:
  void connectionResult(bool suc, qint64 msTimeExecute, const QString& resultText);

 public Q_SLOTS:
  void routine();

 private:
  core::IConnectionSettingsBaseSPtr connection_;
  common::time64_t startTime_;
};

class ConnectionDiagnosticDialog
  : public QDialog {
  Q_OBJECT
 public:
  enum {
    fix_height = 160,
    fix_width = 240
  };

  ConnectionDiagnosticDialog(QWidget* parent, core::IConnectionSettingsBaseSPtr connection);

 private Q_SLOTS:
  void connectionResult(bool suc, qint64 mstimeExecute, const QString& resultText);

 protected:
  virtual void showEvent(QShowEvent* e);

 private:
  void testConnection(core::IConnectionSettingsBaseSPtr connection);

  fasto::qt::gui::GlassWidget* glassWidget_;
  QLabel* executeTimeLabel_;
  QLabel* statusLabel_;
  QLabel* iconLabel_;
};

}  // namespace gui
}  // namespace fastonosql

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

#include "gui/editor/fasto_editor.h"

#define JSON_VIEW 0
#define CSV_VIEW 1
#define RAW_VIEW 2
#define HEX_VIEW 3
#define UNICODE_VIEW 4
#define MSGPACK_VIEW 5
#define GZIP_VIEW 6
#define LZ4_VIEW 7
#define BZIP2_VIEW 8
#define SNAPPY_VIEW 9
#define XML_VIEW 10

namespace fastonosql {
namespace gui {
class FastoHexEdit;

class FastoEditorOutput : public QWidget {
  Q_OBJECT
 public:
  explicit FastoEditorOutput(QWidget* parent = Q_NULLPTR);
  virtual ~FastoEditorOutput();

  void setModel(QAbstractItemModel* model);

  QModelIndex selectedItem(int column) const;
  bool setData(const QModelIndex& index, const QVariant& value, int role);
  int viewMethod() const;
  QString text() const;
  bool isReadOnly() const;
  int childCount() const;

 Q_SIGNALS:
  void textChanged();
  void readOnlyChanged();

 public Q_SLOTS:
  void setReadOnly(bool ro);
  void viewChange(int viewMethod);

 private Q_SLOTS:
  void modelDestroyed();
  void dataChanged(QModelIndex first, QModelIndex last);
  void headerDataChanged();
  void rowsInserted(QModelIndex index, int r, int c);
  void rowsAboutToBeRemoved(QModelIndex index, int r, int c);
  void rowsRemoved(QModelIndex index, int r, int c);
  void columnsAboutToBeRemoved(QModelIndex index, int r, int c);
  void columnsRemoved(QModelIndex index, int r, int c);
  void columnsInserted(QModelIndex index, int r, int c);
  void reset();
  void layoutChanged();

 private:
  void SyncEditors();

  FastoEditor* text_json_editor_;
  QsciLexer* json_lexer_;
  QsciLexer* xml_lexer_;

  QAbstractItemModel* model_;
  int view_method_;
};

}  // namespace gui
}  // namespace fastonosql

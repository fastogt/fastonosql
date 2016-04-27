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

#include <QModelIndex>
#include <QWidget>

#define JSON 0
#define CSV 1
#define RAW 2
#define HEX 3
#define MSGPACK 4
#define GZIP 5

class QLineEdit;
class QToolButton;
class QPushButton;
class QCheckBox;
class QFrame;
class QsciLexer;
class QMenu;

namespace fasto {
namespace qt {
namespace gui {
class FastoScintilla;
}
}
}

namespace fastonosql {
namespace gui {

class FastoHexEdit;
class FastoEditor
  : public QWidget {
  Q_OBJECT
 public:
  enum {
    HeightFindPanel = 40
  };
  explicit FastoEditor(QWidget* parent = 0);
  virtual ~FastoEditor();

  void registerImage(int id, const QPixmap& im);

  QString text() const;
  QString selectedText() const;

 Q_SIGNALS:
  void textChanged();

 public Q_SLOTS:
  void append(const QString& text);
  void setReadOnly(bool ro);
  void setText(const QString& text);
  void clear();

 private Q_SLOTS:
  void goToNextElement();
  void goToPrevElement();

 protected:
  void setShowAutoCompletion(bool showA);
  QMenu* createStandardContextMenu();

  void setLexer(QsciLexer* lexer);
  QsciLexer* lexer() const;

  void setCallTipsStyle(int style);
  void sendScintilla(unsigned int msg, unsigned long wParam = 0, long lParam = 0);

  virtual void keyPressEvent(QKeyEvent* e);
  virtual bool eventFilter(QObject* object, QEvent* event);
  virtual void changeEvent(QEvent* ev);

 private:
  void retranslateUi();
  void findElement(bool forward);

  fasto::qt::gui::FastoScintilla* scin_;
  QFrame* findPanel_;
  QLineEdit* findLine_;
  QToolButton* close_;
  QPushButton* next_;
  QPushButton* prev_;
  QCheckBox* caseSensitive_;
};

class FastoEditorOutput
  : public QWidget {
  Q_OBJECT
 public:
  explicit FastoEditorOutput(const QString& delemitr, QWidget* parent = 0);

  void setModel(QAbstractItemModel* model);

  QModelIndex selectedItem(int column) const;
  bool setData(const QModelIndex& index, const QVariant& value);
  int viewMethod() const;
  QString text() const;
  bool isReadOnly() const;

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
  FastoHexEdit* editor_;
  QAbstractItemModel* model_;
  int view_method_;
  const QString delemitr_;
};

class FastoEditorShell
  : public FastoEditor {
  Q_OBJECT
 protected Q_SLOTS:
  void showContextMenu(const QPoint& pt);

 protected:
  explicit FastoEditorShell(bool showAutoCompl, QWidget* parent = 0);
};

}  // namespace gui
}  // namespace fastonosql

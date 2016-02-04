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

#include "gui/fasto_editor.h"

#include <QMenu>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QKeyEvent>

#include "common/qt/convert_string.h"
#include "common/qt/utils_qt.h"

#include "gui/fasto_common_item.h"
#include "gui/gui_factory.h"
#include "gui/fasto_hex_edit.h"
#include "fasto/qt/gui/fasto_scintilla.h"

#include "translations/global.h"

namespace fastonosql {

FastoEditor::FastoEditor(QWidget* parent)
  : QWidget(parent), scin_(NULL) {
  QFont font = GuiFactory::instance().font();
  setFont(font);
  scin_ = new fasto::qt::gui::FastoScintilla;

  findPanel_ = new QFrame;
  findLine_ = new QLineEdit;
  close_ = new QToolButton;
  next_ = new QPushButton;
  prev_ = new QPushButton;
  caseSensitive_ = new QCheckBox;

  close_->setIcon(GuiFactory::instance().close16Icon());
  close_->setToolButtonStyle(Qt::ToolButtonIconOnly);
  close_->setIconSize(QSize(16, 16));
  findLine_->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(close_);
  layout->addWidget(findLine_);
  layout->addWidget(next_);
  layout->addWidget(prev_);
  layout->addWidget(caseSensitive_);

  findPanel_->setFixedHeight(HeightFindPanel);
  findPanel_->setLayout(layout);

  scin_->installEventFilter(this);

  QVBoxLayout *mainL = new QVBoxLayout;
  mainL->addWidget(scin_);
  mainL->addWidget(findPanel_);
  mainL->setContentsMargins(0, 0, 0, 0);
  setLayout(mainL);

  findPanel_->hide();

  VERIFY(connect(close_, &QToolButton::clicked, findPanel_, &QFrame::hide));
  VERIFY(connect(scin_, &fasto::qt::gui::FastoScintilla::textChanged,
                 this, &FastoEditor::textChanged));
  VERIFY(connect(next_, &QPushButton::clicked, this, &FastoEditor::goToNextElement));
  VERIFY(connect(prev_, &QPushButton::clicked, this, &FastoEditor::goToPrevElement));
  retranslateUi();
}

FastoEditor::~FastoEditor() {
}

void FastoEditor::registerImage(int id, const QPixmap &im) {
  scin_->registerImage(id, im);
}

QString FastoEditor::text() const {
  return scin_->text();
}

QString FastoEditor::selectedText() const {
  return scin_->selectedText();
}

void FastoEditor::setShowAutoCompletion(bool showA) {
  scin_->setShowAutoCompletion(showA);
}

QMenu* FastoEditor::createStandardContextMenu() {
  return scin_->createStandardContextMenu();
}

void FastoEditor::append(const QString &text) {
  scin_->append(text);
}

void FastoEditor::setReadOnly(bool ro) {
  scin_->setReadOnly(ro);
}

void FastoEditor::setText(const QString &text) {
  scin_->setText(text);
}

void FastoEditor::clear() {
  scin_->clear();
}

void FastoEditor::goToNextElement() {
  findElement(true);
}

void FastoEditor::goToPrevElement() {
  findElement(false);
}

void FastoEditor::setLexer(QsciLexer *lexer) {
  scin_->setLexer(lexer);
  scin_->setAutoCompletionCaseSensitivity(false);
}

QsciLexer *FastoEditor::lexer() const {
  return scin_->lexer();
}

void FastoEditor::setCallTipsStyle(int style) {
  scin_->setCallTipsStyle((QsciScintilla::CallTipsStyle)style);
}

void FastoEditor::sendScintilla(unsigned int msg, unsigned long wParam, long lParam) {
  scin_->SendScintilla(msg, wParam, lParam);
}

void FastoEditor::keyPressEvent(QKeyEvent* keyEvent) {
  bool isFocusScin = scin_->isActiveWindow();
  bool isShowFind = findPanel_->isVisible();
  if (keyEvent->key() == Qt::Key_Escape && isFocusScin && isShowFind) {
    findPanel_->hide();
    scin_->setFocus();
    keyEvent->accept();
  } else if (keyEvent->key() == Qt::Key_Return && (keyEvent->modifiers() & Qt::ShiftModifier) &&
             isFocusScin && isShowFind) {
        goToPrevElement();
  } else if (keyEvent->key() == Qt::Key_Return && isFocusScin && isShowFind) {
        goToNextElement();
  }

  QWidget::keyPressEvent(keyEvent);
}

bool FastoEditor::eventFilter(QObject* object, QEvent* event) {
  if (object == scin_) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = (QKeyEvent *)event;
      if (((keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() == Qt::Key_F)) {
        findPanel_->show();
        findLine_->setFocus();
        // findPanel_->selectAll();
        keyEvent->accept();
        return true;
      }
    }
  }

  return QWidget::eventFilter(object, event);
}

void FastoEditor::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void FastoEditor::retranslateUi() {
  next_->setText(translations::trNext);
  prev_->setText(translations::trPrevious);
  caseSensitive_->setText(translations::trMatchCase);
}

void FastoEditor::findElement(bool forward) {
  const QString &text = findLine_->text();
  if (!text.isEmpty()) {
    bool re = false;
    bool wo = false;
    bool looped = true;
    int index = 0;
    int line = 0;
    scin_->getCursorPosition(&line, &index);

    if (!forward) {
      index -= scin_->selectedText().length();
    }

    scin_->setCursorPosition(line, 0);
    bool isFounded = scin_->findFirst(text, re, caseSensitive_->checkState() == Qt::Checked,
                                      wo, looped, forward, line, index);

    if (isFounded) {
      scin_->ensureCursorVisible();
    } else {
      QMessageBox::warning(this, translations::trSearch, tr("The specified text was not found."));
    }
  }
}

FastoEditorOutput::FastoEditorOutput(const QString &delemitr, QWidget *parent)
  : QWidget(parent), model_(NULL), view_method_(JSON), delemitr_(delemitr) {
  QFont font = GuiFactory::instance().font();
  setFont(font);

  editor_ = new FastoHexEdit;
  VERIFY(connect(editor_, &FastoHexEdit::textChanged, this, &FastoEditorOutput::textChanged));

  QVBoxLayout *mainL = new QVBoxLayout;
  mainL->addWidget(editor_);
  mainL->setContentsMargins(0, 0, 0, 0);
  setLayout(mainL);
}

void FastoEditorOutput::setModel(QAbstractItemModel* model) {
  if (model == model_) {
    return;
  }

  if (model_) {
    VERIFY(disconnect(model_, &QAbstractItemModel::destroyed,
                      this, &FastoEditorOutput::modelDestroyed));
    VERIFY(disconnect(model_, &QAbstractItemModel::dataChanged,
                      this, &FastoEditorOutput::dataChanged));
    VERIFY(disconnect(model_, &QAbstractItemModel::headerDataChanged,
                      this, &FastoEditorOutput::headerDataChanged));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsInserted,
                      this, &FastoEditorOutput::rowsInserted));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsAboutToBeRemoved,
                      this, &FastoEditorOutput::rowsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsRemoved,
                      this, &FastoEditorOutput::rowsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsAboutToBeRemoved,
                      this, &FastoEditorOutput::columnsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsRemoved,
                      this, &FastoEditorOutput::columnsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsInserted,
                      this, &FastoEditorOutput::columnsInserted));
    VERIFY(disconnect(model_, &QAbstractItemModel::modelReset,
                      this, &FastoEditorOutput::reset));
    VERIFY(disconnect(model_, &QAbstractItemModel::layoutChanged,
                      this, &FastoEditorOutput::layoutChanged));
  }

  model_ = model;

  // These asserts do basic sanity checking of the model
  Q_ASSERT_X(model_->index(0,0) == model_->index(0,0),
             "QAbstractItemView::setModel",
             "A model should return the exact same index "
             "(including its internal id/pointer) when asked for it twice in a row.");
  Q_ASSERT_X(model_->index(0,0).parent() == QModelIndex(),
             "QAbstractItemView::setModel",
             "The parent of a top level index should be invalid");

  if (model_) {
    VERIFY(connect(model_, &QAbstractItemModel::destroyed,
                   this, &FastoEditorOutput::modelDestroyed));
    VERIFY(connect(model_, &QAbstractItemModel::dataChanged,
                   this, &FastoEditorOutput::dataChanged));
    VERIFY(connect(model_, &QAbstractItemModel::headerDataChanged,
                   this, &FastoEditorOutput::headerDataChanged));
    VERIFY(connect(model_, &QAbstractItemModel::rowsInserted,
                   this, &FastoEditorOutput::rowsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::rowsAboutToBeRemoved,
                   this, &FastoEditorOutput::rowsAboutToBeRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::rowsRemoved,
                   this, &FastoEditorOutput::rowsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsAboutToBeRemoved,
                   this, &FastoEditorOutput::columnsAboutToBeRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsRemoved,
                   this, &FastoEditorOutput::columnsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsInserted,
                   this, &FastoEditorOutput::columnsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::modelReset,
                   this, &FastoEditorOutput::reset));
    VERIFY(connect(model_, &QAbstractItemModel::layoutChanged,
                   this, &FastoEditorOutput::layoutChanged));
  }

  reset();
}

void FastoEditorOutput::setReadOnly(bool ro) {
  editor_->setReadOnly(ro);
}

void FastoEditorOutput::viewChanged(int viewMethod) {
  view_method_ = viewMethod;
  layoutChanged();
}

void FastoEditorOutput::modelDestroyed() {
}

void FastoEditorOutput::dataChanged(QModelIndex first, QModelIndex last) {
  layoutChanged();
}

void FastoEditorOutput::headerDataChanged() {
}

void FastoEditorOutput::rowsInserted(QModelIndex index, int r, int c) {
  layoutChanged();
}

void FastoEditorOutput::rowsAboutToBeRemoved(QModelIndex index, int r, int c) {
}

void FastoEditorOutput::rowsRemoved(QModelIndex index, int r, int c) {
}

void FastoEditorOutput::columnsAboutToBeRemoved(QModelIndex index, int r, int c) {
}

void FastoEditorOutput::columnsRemoved(QModelIndex index, int r, int c) {
}

void FastoEditorOutput::columnsInserted(QModelIndex index, int r, int c) {
}

void FastoEditorOutput::reset() {
  layoutChanged();
}

QModelIndex FastoEditorOutput::selectedItem(int column) const {
  if (!model_) {
    return QModelIndex();
  }

  return model_->index(0, column);
}

bool FastoEditorOutput::setData(const QModelIndex& index, const QVariant& value) {
  if (!model_) {
    return false;
  }

  return model_->setData(index, value);
}

int FastoEditorOutput::viewMethod() const {
  return view_method_;
}

QString FastoEditorOutput::text() const {
  return editor_->text();
}

void FastoEditorOutput::layoutChanged() {
  editor_->clear();
  if (!model_) {
    return;
  }

  QModelIndex index = model_->index(0, 0);
  if (!index.isValid()) {
    return;
  }

  FastoCommonItem* child = common::utils_qt::item<FastoCommonItem*>(index);
  if (!child) {
    return;
  }

  FastoCommonItem* root = dynamic_cast<FastoCommonItem*>(child->parent());
  if (!root) {
    return;
  }

  QString result;
  for (size_t i = 0; i < root->childrenCount(); ++i) {
    FastoCommonItem* child = dynamic_cast<FastoCommonItem*>(root->child(i));
    if (!child) {
      continue;
    }

    if (view_method_ == JSON) {
      QString json = toJson(child);
      result += common::escapedText(json);
    } else if (view_method_ == CSV) {
      QString csv = toCsv(child, delemitr_);
      result += common::escapedText(csv);
    } else if (view_method_ == RAW) {
      QString raw = toRaw(child);
      result += common::escapedText(raw);
    } else if (view_method_ == HEX) {
      result += toRaw(child);
    } else if (view_method_ == MSGPACK) {
      QString msgp = fromHexMsgPack(child);
      result += common::escapedText(msgp);
    } else if (view_method_ == GZIP) {
      QString gzip = fromGzip(child);
      result += common::escapedText(gzip);
    }
  }

  editor_->setMode(view_method_ == HEX ? FastoHexEdit::HEX_MODE : FastoHexEdit::TEXT_MODE);
  editor_->setData(result.toLocal8Bit());
}

FastoEditorShell::FastoEditorShell(bool showAutoCompl, QWidget* parent)
  : FastoEditor(parent) {
  setShowAutoCompletion(showAutoCompl);
  VERIFY(connect(this, &FastoEditorShell::customContextMenuRequested,
                 this, &FastoEditorShell::showContextMenu));
}

void FastoEditorShell::showContextMenu(const QPoint& pt) {
  QMenu *menu = createStandardContextMenu();
  menu->exec(mapToGlobal(pt));
  delete menu;
}

}  // namespace fastonosql

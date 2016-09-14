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

#include "gui/editor/fasto_hex_edit.h"

#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>

#include "common/macros.h"  // for UNUSED, NOTREACHED

namespace {
const QColor selectedColor = QColor(0x6d, 0x9e, 0xff, 0xff);
}

namespace fastonosql {
namespace gui {

FastoHexEdit::FastoHexEdit(QWidget* parent)
    : base_class(parent), mode_(TEXT_MODE), in_selection_state_(false) {
  setFocusPolicy(Qt::StrongFocus);
}

QString FastoHexEdit::text() const {
  if (mode_ == HEX_MODE) {
    return data_;
  } else if (mode_ == TEXT_MODE) {
    return toPlainText();
  } else if (mode_ == HTML_MODE) {
    return toHtml();
  } else {
    NOTREACHED();
    return QString();
  }
}

void FastoHexEdit::setMode(DisplayMode mode) {
  mode_ = mode;
  setReadOnly(mode_ == HEX_MODE);
}

void FastoHexEdit::setData(const QByteArray& arr) {
  if (mode_ == HEX_MODE) {
    verticalScrollBar()->setValue(0);
    data_ = arr;
    viewport()->update();
  } else if (mode_ == TEXT_MODE) {
    setText(arr);
  } else if (mode_ == HTML_MODE) {
    setHtml(arr);
  }
}

void FastoHexEdit::clear() {
  if (mode_ == HEX_MODE) {
    verticalScrollBar()->setValue(0);
  }

  data_.clear();
  base_class::clear();
}

int FastoHexEdit::charWidth() const {
  return fontMetrics().averageCharWidth() + 1;
}

int FastoHexEdit::charHeight() const {
  return fontMetrics().height();
}

int FastoHexEdit::asciiCharInLine(int wid) const {
  int res = wid / 4 / charWidth();
  return res;
}

QSize FastoHexEdit::fullSize() const {
  const int charW = charWidth();
  const int charH = charHeight();

  const QRect rect = stableRect(viewport()->rect());
  const int yPosStart = rect.top();
  UNUSED(yPosStart);
  const int xPosStart = rect.left();
  const int yPosEnd = rect.bottom();
  UNUSED(yPosEnd);
  const int xPosEnd = rect.right();

  const int wid = xPosEnd - xPosStart;
  const int widchars = wid - TextMarginXY * 2;
  const int xPosAscii = widchars / 4 * 3;  // line pos

  int acharInLine = asciiCharInLine(widchars);

  int width = xPosAscii + (acharInLine * charW);
  int height = data_.size() / acharInLine;
  if (data_.size() % acharInLine) {
    height++;
  }

  height *= charH;

  return QSize(width, height);
}

void FastoHexEdit::paintEvent(QPaintEvent* event) {
  if (mode_ == HEX_MODE) {
    QPainter painter(viewport());

    QSize areaSize = viewport()->size();
    QSize widgetSize = fullSize();

    const int charW = charWidth();
    const int charH = charHeight();

    int firstLineIdx = verticalScrollBar()->value();
    int lastLineIdx = firstLineIdx + areaSize.height() / charH;

    const QRect rect = stableRect(event->rect());
    const int yPosStart = rect.top();
    const int xPosStart = rect.left();
    const int yPosEnd = rect.bottom();
    const int xPosEnd = rect.right();

    const int wid = xPosEnd - xPosStart;
    const int height = yPosEnd - yPosStart;
    UNUSED(height);
    const int widchars = wid - TextMarginXY * 2;
    const int acharInLine = asciiCharInLine(widchars);
    if (acharInLine <= 0) {
      return;
    }

    const int xPosAscii = widchars / 4 * 3;  // line pos
    const int xPosAsciiStart = xPosAscii + TextMarginXY;

    int indexCount = data_.size() / acharInLine;
    if (lastLineIdx > indexCount) {
      lastLineIdx = indexCount;
      if (data_.size() % acharInLine) {
        lastLineIdx++;
      }
    }
    verticalScrollBar()->setPageStep(areaSize.height() / charH);
    verticalScrollBar()->setRange(0, (widgetSize.height() - areaSize.height()) / charH + 1);

    painter.setPen(Qt::gray);
    painter.drawLine(xPosAscii, yPosStart, xPosAscii, yPosEnd);

    painter.setPen(Qt::black);

    int size = data_.size();
    for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx;
         lineIdx += 1, yPos += charH) {
      QByteArray part = data_.begin() + (lineIdx * acharInLine);
      int part_size = size / acharInLine ? acharInLine : size % acharInLine;
      part.resize(part_size);
      size -= part_size;
      QByteArray hex = part.toHex();

      painter.setBackgroundMode(Qt::OpaqueMode);
      for (int xPos = xPosStart, i = 0; i < hex.size(); i++, xPos += 3 * charW) {
        QString val = hex.mid(i * 2, 2);
        QRect hexrect(xPos, yPos, 3 * charW, charH);
        painter.drawText(hexrect, Qt::AlignLeft, val);
        char ch = part[i];
        if ((ch < 0x20) || (ch > 0x7e)) {
          part[i] = '.';
        }
      }

      painter.setBackgroundMode(Qt::TransparentMode);
      QRect asciirect(xPosAsciiStart, yPos, acharInLine * charW, charH);
      painter.drawText(asciirect, Qt::AlignLeft, part);
    }
  } else {
    base_class::paintEvent(event);
  }
}

void FastoHexEdit::mousePressEvent(QMouseEvent* event) {
  if (mode_ == HEX_MODE) {
    if (event->button() == Qt::LeftButton) {
      in_selection_state_ = true;
    }
  }

  base_class::mousePressEvent(event);
}

void FastoHexEdit::mouseMoveEvent(QMouseEvent* event) {
  if (mode_ == HEX_MODE && in_selection_state_) {
  }

  base_class::mouseMoveEvent(event);
}

void FastoHexEdit::mouseReleaseEvent(QMouseEvent* event) {
  if (mode_ == HEX_MODE) {
    if ((event->button() == Qt::LeftButton) && in_selection_state_) {
      in_selection_state_ = false;
    }
  }

  base_class::mouseReleaseEvent(event);
}

bool FastoHexEdit::event(QEvent* event) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  if (event->type() == QEvent::ReadOnlyChange) {
    emit readOnlyChanged();
  }
#endif

  return base_class::event(event);
}

QRect FastoHexEdit::stableRect(const QRect& rect) {
  const int yPosStart = rect.top() + TextMarginXY;
  const int xPosStart = rect.left() + TextMarginXY;
  const int yPosEnd = rect.bottom() - TextMarginXY;
  const int xPosEnd = rect.right() - TextMarginXY;
  return QRect(QPoint(xPosStart, yPosStart), QPoint(xPosEnd, yPosEnd));
}

int FastoHexEdit::positionAtPoint(const QPoint& point) const {
  const int px = point.x();
  const int py = point.y();
  const int charW = charWidth();
  const int charH = charHeight();

  const QRect rect = stableRect(viewport()->rect());
  const int yPosStart = rect.top();
  const int xPosStart = rect.left();
  const int yPosEnd = rect.bottom();
  const int xPosEnd = rect.right();

  const int wid = xPosEnd - xPosStart;
  const int widchars = wid - TextMarginXY * 2;
  const int xPosAscii = widchars / 4 * 3;  // line pos

  int acharInLine = asciiCharInLine(widchars);
  if (acharInLine < 0) {
    acharInLine = 0;
  }

  if ((px >= xPosStart && px < xPosAscii) && (py >= yPosStart && py < yPosEnd)) {
    int posx = (xPosStart + px) / charW;
    int div = posx / 3;
    int mod = posx % 3;

    int pos = 0;  // symbol pos in data;
    if (mod == 0) {
      pos = div * 2;
    } else {
      pos = (div * 2) + 1;
    }

    int firstLineIdx = verticalScrollBar()->value();
    int posy = (py - yPosStart) / charH;
    pos += (firstLineIdx + posy) * acharInLine * 2;
    return pos;
  }

  return -1;
}

}  // namespace gui
}  // namespace fastonosql

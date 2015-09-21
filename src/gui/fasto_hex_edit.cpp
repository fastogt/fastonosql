#include "gui/fasto_hex_edit.h"

#include <QScrollBar>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>

#include "common/macros.h"

namespace
{
    const QColor selectedColor = QColor(0x6d, 0x9e, 0xff, 0xff);
}

namespace fastonosql
{
    FastoHexEdit::FastoHexEdit(QWidget *parent)
        : QPlainTextEdit(parent), mode_(TEXT_MODE)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

    QString FastoHexEdit::text() const
    {
        return toPlainText();
    }

    void FastoHexEdit::setMode(DisplayMode mode)
    {
        mode_ = mode;
        setReadOnly(mode_ == HEX_MODE);
        forceRepaint();
    }

    void FastoHexEdit::setText(const QString &arr)
    {
        verticalScrollBar()->setValue(0);
        cursorPos_ = 0;
        resetSelection(0);

        setPlainText(arr);
    }

    void FastoHexEdit::clear()
    {
        verticalScrollBar()->setValue(0);

        QPlainTextEdit::clear();
    }

    int FastoHexEdit::charWidth() const
    {
        return fontMetrics().width(QLatin1Char('9'));
    }

    int FastoHexEdit::charHeight() const
    {
        return fontMetrics().height();
    }

    int FastoHexEdit::asciiCharInLine() const
    {
        int wid = viewport()->width();
        int res = (wid / 4 / charWidth()) - 1;
        DCHECK(res > 0);
        return res;
    }

    void FastoHexEdit::paintEvent(QPaintEvent *event)
    {
        if(mode_ == HEX_MODE){
            QPainter painter(viewport());
            const QRect rect = event->rect();
            QSize areaSize = viewport()->size();
            const int charW = charWidth();
            const int charH = charHeight();

            int firstLineIdx = verticalScrollBar()->value();
            int lastLineIdx = firstLineIdx + areaSize.height() / charH;

            const int acharInLine = asciiCharInLine();
            const int yPosStart = charH;
            const int xPosStart = charW;
            const int yPosEnd = rect.bottom() - charH;
            const int xPosEnd = rect.right() - charW;

            int xPosAscii = xPosEnd - areaSize.width()/4;

            QByteArray data = text().toLocal8Bit();

            int indexCount = data.size() / acharInLine;
            if(lastLineIdx > indexCount) {
                lastLineIdx = indexCount;
                if(data.size() % acharInLine){
                    lastLineIdx++;
                }
            }

            verticalScrollBar()->setPageStep(areaSize.height() / charH);
            verticalScrollBar()->setRange(0, (lastLineIdx * charH - areaSize.height()) / charH + 1);

            painter.setPen(Qt::gray);
            painter.drawLine(xPosAscii, rect.top(), xPosAscii, height());

            painter.setPen(Qt::black);

            QBrush def = painter.brush();

            for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += 1, yPos += charH) {
                painter.setBackgroundMode(Qt::OpaqueMode);
                for(int xPos = xPosStart, i = 0; i < acharInLine && (lineIdx * acharInLine + i) < data.size(); i++, xPos += 3 * charW) {
                    const int pos = lineIdx * acharInLine + i;
                    const int dpos = pos * 2;
                    char c = data[pos];

                    // set setBackground for symbol
                    if(dpos >= selectBegin_ && dpos < selectEnd_) {
                        painter.setBackground(QBrush(selectedColor));
                    }
                    else{
                        painter.setBackground(def);
                    }

                    QString val = QString::number((c & 0xF0) >> 4, 16);
                    painter.drawText(xPos, yPos, val);

                    // set setBackground for symbol
                    if((dpos + 1) >= selectBegin_ && (dpos + 1) < selectEnd_) {
                        painter.setBackground(QBrush(selectedColor));
                    }
                    else{
                        painter.setBackground(def);                        
                    }

                    val = QString::number((c & 0xF), 16);
                    painter.drawText(xPos + charW, yPos, val);
                }

                painter.setBackgroundMode(Qt::TransparentMode);
                QByteArray part = data.begin() + (lineIdx * acharInLine);
                part.resize(acharInLine);
                painter.drawText(xPosAscii + 4, yPos, part);
            }
        }
        else{
            QPlainTextEdit::paintEvent(event);
        }
    }

    void FastoHexEdit::keyPressEvent(QKeyEvent *event)
    {
        if(mode_ == HEX_MODE){
            bool setVisible = false;
            QString data = text();

            if(event->matches(QKeySequence::MoveToNextChar)){
                setCursorPos(cursorPos_ + 1);
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToPreviousChar)){
                setCursorPos(cursorPos_ - 1);
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToEndOfLine)){
                setCursorPos(cursorPos_ | ((asciiCharInLine() * 2) - 1));
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToStartOfLine)){
                setCursorPos(cursorPos_ | (cursorPos_ % (asciiCharInLine() * 2)));
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToPreviousLine)){
                setCursorPos(cursorPos_ - asciiCharInLine() * 2);
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToNextLine)){
                setCursorPos(cursorPos_ + asciiCharInLine() * 2);
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToNextPage)){
                setCursorPos(cursorPos_ + (viewport()->height() / charHeight() - 1) * 2 * asciiCharInLine());
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToPreviousPage)){
                setCursorPos(cursorPos_ - (viewport()->height() / charHeight() - 1) * 2 * asciiCharInLine());
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToEndOfDocument)){
                setCursorPos(data.size() * 2);
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if(event->matches(QKeySequence::MoveToStartOfDocument)){
                setCursorPos(0);
                resetSelection(cursorPos_);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectAll)){
                resetSelection(0);
                setSelection(2 * data.size() + 1);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectNextChar)){
                int pos = cursorPos_ + 1;
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectPreviousChar)){
                int pos = cursorPos_ - 1;
                setSelection(pos);
                setCursorPos(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectEndOfLine)){
                int pos = cursorPos_ - (cursorPos_ % (2 * asciiCharInLine())) + (2 * asciiCharInLine());
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectStartOfLine)){
                int pos = cursorPos_ - (cursorPos_ % (2 * asciiCharInLine()));
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectPreviousLine)){
                int pos = cursorPos_ - (2 * asciiCharInLine());
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectNextLine)){
                int pos = cursorPos_ + (2 * asciiCharInLine());
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectNextPage)){
                int pos = cursorPos_ + (((viewport()->height() / charHeight()) - 1) * 2 * asciiCharInLine());
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectPreviousPage)){
                int pos = cursorPos_ - (((viewport()->height() / charHeight()) - 1) * 2 * asciiCharInLine());
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectEndOfDocument)){
                int pos = data.size() * 2;
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }
            else if (event->matches(QKeySequence::SelectStartOfDocument)){
                int pos = 0;
                setCursorPos(pos);
                setSelection(pos);
                setVisible = true;
            }

            if(setVisible){
                ensureVisible();
                forceRepaint();
            }
        }

        QPlainTextEdit::keyPressEvent(event);
    }

    void FastoHexEdit::mouseMoveEvent(QMouseEvent * event)
    {
        if(mode_ == HEX_MODE){
            int actPos = cursorPos(event->pos());
            setCursorPos(actPos);
            setSelection(actPos);
            forceRepaint();
        }

        QPlainTextEdit::mouseMoveEvent(event);
    }

    void FastoHexEdit::mousePressEvent(QMouseEvent * event)
    {
        if(event->button() == Qt::LeftButton && mode_ == HEX_MODE){
            int cPos = cursorPos(event->pos());
            setSelection(cPos);
            setCursorPos(cPos);
            forceRepaint();
        }

        QPlainTextEdit::mousePressEvent(event);
    }

    void FastoHexEdit::mouseReleaseEvent(QMouseEvent *event)
    {
        if(event->button() == Qt::LeftButton && mode_ == HEX_MODE){
            int cPos = cursorPos(event->pos());
            resetSelection(cPos);
        }

        QPlainTextEdit::mouseReleaseEvent(event);
    }

    void FastoHexEdit::forceRepaint()
    {
        viewport()->update();
    }

    int FastoHexEdit::cursorPos(const QPoint &position)
    {
        int charW = charWidth();
        int charH = charHeight();

        int wid = width() - asciiCharInLine();
        if ((position.x() >= 0) && (position.x() < wid)){
            int x = position.x() / charW;
            if ((x % 3) == 0){
                x = (x / 3) * 2;
            }
            else {
                x = ((x / 3) * 2) + 1;
            }

            int firstLineIdx = verticalScrollBar()->value();
            int y = (position.y() / charH) * 2 * asciiCharInLine();
            return x + y + firstLineIdx * asciiCharInLine() * 2;
        }

        return -1;
    }

    void FastoHexEdit::resetSelection(int pos)
    {
        if (pos < 0)
            pos = 0;

        selectInit_ = pos;
        selectBegin_ = pos;
        selectEnd_ = pos;
    }

    void FastoHexEdit::setSelection(int pos)
    {
        if (pos < 0)
            pos = 0;

        if (pos >= selectInit_){
            selectEnd_ = pos;
            selectBegin_ = selectInit_;
        }
        else{
            selectBegin_ = pos;
            selectEnd_ = selectInit_;
        }
    }

    void FastoHexEdit::setCursorPos(int position)
    {
        if(position < 0)
            position = 0;

        QString data = text();
        int maxPos = data.size() * 2;
        if(data.size() % asciiCharInLine())
            maxPos++;

        if(position > maxPos)
            position = maxPos;

        cursorPos_ = position;
    }

    void FastoHexEdit::ensureVisible()
    {
        QSize areaSize = viewport()->size();
        int charH = charHeight();

        int firstLineIdx = verticalScrollBar() -> value();
        int lastLineIdx = firstLineIdx + areaSize.height() / charH;

        int cursorY = cursorPos_ / (2 * asciiCharInLine());

        if(cursorY < firstLineIdx){
            verticalScrollBar() -> setValue(cursorY);
        }
        else if(cursorY >= lastLineIdx){
            verticalScrollBar() -> setValue(cursorY - areaSize.height() / charH + 1);
        }
    }
}

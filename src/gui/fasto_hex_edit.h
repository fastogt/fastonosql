#pragma once

#include <QPlainTextEdit>
#include <QByteArray>

namespace fastonosql
{
    class FastoHexEdit
            : public QPlainTextEdit
    {
            Q_OBJECT
        public:
            FastoHexEdit(QWidget *parent = 0);

            enum DisplayMode
            {
                TEXT_MODE,
                HEX_MODE
            };

            QString text() const;

        public Q_SLOTS:
            void setMode(DisplayMode mode);
            void setText(const QString &arr);
            void clear();

        protected:
            virtual void paintEvent(QPaintEvent *event);
            virtual void keyPressEvent(QKeyEvent *event);

            virtual void mouseMoveEvent(QMouseEvent *event);
            virtual void mousePressEvent(QMouseEvent *event);
            virtual void mouseReleaseEvent(QMouseEvent *event);

        private:
            void forceRepaint();

            DisplayMode mode_;

            int charWidth() const;
            int charHeight() const;

            int asciiCharInLine() const;

            size_t selectBegin_;
            size_t selectEnd_;
            size_t selectInit_;
            size_t cursorPos_;

            void resetSelection(int pos);
            void setSelection(int pos);
            void ensureVisible();
            void setCursorPos(int pos);
            int cursorPos(const QPoint &position);
    };
}

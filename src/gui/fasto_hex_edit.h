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
            typedef QPlainTextEdit base_class;
            FastoHexEdit(QWidget *parent = 0);

            enum DisplayMode
            {
                TEXT_MODE,
                HEX_MODE
            };

            enum
            {
                TextMarginXY = 4
            };

            QString text() const;

        public Q_SLOTS:
            void setMode(DisplayMode mode);
            void setData(const QByteArray &arr);
            void clear();

        protected:
            virtual void paintEvent(QPaintEvent *event);

            virtual void mousePressEvent(QMouseEvent* event);
            virtual void mouseMoveEvent(QMouseEvent* event);
            virtual void mouseReleaseEvent(QMouseEvent* event);

        private:
            static QRect stableRect(const QRect& rect);
            QSize fullSize() const;

            QByteArray data_;
            DisplayMode mode_;

            bool inSelectionState_;

            int charWidth() const;
            int charHeight() const;

            int asciiCharInLine(int wid) const;
            int positionAtPoint(const QPoint &point) const;
    };
}

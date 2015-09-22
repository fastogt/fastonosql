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

        private:
            QByteArray data_;
            DisplayMode mode_;

            int charWidth() const;
            int charHeight() const;

            int asciiCharInLine(int wid) const;
    };
}

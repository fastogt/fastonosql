#pragma once

#include <QWidget>

class QRadioButton;
class QAbstractItemModel;

namespace fastonosql
{
    class FastoEditorOutput;
    class FastoTextView
            : public QWidget
    {
        Q_OBJECT
    public:
        FastoTextView(const QString& delemitr, QWidget* parent = 0);

        void setModel(QAbstractItemModel* model);
        void setReadOnly(bool ro);

    private Q_SLOTS:
        void viewChanged(bool checked);

    protected:
        virtual void changeEvent(QEvent *);

    private:
        void retranslateUi();        

        FastoEditorOutput* editor_;
        QRadioButton* jsonRadioButton_;
        QRadioButton* csvRadioButton_;
        QRadioButton* rawRadioButton_;
        QRadioButton* hexRadioButton_;
        QRadioButton* msgPackRadioButton_;
        QRadioButton* gzipRadioButton_;
    };
}

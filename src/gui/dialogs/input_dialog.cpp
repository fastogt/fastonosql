#include "gui/dialogs/input_dialog.h"

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QGridLayout>

#include "gui/gui_factory.h"

namespace fastonosql
{
    InputDialog::InputDialog(QWidget* parent, const QString& title, InputType type,
                             const QString& firstLabelText, const QString& secondLabelText)
        : QDialog(parent)
    {
        setWindowTitle(title);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        QGridLayout* glayout = new QGridLayout;

        QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &InputDialog::accept));
        VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &InputDialog::reject));
        QLabel* firstLabel = new QLabel(firstLabelText);
        firstLine_ = new QLineEdit;
        secondLine_ = new QLineEdit;

        glayout->addWidget(firstLabel, 0, 0);
        glayout->addWidget(firstLine_, 0, 1);

        if(type == DoubleLine){
            QLabel* secondLabel = new QLabel(secondLabelText);
            glayout->addWidget(secondLabel, 1, 0);
            glayout->addWidget(secondLine_, 1, 1);
        }

        glayout->addWidget(buttonBox, 2, 1);

        setLayout(glayout);
        glayout->setSizeConstraint(QLayout::SetFixedSize);
    }

    QString InputDialog::firstText() const
    {
        return firstLine_->text();
    }

    QString InputDialog::secondText() const
    {
        return secondLine_->text();
    }
}

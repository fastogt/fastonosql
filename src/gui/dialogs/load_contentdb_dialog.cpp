#include "gui/dialogs/load_contentdb_dialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QLabel>
#include <QSpinBox>

#include "gui/gui_factory.h"
#include "translations/global.h"

namespace fastonosql
{
    LoadContentDbDialog::LoadContentDbDialog(const QString &title, connectionTypes type, QWidget* parent)
        : QDialog(parent), type_(type)
    {
        setWindowIcon(GuiFactory::instance().icon(type_));
        setWindowTitle(title);
        QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &LoadContentDbDialog::accept));
        VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &LoadContentDbDialog::reject));

        QVBoxLayout *mainLayout = new QVBoxLayout;

        QHBoxLayout* countLayout = new QHBoxLayout;
        countLayout->addWidget(new QLabel(tr("Keys count:")));
        countSpinEdit_ = new QSpinBox;
        countSpinEdit_->setRange(min_key_on_page, max_key_on_page);
        countSpinEdit_->setSingleStep(step_keys_on_page);
        countSpinEdit_->setValue(defaults_key);
        countLayout->addWidget(countSpinEdit_);
        mainLayout->addLayout(countLayout);

        QHBoxLayout* patternLayout = new QHBoxLayout;
        patternLayout->addWidget(new QLabel(tr("Pattern:")));
        patternEdit_ = new QLineEdit;
        patternEdit_->setFixedWidth(80);
        patternEdit_->setText("*");
        patternLayout->addWidget(patternEdit_);
        mainLayout->addLayout(patternLayout);

        mainLayout->addWidget(buttonBox);

        setMinimumSize(QSize(min_width, min_height));
        setLayout(mainLayout);
    }

    uint32_t LoadContentDbDialog::count() const
    {
        return countSpinEdit_->value();
    }

    QString LoadContentDbDialog::pattern() const
    {
        return patternEdit_->text();
    }

    void LoadContentDbDialog::accept()
    {
        using namespace translations;
        QString pattern = patternEdit_->text();
        if(pattern.isEmpty()){
            QMessageBox::warning(this, trError, QObject::tr("Invalid pattern!"));
            countSpinEdit_->setFocus();
            return;
        }

        QDialog::accept();
    }
}

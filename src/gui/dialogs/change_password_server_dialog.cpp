#include "gui/dialogs/change_password_server_dialog.h"

#include <QDialogButtonBox>
#include <QLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>

#include "core/iserver.h"

#include "fasto/qt/gui/glass_widget.h"
#include "gui/gui_factory.h"

#include "translations/global.h"

namespace fastonosql
{
    ChangePasswordServerDialog::ChangePasswordServerDialog(const QString &title, IServerSPtr server, QWidget* parent)
        : QDialog(parent), server_(server)
    {
        DCHECK(server_);

        setWindowTitle(title);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        QGridLayout* mainLayout = new QGridLayout;

        mainLayout->addWidget(new QLabel(tr("Password:")), 0, 0);
        passwordLineEdit_ = new QLineEdit;
        passwordLineEdit_->setEchoMode(QLineEdit::Password);
        mainLayout->addWidget(passwordLineEdit_, 0, 1);

        mainLayout->addWidget(new QLabel(tr("Confirm Password:")), 1, 0);
        confPasswordLineEdit_ = new QLineEdit;
        confPasswordLineEdit_->setEchoMode(QLineEdit::Password);
        mainLayout->addWidget(confPasswordLineEdit_, 1, 1);

        QDialogButtonBox* buttonBox = new QDialogButtonBox;
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ChangePasswordServerDialog::tryToCreatePassword));
        VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ChangePasswordServerDialog::reject));

        VERIFY(connect(server_.get(), &IServer::startedChangePassword, this, &ChangePasswordServerDialog::startChangePassword));
        VERIFY(connect(server_.get(), &IServer::finishedChangePassword, this, &ChangePasswordServerDialog::finishChangePassword));

        mainLayout->addWidget(buttonBox);
        setFixedSize(QSize(fix_width, fix_height));
        setLayout(mainLayout);

        using namespace translations;
        glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(), trTryToChangePassword, 0.5, QColor(111, 111, 100), this);
    }

    void ChangePasswordServerDialog::tryToCreatePassword()
    {
        if(validateInput()){
            server_->changePassword("", passwordLineEdit_->text());
        }
        else{
            using namespace translations;
            QMessageBox::critical(this, trError, QObject::tr("Invalid input!"));
        }
    }

    void ChangePasswordServerDialog::startChangePassword(const EventsInfo::ChangePasswordRequest& req)
    {
        glassWidget_->start();
    }

    void ChangePasswordServerDialog::finishChangePassword(const EventsInfo::ChangePasswordResponce& res)
    {
        glassWidget_->stop();
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        using namespace translations;
        QMessageBox::information(this, trInfo, QObject::tr("Password successfully changed!"));
        ChangePasswordServerDialog::accept();
    }

    bool ChangePasswordServerDialog::validateInput()
    {
        const QString pass = passwordLineEdit_->text();
        const QString cpass = confPasswordLineEdit_->text();
        if(pass.isEmpty() || cpass.isEmpty()){
            return false;
        }

        return pass == cpass;
    }
}

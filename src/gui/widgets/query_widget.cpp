#include "gui/widgets/query_widget.h"

#include <QHBoxLayout>
#include <QSplitter>

#include "shell/shell_widget.h"

#include "gui/widgets/output_widget.h"

namespace fastoredis
{
    QueryWidget::QueryWidget(IServerSPtr server, QWidget* parent)
        : QWidget(parent)
    {
        shellWidget_ = new BaseShellWidget(server);
        outputWidget_ = new OutputWidget(server);

        VERIFY(connect(shellWidget_, &BaseShellWidget::rootCreated, outputWidget_, &OutputWidget::rootCreate));
        VERIFY(connect(shellWidget_, &BaseShellWidget::rootCompleated, outputWidget_, &OutputWidget::rootCompleate));

        VERIFY(connect(shellWidget_, &BaseShellWidget::addedChild, outputWidget_, &OutputWidget::addChild));
        VERIFY(connect(shellWidget_, &BaseShellWidget::itemUpdated, outputWidget_, &OutputWidget::itemUpdate));

        QSplitter* splitter = new QSplitter;
#ifdef OS_WIN
        splitter->setStyleSheet("QSplitter::handle { background-color: gray }");
#endif
        splitter->setOrientation(Qt::Vertical);
        splitter->setHandleWidth(1);
        splitter->setContentsMargins(0, 0, 0, 0);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->setSpacing(0);
        splitter->addWidget(shellWidget_);
        splitter->addWidget(outputWidget_);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        mainLayout->addWidget(splitter);

        setLayout(mainLayout);
    }

    QueryWidget* QueryWidget::clone(const QString& text)
    {
        QueryWidget *result = new QueryWidget(shellWidget_->server(), parentWidget());
        result->shellWidget_->setText(text);
        return result;
    }

    connectionTypes QueryWidget::connectionType() const
    {
        IServerSPtr ser = shellWidget_->server();
        if(!ser){
            return DBUNKNOWN;
        }

        return ser->type();
    }

    QString QueryWidget::inputText() const
    {
        return shellWidget_->text();
    }

    void QueryWidget::setInputText(const QString& text)
    {
        shellWidget_->setText(text);
    }

    void QueryWidget::execute(const QString& text)
    {
        shellWidget_->executeText(text);
    }

    void QueryWidget::reload()
    {

    }
}

#include "gui/dialogs/view_keys_dialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QEvent>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QScrollBar>
#include <QSplitter>

#include "common/qt/convert_string.h"

#include "core/iserver.h"
#include "core/idatabase.h"

#include "gui/keys_table_model.h"
#include "gui/fasto_table_view.h"
#include "gui/gui_factory.h"

#include "translations/global.h"

namespace
{
    QPushButton *createButtonWithIcon(const QIcon &icon)
    {
        QPushButton *button = new QPushButton;
        button->setIcon(icon);
        button->setFixedSize(24, 24);
        button->setFlat(true);
        return button;
    }
}

namespace fastonosql
{
    ViewKeysDialog::ViewKeysDialog(const QString &title, IDatabaseSPtr db, QWidget* parent)
        : QDialog(parent), db_(db), cursorStack_(), curPos_(0)
    {
        DCHECK(db_);
        if(db_){
            IServerSPtr serv = db_->server();
            VERIFY(connect(serv.get(), &IServer::startedLoadDataBaseContent, this, &ViewKeysDialog::startLoadDatabaseContent));
            VERIFY(connect(serv.get(), &IServer::finishedLoadDatabaseContent, this, &ViewKeysDialog::finishLoadDatabaseContent));
        }

        setWindowTitle(title);

        // main layout
        QVBoxLayout *mainlayout = new QVBoxLayout;

        QHBoxLayout* searchLayout = new QHBoxLayout;
        searchBox_ = new QLineEdit;
        searchBox_->setText("*");
        VERIFY(connect(searchBox_, &QLineEdit::textChanged, this, &ViewKeysDialog::searchLineChanged));
        searchLayout->addWidget(searchBox_);

        countSpinEdit_ = new QSpinBox;
        countSpinEdit_->setRange(min_key_on_page, max_key_on_page);
        countSpinEdit_->setSingleStep(step_keys_on_page);
        countSpinEdit_->setValue(defaults_key);

        keyCountLabel_ = new QLabel;

        searchLayout->addWidget(keyCountLabel_);
        searchLayout->addWidget(countSpinEdit_);

        searchButton_ = new QPushButton;
        VERIFY(connect(searchButton_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
        searchLayout->addWidget(searchButton_);

        keysModel_ = new KeysTableModel(this);
        keysTable_ = new FastoTableView;
        keysTable_->setModel(keysModel_);

        QDialogButtonBox* buttonBox = new QDialogButtonBox;
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ViewKeysDialog::accept));
        VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ViewKeysDialog::reject));
        mainlayout->addLayout(searchLayout);
        mainlayout->addWidget(keysTable_);

        leftButtonList_ = createButtonWithIcon(GuiFactory::instance().leftIcon());
        rightButtonList_ = createButtonWithIcon(GuiFactory::instance().rightIcon());
        VERIFY(connect(leftButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::leftPageClicked));
        VERIFY(connect(rightButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
        QHBoxLayout* pagingLayout = new QHBoxLayout;
        pagingLayout->addWidget(leftButtonList_);
        DataBaseInfoSPtr inf = db_->info();
        size_t sizeKey = inf->size();
        currentKey_ = new QSpinBox;
        currentKey_->setEnabled(false);
        currentKey_->setValue(0);
        currentKey_->setMinimum(0);
        currentKey_->setMaximum(sizeKey);
        countKey_ = new QSpinBox;
        countKey_->setEnabled(false);
        countKey_->setValue(sizeKey);
        pagingLayout->addWidget(new QSplitter(Qt::Horizontal));
        pagingLayout->addWidget(currentKey_);
        pagingLayout->addWidget(countKey_);
        pagingLayout->addWidget(new QSplitter(Qt::Horizontal));
        pagingLayout->addWidget(rightButtonList_);

        mainlayout->addLayout(pagingLayout);
        mainlayout->addWidget(buttonBox);

        setMinimumSize(QSize(min_width, min_height));
        setLayout(mainlayout);

        updateControls();
        retranslateUi();
    }

    void ViewKeysDialog::startLoadDatabaseContent(const EventsInfo::LoadDatabaseContentRequest& req)
    {
        keysModel_->clear();
    }

    void ViewKeysDialog::finishLoadDatabaseContent(const EventsInfo::LoadDatabaseContentResponce& res)
    {
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        if(!keysModel_){
            return;
        }

        EventsInfo::LoadDatabaseContentResponce::keys_cont_type keys = res.keys_;

        size_t size = keys.size();
        for(size_t i = 0; i < size; ++i){
            NKey key = keys[i];
            keysModel_->insertItem(new KeyTableItem(key));
        }

        int curv = currentKey_->value();
        if(cursorStack_.size() == curPos_){
            cursorStack_.push_back(res.cursorOut_);
            currentKey_->setValue(curv + size);
        }
        else{
            currentKey_->setValue(curv - size);
        }

        updateControls();
    }

    void ViewKeysDialog::search(bool forward)
    {
        if(!db_){
            return;
        }

        QString pattern = searchBox_->text();
        if(pattern.isEmpty()){
            return;
        }

        if(cursorStack_.empty()){
            cursorStack_.push_back(0);
        }

        DCHECK(cursorStack_[0] == 0);
        if(forward){
            db_->loadContent(common::convertToString(pattern), countSpinEdit_->value(), cursorStack_[curPos_]);
            ++curPos_;
        }
        else{
            if(curPos_ > 0){
                db_->loadContent(common::convertToString(pattern), countSpinEdit_->value(), cursorStack_[--curPos_]);
            }
        }
    }

    void ViewKeysDialog::searchLineChanged(const QString& text)
    {
        cursorStack_.clear();
        curPos_ = 0;
        currentKey_->setValue(0);
        updateControls();
    }

    void ViewKeysDialog::leftPageClicked()
    {
        search(false);
    }

    void ViewKeysDialog::rightPageClicked()
    {
        search(true);
    }

    void ViewKeysDialog::changeEvent(QEvent* e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }
        QDialog::changeEvent(e);
    }

    void ViewKeysDialog::retranslateUi()
    {
        using namespace translations;
        keyCountLabel_->setText(trKeyCountOnThePage);
        searchButton_->setText(trSearch);
    }

    void ViewKeysDialog::updateControls()
    {
        bool isEmptyDb = keysCount() == 0;
        bool isEndSearch = curPos_ ? (cursorStack_[curPos_] == 0) : false;

        leftButtonList_->setEnabled(curPos_ > 0);
        rightButtonList_->setEnabled(!isEmptyDb && !isEndSearch);
        searchButton_->setEnabled(!isEmptyDb && !isEndSearch);
    }

    size_t ViewKeysDialog::keysCount() const
    {
        return countKey_->value();
    }
}

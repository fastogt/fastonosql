#include "gui/fasto_text_view.h"

#include <QVBoxLayout>
#include <QRadioButton>
#include <QEvent>
#include <QPushButton>
#include <QSplitter>
#include <QDebug>

#include "common/macros.h"

#include "translations/global.h"

#include "gui/fasto_editor.h"
#include "gui/gui_factory.h"

namespace fastonosql
{
    FastoTextView::FastoTextView(const QString& delemitr, QWidget* parent)
        : QWidget(parent)
    {
        QVBoxLayout *mainL = new QVBoxLayout;

        editor_ = new FastoEditorOutput(delemitr);

        jsonRadioButton_ = new QRadioButton;
        csvRadioButton_ = new QRadioButton;
        rawRadioButton_ = new QRadioButton;
        hexRadioButton_ = new QRadioButton;
        msgPackRadioButton_ = new QRadioButton;
        gzipRadioButton_ = new QRadioButton;

        saveChangeButton_ = new QPushButton;
        saveChangeButton_->setIcon(GuiFactory::instance().saveIcon());
        saveChangeButton_->setEnabled(false);

        VERIFY(connect(jsonRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChanged));
        VERIFY(connect(csvRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChanged));
        VERIFY(connect(rawRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChanged));
        VERIFY(connect(hexRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChanged));
        VERIFY(connect(msgPackRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChanged));
        VERIFY(connect(gzipRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChanged));
        VERIFY(connect(saveChangeButton_, &QPushButton::clicked, this, &FastoTextView::saveChanges));
        VERIFY(connect(editor_, &FastoEditorOutput::textChanged, this, &FastoTextView::textChange));

        QHBoxLayout* radLaout = new QHBoxLayout;
        radLaout->addWidget(jsonRadioButton_);
        radLaout->addWidget(csvRadioButton_);
        radLaout->addWidget(rawRadioButton_);
        radLaout->addWidget(hexRadioButton_);
        radLaout->addWidget(msgPackRadioButton_);
        radLaout->addWidget(gzipRadioButton_);

        mainL->addLayout(radLaout);
        mainL->addWidget(editor_);

        QHBoxLayout *hlayout = new QHBoxLayout;
        QSplitter *splitter = new QSplitter;
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(1);
        splitter->setContentsMargins(0, 0, 0, 0);
        hlayout->addWidget(splitter);
        hlayout->addWidget(saveChangeButton_);

        mainL->addLayout(hlayout);
        setLayout(mainL);

        jsonRadioButton_->setChecked(true);
        retranslateUi();
    }

    void FastoTextView::setModel(QAbstractItemModel* model)
    {
        editor_->setModel(model);
    }

    void FastoTextView::setReadOnly(bool ro)
    {
        editor_->setReadOnly(ro);
    }

    void FastoTextView::saveChanges()
    {
        QModelIndex index = editor_->selectedItem(1); //eValue
        editor_->setData(index, editor_->text().simplified());
    }

    void FastoTextView::textChange()
    {
        QModelIndex index = editor_->selectedItem(1); //eValue
        bool isEnabled = index.isValid()
                && (index.flags() & Qt::ItemIsEditable)
                && index.data() != editor_->text().simplified();

        saveChangeButton_->setEnabled(isEnabled);
    }

    void FastoTextView::viewChanged(bool checked)
    {
        if (!checked){
            return;
        }

        if(jsonRadioButton_->isChecked()){
            editor_->viewChanged(JSON);
            return;
        }

        if(csvRadioButton_->isChecked()){
            editor_->viewChanged(CSV);
            return;
        }

        if(rawRadioButton_->isChecked()){
            editor_->viewChanged(RAW);
            return;
        }

        if(hexRadioButton_->isChecked()){
            editor_->viewChanged(HEX);
            return;
        }

        if(msgPackRadioButton_->isChecked()){
            editor_->viewChanged(MSGPACK);
            return;
        }

        if(gzipRadioButton_->isChecked()){
            editor_->viewChanged(GZIP);
            return;
        }
    }

    void FastoTextView::changeEvent(QEvent* e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }

        QWidget::changeEvent(e);
    }

    void FastoTextView::retranslateUi()
    {
        using namespace translations;
        jsonRadioButton_->setText(trJson);
        csvRadioButton_->setText(trCsv);
        rawRadioButton_->setText(trRawText);
        hexRadioButton_->setText(trHex);
        msgPackRadioButton_->setText(trMsgPack);
        gzipRadioButton_->setText(trGzip);
        saveChangeButton_->setText(trSave);
    }
}

#include "gui/dialogs/encode_decode_dialog.h"

#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QSplitter>
#include <QToolButton>
#include <QComboBox>
#include <QRadioButton>
#include <QEvent>
#include <QKeyEvent>

#include "gui/gui_factory.h"
#include "gui/fasto_editor.h"

#include "common/text_decoders/iedcoder.h"
#include "common/qt/convert_string.h"

#include "translations/global.h"

namespace fastonosql
{
    EncodeDecodeDialog::EncodeDecodeDialog(QWidget* parent)
        : QDialog(parent)
    {
        setWindowIcon(GuiFactory::instance().encodeDecodeIcon());

        setWindowTitle(translations::trEncodeDecode);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        QVBoxLayout* layout = new QVBoxLayout;

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        QPushButton* closeButton = buttonBox->button(QDialogButtonBox::Close);
        buttonBox->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
        VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &EncodeDecodeDialog::reject));

        QToolButton* decode = new QToolButton;
        decode->setIcon(GuiFactory::instance().executeIcon());
        VERIFY(connect(decode, &QToolButton::clicked, this, &EncodeDecodeDialog::decode));

        decoders_ = new QComboBox;
        for(int i = 0; i < SIZEOFMASS(common::EDecoderTypes); ++i){
            decoders_->addItem(common::convertFromString<QString>(common::EDecoderTypes[i]));
        }

        QHBoxLayout* toolBarLayout = new QHBoxLayout;
        toolBarLayout->setContentsMargins(0, 0, 0, 0);
        toolBarLayout->addWidget(decode);
        toolBarLayout->addWidget(decoders_);

        encodeButton_ = new QRadioButton;
        decodeButton_ = new QRadioButton;
        toolBarLayout->addWidget(encodeButton_);
        toolBarLayout->addWidget(decodeButton_);

        QSplitter* splitter = new QSplitter;
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(1);
        splitter->setContentsMargins(0, 0, 0, 0);
        toolBarLayout->addWidget(splitter);

        input_ = new FastoEditor;
        input_->installEventFilter(this);
        output_ = new FastoEditor;
        output_->installEventFilter(this);

        layout->addWidget(input_);       
        layout->addLayout(toolBarLayout);        
        layout->addWidget(output_);
        layout->addWidget(buttonBox);

        setMinimumSize(QSize(width, height));
        setLayout(layout);

        retranslateUi();        
    }

    bool EncodeDecodeDialog::eventFilter(QObject* object, QEvent* event)
    {
        if (object == output_ || object == input_) {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = (QKeyEvent *)event;
                if (keyEvent->key() == Qt::Key_Escape) {
                    reject();
                    return true;
                }
            }
        }

        return QWidget::eventFilter(object, event);
    }

    void EncodeDecodeDialog::changeEvent(QEvent* e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }

        QWidget::changeEvent(e);
    }

    void EncodeDecodeDialog::decode()
    {
        const QString in = input_->text();
        if(in.isEmpty()){
            return;
        }

        output_->clear();
        QString decoderText = decoders_->currentText();
        std::string sdec = common::convertToString(decoderText);
        common::IEDcoder* dec = common::IEDcoder::createEDCoder(sdec);

        if(!dec){
            return;
        }

        const std::string sin = common::convertToString(in);
        std::string out;
        common::ErrorValueSPtr er;
        if(encodeButton_->isChecked()){
            er = dec->encode(sin, out);
        }
        else{
            er = dec->decode(sin, out);
        }

        if(!er){
            output_->setText(common::convertFromString<QString>(out));
        }
    }

    void EncodeDecodeDialog::retranslateUi()
    {
        using namespace translations;
        encodeButton_->setText(trEncode);
        decodeButton_->setText(trDecode);
    }
}

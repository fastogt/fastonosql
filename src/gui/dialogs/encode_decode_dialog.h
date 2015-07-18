#pragma  once

#include <QDialog>

class QComboBox;
class QRadioButton;

namespace fastoredis
{
    class FastoEditor;
    class EncodeDecodeDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        explicit EncodeDecodeDialog(QWidget* parent);

        enum
        {
            height = 480,
            width = 640
        };

    protected:
        virtual void changeEvent(QEvent* );
        virtual bool eventFilter(QObject* object, QEvent* event);

    private Q_SLOTS:
        void decode();

    private:
        void retranslateUi();

        FastoEditor* input_;
        FastoEditor* output_;
        QComboBox* decoders_;
        QRadioButton* encodeButton_;
        QRadioButton* decodeButton_;
    };
}

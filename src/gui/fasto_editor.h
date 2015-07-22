#pragma once

#include <QModelIndex>
#include <QWidget>

#define JSON 0
#define CSV 1
#define RAW 2
#define HEX 3
#define MSGPACK 4
#define GZIP 5

class QLineEdit;
class QToolButton;
class QPushButton;
class QCheckBox;
class QFrame;
class QsciLexer;
class QMenu;

namespace fasto
{
    namespace qt
    {
        namespace gui
        {
            class FastoScintilla;
        }
    }
}

namespace fastonosql
{
    class FastoEditor
        : public QWidget
    {
        Q_OBJECT
    public:
        enum
        {
            HeightFindPanel = 40
        };
        FastoEditor(QWidget* parent = 0);
        virtual ~FastoEditor();

        void registerImage(int id, const QPixmap &im);

        QString text() const;
        QString selectedText() const;

    public Q_SLOTS:
        void append(const QString &text);
        void setReadOnly(bool ro);
        void setText(const QString &text);
        void clear();

    private Q_SLOTS:
        void goToNextElement();
        void goToPrevElement();

    protected:
        void setShowAutoCompletion(bool showA);
        QMenu* createStandardContextMenu();

        void setAllCommands(const QString& allCommands);
        void setLexer(QsciLexer *lexer);

        virtual void keyPressEvent(QKeyEvent* e);
        virtual bool eventFilter(QObject* object, QEvent* event);
        virtual void changeEvent(QEvent *);

    private:
        void retranslateUi();
        void findElement(bool forward);

        fasto::qt::gui::FastoScintilla* scin_;
        QFrame* findPanel_;
        QLineEdit* findLine_;
        QToolButton* close_;
        QPushButton* next_;
        QPushButton* prev_;
        QCheckBox*  caseSensitive_;
    };

    class FastoEditorOutput
            : public FastoEditor
    {
        Q_OBJECT
    public:
        FastoEditorOutput(const QString &delemitr, QWidget *parent = 0);

        void setModel(QAbstractItemModel* model);

    public Q_SLOTS:
        void viewChanged(int viewMethod);

    private Q_SLOTS:
        void modelDestroyed();
        void dataChanged(QModelIndex first, QModelIndex last);
        void headerDataChanged();
        void rowsInserted(QModelIndex index, int r, int c);
        void rowsAboutToBeRemoved(QModelIndex index, int r, int c);
        void rowsRemoved(QModelIndex index, int r, int c);
        void columnsAboutToBeRemoved(QModelIndex index, int r, int c);
        void columnsRemoved(QModelIndex index, int r, int c);
        void columnsInserted(QModelIndex index, int r, int c);
        void reset();
        void layoutChanged();

    private:
        QAbstractItemModel* model_;
        int viewMethod_;
        const QString delemitr_;
    };

    class FastoEditorShell
            : public FastoEditor
    {
        Q_OBJECT
    public:
        QString version() const;

    protected Q_SLOTS:
        void showContextMenu(const QPoint& pt);

    protected:
        FastoEditorShell(const QString& version, bool showAutoCompl, QWidget *parent = 0);

    private:
        const QString version_;
    };
}

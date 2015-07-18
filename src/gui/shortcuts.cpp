#include "gui/shortcuts.h"

namespace fastoredis
{
    /*bool isDuplicateTabShortcut(QKeyEvent* keyEvent)
    {
        bool ctrlShiftT = (keyEvent->modifiers() & Qt::ControlModifier) &&
            (keyEvent->modifiers() & Qt::ShiftModifier) &&
            (keyEvent->key() == Qt::Key_T);

        return ctrlShiftT;
    }*/

    // global variables
    const fasto::qt::gui::FastoQKeySequence openKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::Open);
    const fasto::qt::gui::FastoQKeySequence saveKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::Save);
    const fasto::qt::gui::FastoQKeySequence saveAsKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::SaveAs);
#ifdef OS_MAC
    const fasto::qt::gui::FastoQKeySequence quitKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::Quit);
#else
    const fasto::qt::gui::FastoQKeySequence quitKey = fasto::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Q);
#endif
    const fasto::qt::gui::FastoQKeySequence closeKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::Close);
    const fasto::qt::gui::FastoQKeySequence newTabKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::AddTab);
    const fasto::qt::gui::FastoQKeySequence nextTabKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::NextChild);
    const fasto::qt::gui::FastoQKeySequence prevTabKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::PreviousChild);
    const fasto::qt::gui::FastoQKeySequence refreshKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::Refresh);
#ifdef OS_MAC
    const fasto::qt::gui::FastoQKeySequence fullScreenKey = fasto::qt::gui::FastoQKeySequence(QKeySequence::FullScreen);
#else
    const fasto::qt::gui::FastoQKeySequence fullScreenKey = fasto::qt::gui::FastoQKeySequence(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F11);
#endif
    const fasto::qt::gui::FastoQKeySequence executeKey = fasto::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Return);

    bool isOpenShortcut(QKeyEvent* keyEvent)
    {
        return openKey == keyEvent;
    }

    bool isSaveShortcut(QKeyEvent* keyEvent)
    {
        return saveKey == keyEvent;
    }

    bool isSaveAsShortcut(QKeyEvent* keyEvent)
    {
        return saveAsKey == keyEvent;
    }

    bool isQuitShortcut(QKeyEvent *keyEvent)
    {
        return quitKey == keyEvent;
    }

    bool isCloseShortcut(QKeyEvent* keyEvent)
    {
        return closeKey == keyEvent;
    }

    bool isSetFocusOnQueryLineShortcut(QKeyEvent* keyEvent)
    {
        return keyEvent->key() == Qt::Key_F6;
    }

    bool isNewTabShortcut(QKeyEvent* keyEvent)
    {
        return newTabKey == keyEvent;
    }

    bool isNextTabShortcut(QKeyEvent* keyEvent)
    {
        return nextTabKey == keyEvent;
    }

    bool isPreviousTabShortcut(QKeyEvent* keyEvent)
    {
        return prevTabKey == keyEvent;
    }

    bool isRefreshShortcut(QKeyEvent* keyEvent)
    {
        return refreshKey == keyEvent;
    }

    bool isFullScreenShortcut(QKeyEvent* keyEvent)
    {
        return fullScreenKey == keyEvent;
    }

    bool isExecuteScriptShortcut(QKeyEvent* keyEvent)
    {
        return executeKey == keyEvent;
    }

    /*bool isToggleCommentsShortcut(QKeyEvent* keyEvent)
    {
        return ((keyEvent->modifiers() & Qt::ControlModifier) && (keyEvent->key() == Qt::Key_Slash)) ||
               ((keyEvent->modifiers() & Qt::ControlModifier) && (keyEvent->modifiers() & Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_C));
    }*/
}

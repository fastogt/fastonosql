#pragma once

#include "fasto/qt/gui/shortcuts.h"

namespace fastoredis
{
    // global variables
    extern const fasto::qt::gui::FastoQKeySequence openKey;
    extern const fasto::qt::gui::FastoQKeySequence saveKey;
    extern const fasto::qt::gui::FastoQKeySequence saveAsKey;
    extern const fasto::qt::gui::FastoQKeySequence quitKey;
    extern const fasto::qt::gui::FastoQKeySequence closeKey;
    extern const fasto::qt::gui::FastoQKeySequence newTabKey;
    extern const fasto::qt::gui::FastoQKeySequence nextTabKey;
    extern const fasto::qt::gui::FastoQKeySequence prevTabKey;
    extern const fasto::qt::gui::FastoQKeySequence refreshKey;
    extern const fasto::qt::gui::FastoQKeySequence fullScreenKey;
    extern const fasto::qt::gui::FastoQKeySequence executeKey;

    bool isOpenShortcut(QKeyEvent* keyEvent);
    bool isSaveShortcut(QKeyEvent* keyEvent);
    bool isSaveAsShortcut(QKeyEvent* keyEvent);
    bool isQuitShortcut(QKeyEvent* keyEvent);
    bool isCloseShortcut(QKeyEvent* keyEvent);
    bool isNewTabShortcut(QKeyEvent* keyEvent);
    bool isNextTabShortcut(QKeyEvent* keyEvent);
    bool isPreviousTabShortcut(QKeyEvent* keyEvent);
    bool isRefreshShortcut(QKeyEvent* keyEvent);
    bool isFullScreenShortcut(QKeyEvent* keyEvent);
    bool isExecuteScriptShortcut(QKeyEvent* keyEvent);
}

#pragma once

#include <QWidget>

#include "core/events/events_info.h"

class QAction;
class QPushButton;

namespace fasto
{
    namespace qt
    {
        namespace gui
        {
            class IconLabel;
        }
    }
}

namespace fastonosql
{
    class FastoTextView;
    class FastoTreeView;
    class FastoTableView;
    class FastoCommonModel;

    class OutputWidget
            : public QWidget
    {
        Q_OBJECT
    public:
        OutputWidget(IServerSPtr server, QWidget* parent = 0);

    public Q_SLOTS:
        void rootCreate(const EventsInfo::CommandRootCreatedInfo& res);
        void rootCompleate(const EventsInfo::CommandRootCompleatedInfo& res);

        void startExecuteCommand(const EventsInfo::CommandRequest& req);
        void finishExecuteCommand(const EventsInfo::CommandResponce& res);

        void addChild(FastoObject* child);
        void itemUpdate(FastoObject* item, common::Value* newValue);

    private Q_SLOTS:
        void executeCommand(CommandKeySPtr cmd);

        void setTreeView();
        void setTableView();
        void setTextView();

    private:
        void syncWithSettings();
        void updateTimeLabel(const EventsInfo::EventInfoBase& evinfo);
        fasto::qt::gui::IconLabel* timeLabel_;
        QPushButton* treeButton_;
        QPushButton* tableButton_;
        QPushButton* textButton_;

        FastoCommonModel* commonModel_;
        FastoTreeView* treeView_;
        FastoTableView* tableView_;
        FastoTextView* textView_;
        IServerSPtr server_;
    };
}

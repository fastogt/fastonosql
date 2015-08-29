#pragma once

#include <QTreeView>

namespace fastonosql
{
    class FastoTreeView
            : public QTreeView
    {        
        Q_OBJECT
    public:
        explicit FastoTreeView(QWidget* parent = 0);

    private Q_SLOTS:
        void showContextMenu(const QPoint& point);

    protected:
        virtual void resizeEvent(QResizeEvent *event);
    };
}


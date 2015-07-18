#pragma once

#include <QTreeView>

namespace fastoredis
{
    class FastoTreeView
            : public QTreeView
    {        
        Q_OBJECT
    public:
        FastoTreeView(QWidget* parent = 0);

    private Q_SLOTS:
        void showContextMenu(const QPoint& point);
    };
}


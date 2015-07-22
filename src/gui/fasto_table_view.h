#pragma once

#include <QTableView>

namespace fastonosql
{
    class FastoTableView
            : public QTableView
    {        
        Q_OBJECT
    public:
        FastoTableView(QWidget* parent = 0);

    private Q_SLOTS:
        void showContextMenu(const QPoint& point);
    };
}


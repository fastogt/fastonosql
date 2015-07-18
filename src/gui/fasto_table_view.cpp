#include "gui/fasto_table_view.h"

#include <QMenu>
#include <QHeaderView>

#include "common/macros.h"

namespace fastoredis
{
    FastoTableView::FastoTableView(QWidget* parent)
        : QTableView(parent)
    {
        verticalHeader()->setDefaultAlignment(Qt::AlignLeft);
        horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setSelectionBehavior(QAbstractItemView::SelectItems);

        setContextMenuPolicy(Qt::CustomContextMenu);
        VERIFY(connect(this, &FastoTableView::customContextMenuRequested, this, &FastoTableView::showContextMenu));
    }

    void FastoTableView::showContextMenu(const QPoint& point)
    {
        QPoint menuPoint = mapToGlobal(point);
        menuPoint.setY(menuPoint.y() + horizontalHeader()->height());
        menuPoint.setX(menuPoint.x() + verticalHeader()->width());
        QMenu menu(this);
        menu.exec(menuPoint);
    }
}

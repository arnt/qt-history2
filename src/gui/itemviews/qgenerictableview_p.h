#ifndef QGENERICTABLEVIEW_P_H
#define QGENERICTABLEVIEW_P_H

#include <private/qabstractitemview_p.h>

class QGenericTableViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericTableView)
public:
    QGenericTableViewPrivate()
        : showGrid(true), gridStyle(Qt::SolidLine), horizontalHeader(0), verticalHeader(0) {}
    void init();

    bool showGrid;
    Qt::PenStyle gridStyle;
    QGenericHeader *horizontalHeader;
    QGenericHeader *verticalHeader;
    QModelIndex topLeft, bottomRight; // Used for optimization in setSelection
};

#endif

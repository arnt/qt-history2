#ifndef QGENERICTABLEVIEW_P_H
#define QGENERICTABLEVIEW_P_H

#include <private/qabstractitemview_p.h>

class QGenericTableViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericTableView);
public:
    QGenericTableViewPrivate()
	: showGrid(true), topHeader(0), leftHeader(0) {}

    bool showGrid;
    QGenericHeader *topHeader, *leftHeader;
    QModelIndex topLeft, bottomRight; // Used for optimization in setSelection
};

#endif

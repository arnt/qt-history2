#ifndef QGENERICTREEVIEW_P_H
#define QGENERICTREEVIEW_P_H

#include <private/qabstractitemview_p.h>

struct QGenericTreeViewItem
{
    QGenericTreeViewItem() : open(false), hidden(false), total(0), level(0) {}
    QModelIndex index;
    uint open : 1;
    uint hidden : 1;
    uint total : 30; // total number of children visible
    uint level : 16; // indentation
};

class QGenericTreeViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericTreeView);
public:

    QGenericTreeViewPrivate()
        : QAbstractItemViewPrivate(), header(0), indent(20), itemHeight(-1) { }

    ~QGenericTreeViewPrivate() {}

    bool isOpen(int item) const;
    void open(int item);
    void close(int item);

    int pageUp(int item) const;
    int pageDown(int item) const;
    int above(int item) const;
    int below(int item) const;
    int first() const;
    int last() const;

    int indentation(int item) const;
    int coordinate(int item, int value) const;
    int item(int coordinate, int value) const;

    int viewIndex(const QModelIndex &index, int value) const;
    QModelIndex modelIndex(int i) const;

    int itemAt(int value) const;
    int coordinateAt(int value, int iheight) const;

    QGenericHeader *header;
    int indent;

    //Expanded expanded;
    QVector<QGenericTreeViewItem> items;
    int itemHeight; // this is just a number; contentsHeight() / numItems

    int layout_parent_index;
    int layout_from_index;
    int layout_count;

    // used for drawing
    int left;
    int right;
    int current;
};

#endif

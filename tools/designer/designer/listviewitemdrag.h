#ifndef LISTVIEWITEMDRAG_H
#define LISTVIEWITEMDRAG_H

#include <qdragobject.h>
#include <qlistview.h>
#include <qvaluelist.h>

typedef QPtrList<QListViewItem> ListViewItemList;

class ListViewItemDrag : public QStoredDrag
{
public:
    ListViewItemDrag( ListViewItemList &items, QWidget * parent = 0, const char * name = 0 );
    ~ListViewItemDrag() {};

    static bool canDecode( QDragMoveEvent * event );
    static bool decode( QDropEvent * event, QListView *parent, QListViewItem *below );
};

#endif // LISTVIEWITEMDRAG_H

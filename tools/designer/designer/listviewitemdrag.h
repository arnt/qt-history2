#ifndef LISTVIEWITEMDRAG_H
#define LISTVIEWITEMDRAG_H

#include <qdragobject.h>
#include <qlistview.h>

class ListViewItemDrag : public QStoredDrag, public QListViewItem
{
public:
    ListViewItemDrag( QListViewItem * item, QWidget * parent = 0, const char * name = 0 );
    ~ListViewItemDrag() {};

    static bool canDecode( QDragMoveEvent * event );
    static bool decode( QDropEvent * event, QListViewItem * item );
};

#endif // LISTVIEWITEMDRAG_H

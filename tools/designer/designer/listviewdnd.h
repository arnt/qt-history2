#ifndef LISTVIEWDND_H
#define LISTVIEWDND_H

#include <qobject.h>

class QWidget;
class QListView;
class QListViewItem;
typedef QPtrList<QListViewItem> ListViewItemList;

class ListViewDnd : public QObject
{
    Q_OBJECT
public:
    ListViewDnd( QListView * eventSource, const char *name = 0 );
    
    enum DragMode { None = 0, External = 1, Internal = 2, Both = 3, Move = 4, Flat = 8, NullDrop = 16 };
    void setDragMode( int mode );
    int dragMode() const;
    bool eventFilter( QObject *, QEvent * event ); 

signals:
    void dropped( QListViewItem * );

public slots:
    void confirmDrop( QListViewItem * );

protected:
    bool dragEnterEvent( QDragEnterEvent * event );
    bool dragLeaveEvent( QDragLeaveEvent * );
    bool dragMoveEvent( QDragMoveEvent * event );
    bool dropEvent( QDropEvent * event );
    bool mousePressEvent( QMouseEvent * event );
    bool mouseMoveEvent( QMouseEvent * event );

private:
    QListViewItem *itemAt( QPoint & pos );
    int buildFlatList( ListViewItemList &list );
    int buildTreeList( ListViewItemList &list );
    QListView *src;
    QWidget *line;
    QPoint mousePressPos;
    QPoint dragPos;
    bool dragInside;
    bool dragDelete;
    bool dropConfirmed;
    int dMode;
};

#endif //LISTVIEWDND_H

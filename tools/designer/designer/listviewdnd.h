#ifndef LISTVIEWDND_H
#define LISTVIEWDND_H

#include <qobject.h>

class QWidget;
class QListView;
class QListViewItem;

class ListViewDnd : public QObject
{
    Q_OBJECT
public:
    
    ListViewDnd( QListView * eventSource, const char *name = 0 );
    
    enum DragMode { None = 0, External = 1, Internal = 2, Both = 3, Move = 4, NullDrop = 8 };
    void setDragMode( int mode );
    int dragMode() const;
    bool eventFilter( QObject *, QEvent * event ); 
signals:
    void dragged( QListViewItem * );
    void dropped( QListViewItem * );
protected:
    bool dragEnterEvent( QDragEnterEvent * event );
    bool dragLeaveEvent( QDragLeaveEvent * );
    bool dragMoveEvent( QDragMoveEvent * event );
    bool dropEvent( QDropEvent * event );
    bool mousePressEvent( QMouseEvent * event );
    bool mouseMoveEvent( QMouseEvent * event );
private:
    int moveItemTo( QListViewItem * item, QPoint & pos );
    QListView * src;
    QWidget * line;
    QPoint mousePressPos;
    QPoint dragPos;
    bool dragInside;
    bool dragDelete;
    int dMode;
};

#endif //LISTVIEWDND_H

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LISTVIEWDND_H
#define LISTVIEWDND_H

#include <qlistview.h>
#include <qlist.h>
#include "listdnd.h"

class QWidget;
class QListView;
typedef QList<QListViewItem*> ListViewItemList;

class ListViewDnd : public ListDnd
{
    Q_OBJECT
public:
    enum DragMode { Flat = 16 }; // see ListDnd::DragMode

    ListViewDnd( QListView * eventSource, const char * name = 0 );

signals:
    void dropped( QListViewItem * );

public slots:
    void confirmDrop( QListViewItem * );

protected:
    virtual bool dropEvent( QDropEvent * event );
    virtual bool mouseMoveEvent( QMouseEvent * event );
    virtual void updateLine( const QPoint & pos );
    virtual bool canDecode( QDragEnterEvent * event );
private:
    QListViewItem * itemAt( QPoint pos );
    int dropDepth( QListViewItem * item, QPoint pos );
    int buildFlatList( ListViewItemList & list );
    int buildTreeList( ListViewItemList & list );
    void setVisibleItems( bool b );
    ListViewItemList disabledItems;
};

#endif //LISTVIEWDND_H

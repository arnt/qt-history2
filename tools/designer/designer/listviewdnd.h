/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef LISTVIEWDND_H
#define LISTVIEWDND_H

#include <qobject.h>
#include <qptrlist.h>
#include <qlistview.h>

class QWidget;
class QListView;
typedef QPtrList<QListViewItem> ListViewItemList;

class ListViewDnd : public QObject
{
    Q_OBJECT
public:
    enum DragMode { None = 0, External = 1, Internal = 2, Both = 3, Move = 4, Flat = 8, NullDrop = 16 };

    ListViewDnd( QListView * eventSource, const char *name = 0 );
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
    void updateLine( const QPoint &pos );
    QListViewItem *itemAt( QPoint pos );
    int dropDepth( QListViewItem* item, QPoint pos );
    int buildFlatList( ListViewItemList &list );
    int buildTreeList( ListViewItemList &list );
    void setEnableItems( bool b );
    QListView *src;
    QWidget *line;
    QPoint mousePressPos;
    QPoint dragPos;
    bool dragInside;
    bool dragDelete;
    bool dropConfirmed;
    ListViewItemList disabledItems;
    int dMode;
};

#endif //LISTVIEWDND_H

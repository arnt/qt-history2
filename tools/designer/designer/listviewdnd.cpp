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

#include "listviewdnd.h"
#include "listviewitemdrag.h"
#include <qwidget.h>
#include <qheader.h>
#include <qpainter.h>
#include <qlistview.h>

ListViewDnd::ListViewDnd( QListView * eventSource, const char *name )
    : QObject( eventSource, name ), 
      dragInside( FALSE ), dragDelete( TRUE ), dropConfirmed( FALSE ), dMode( Both )
{
    src = eventSource;
    src->setAcceptDrops( true );
    src->installEventFilter( this );

    line = new QWidget( src->viewport(), 0, Qt::WStyle_NoBorder | WStyle_StaysOnTop );
    line->setBackgroundColor( Qt::black );
    line->resize( src->viewport()->width(), 2 );
    line->hide();
    
}

void ListViewDnd::setDragMode( int mode )
{
    dMode = mode;
}

int ListViewDnd::dragMode() const
{
    return dMode;
}

bool ListViewDnd::eventFilter( QObject *, QEvent * event )
{
    switch ( event->type() ) {
    case QEvent::DragEnter:
	return dragEnterEvent( (QDragEnterEvent *) event );
    case QEvent::DragLeave:
	return dragLeaveEvent( (QDragLeaveEvent *) event );
    case QEvent::DragMove:
	return dragMoveEvent( (QDragMoveEvent *) event );
    case QEvent::Drop:
	return dropEvent( (QDropEvent *) event );
    case QEvent::MouseButtonPress:
	return mousePressEvent( (QMouseEvent *) event );
    case QEvent::MouseMove:
	return mouseMoveEvent( (QMouseEvent *) event );
    default:
	break;
    }
    return FALSE;
}

void ListViewDnd::confirmDrop( QListViewItem * )
{
    dropConfirmed = TRUE;
}

bool ListViewDnd::dragEnterEvent( QDragEnterEvent * event )
{
    if ( dMode == None ) {
	return TRUE;
    }

    bool ok = ( ( ( dMode & Both ) == Both ) ||
		( ( dMode & Internal ) && ( event->source() == src ) ) ||
		( ( dMode & External ) && ( event->source() != src ) ) );
    
    if ( ok && ListViewItemDrag::canDecode( event ) ) {
	event->accept();
	dragInside = TRUE;
	if ( !( dMode & NullDrop ) ) {
	    dragPos = event->pos();
	    line->resize( src->viewport()->width(), line->height() );
	    QListViewItem *item = itemAt(dragPos);
	    int pos = item ? 
		( src->itemRect( item ).bottom() - ( line->height() / 2 ) ) : 
		( src->itemRect( src->firstChild() ).top() );
	    line->move( 0, pos );
	    line->show();
	}
    }

    return TRUE;
}

bool ListViewDnd::dragLeaveEvent( QDragLeaveEvent * )
{
    dragInside = FALSE;
    line->hide();
    return TRUE;
}

bool ListViewDnd::dragMoveEvent( QDragMoveEvent * event )
{
    if ( dragInside && dMode && !( dMode & NullDrop ) ) {
	QPoint dragPos = event->pos();
	QListViewItem *item = itemAt(dragPos);
	int pos = item ? 
	    ( src->itemRect( item ).bottom() - ( line->height() / 2 ) ) : 
	    ( src->itemRect( src->firstChild() ).top() );
	line->move( 0, pos );
    }
    return TRUE;
}

bool ListViewDnd::dropEvent( QDropEvent * event )
{
    if ( dragInside) {
    
	if ( dMode & NullDrop ) { // combined with Move, a NullDrop will delete an item
	    event->accept();
	    emit dropped( 0 ); // a NullDrop
	    return TRUE;
	}
	
	QPoint pos = event->pos();
	if ( ListViewItemDrag::decode( event, src, itemAt( pos ) ) ) {
	    event->accept();
	    emit dropped( 0 ); // Use ID instead of item?
	}
    }

    line->hide();
    dragInside = FALSE;

    return TRUE;
}

bool ListViewDnd::mousePressEvent( QMouseEvent * event )
{
    if ( event->button() == LeftButton )
	mousePressPos = event->pos();
    return FALSE;
}

bool ListViewDnd::mouseMoveEvent( QMouseEvent * event )
{
    if ( event->state() & LeftButton ) {
	if ( ( event->pos() - mousePressPos ).manhattanLength() > 3 ) {
	    ListViewItemList list;

	    if ( dMode & Flat )
		buildFlatList( list );
	    else
		buildTreeList( list );

	    ListViewItemDrag * dragobject = new ListViewItemDrag( list, src );

	    dragobject->dragCopy();
	    // Did the target accept the drop?
	    if ( ( dMode & Move ) && dropConfirmed ) {
		// Shouldn't autoDelete handle this?
		for( list.first(); list.current(); list.next() ) 
		    delete list.current();
		dropConfirmed = FALSE;
	    }
	}
    }
    return FALSE;
}
int ListViewDnd::buildFlatList( ListViewItemList &list )
{
    bool addKids = FALSE;
    QListViewItem *nextSibling = 0;
    QListViewItem *nextParent = 0;
    QListViewItemIterator it = src->firstChild();
    for ( ; *it; it++ ) {
	// Hit the nextSibling, turn of child processing
	if ( (*it) == nextSibling )
	    addKids = FALSE;

	if ( (*it)->isSelected() ) {
	    if ( (*it)->childCount() == 0 ) {
		// Selected, no children
		list.append( *it );
	    } else if ( !addKids ) {
		// Children processing not set, so set it
		// Also find the item were we shall quit
		// processing children...if any such item
		addKids = TRUE;
		nextSibling = (*it)->nextSibling();
		nextParent = (*it)->parent();
		while ( nextParent && !nextSibling ) {
		    nextSibling = nextParent->nextSibling();
		    nextParent = nextParent->parent();
		}
	    }
	} else if ( ((*it)->childCount() == 0) && addKids ) {
	    // Leaf node, and we _do_ process children
	    list.append( *it );
	}
    }
    return list.count();
}

int ListViewDnd::buildTreeList( ListViewItemList &list )
{
    QListViewItemIterator it = src->firstChild();
    for ( ; *it; it++ ) {
	if ( (*it)->isSelected() )
	    list.append( *it );
    }
    return list.count();
}

QListViewItem *ListViewDnd::itemAt( QPoint & pos )
{
    pos.ry() -= (int)(src->header()->height() * 1.5);
    QListViewItem *result = src->itemAt( pos );
    while ( result && result->parent() )
	result = result->parent();
    if ( !result && src->firstChild() )
	if ( pos.y() > src->itemRect( src->firstChild() ).bottom() )
	    result = src->lastItem();
    return result;
}

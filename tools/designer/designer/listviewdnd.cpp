#include "listviewdnd.h"
#include "listviewitemdrag.h"
#include <qwidget.h>
#include <qheader.h>
#include <qpainter.h>
#include <qlistview.h>

ListViewDnd::ListViewDnd( QListView * eventSource, const char *name )
    : QObject( eventSource, name )
{
    src = eventSource;
    src->setAcceptDrops( true );
    src->installEventFilter( this );

    dragInside = FALSE;
    dragDelete = TRUE;
    dropConfirmed = FALSE;
    dMode = Both;

    line = new QWidget( src->viewport(), 0, Qt::WStyle_NoBorder | WStyle_StaysOnTop );
    line->resize( src->viewport()->width(), 1 );
    
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
	    line->resize( src->viewport()->width(), 1 );
	    line->move( 0, moveItemTo( 0, dragPos ) );
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
	dragPos = event->pos();
	line->move( 0, moveItemTo( 0, dragPos ) );
    }
    return TRUE;
}

bool ListViewDnd::dropEvent( QDropEvent * event )
{
    dragInside = FALSE;
    line->hide();
    
    if ( dMode & NullDrop ) { // combined with Move, a NullDrop will delete an item
	event->accept();
	emit dropped( 0 ); // a NullDrop
	return TRUE;
    }
    
    QListViewItem * item = new QListViewItem( src );

    if ( ListViewItemDrag::decode( event, item ) ) {
	event->accept();
	emit dropped( item );
	src->insertItem( item );
	QPoint pos = event->pos();
	moveItemTo( item, pos );
	src->setCurrentItem( item );
	src->ensureItemVisible( item );
	emit added( item ); // NOTE: signal is emitted _after_ item has been added
    }
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
	    QListViewItem * item = src->selectedItem();
	    ListViewItemDrag * dragobject = new ListViewItemDrag( item, src );
	    dragobject->dragCopy();
	    emit dragged( item );
	    if ( ( dMode & Move ) && dropConfirmed ) {
		emit deleting( item ); // NOTE: signal is emitted _before_ item is deleted
		//delete item;
		src->takeItem( item ); //FIXME: memleak
		dropConfirmed = FALSE;
	    }
	}
    }
    return FALSE;
}

int ListViewDnd::moveItemTo( QListViewItem * item, QPoint & pos )
{
    QListViewItem * current = src->firstChild();
    QListViewItem * below;
    int top, mid, bot, line = 0;
    QPoint vp = src->viewportToContents( pos );
    int y = vp.y() - src->header()->height();

    while ( current ) {

	top = current->itemPos();
	mid = top + current->height() / 2;
	bot = top + current->height();

	if ( y >= top && y < mid ) { // in upper half of item
	    line = top; // line above current item
	    break; // insert above
	}

	line = bot; // line below current item
	below = current->itemBelow();

	if ( !below ) {
	    break; // always return last item if we are at the end of the list
	}

	current = below; // next item
	
	if ( y >= mid && y <= bot ) { // in lower half of item
	    break; // insert below
	}
    }

    if ( item ) // if item is 0, this function was only used to find line
	item->moveItem( current );
   
    return line + ( pos.y() - vp.y() );
}

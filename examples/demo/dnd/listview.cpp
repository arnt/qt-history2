#include <iostream.h>
#include <qdragobject.h>
#include <qapplication.h>
#include "listview.h"
#include "dnd.h"

ListView::ListView( QWidget* parent, const char* name )
    : QListView( parent, name )
{
    setAcceptDrops( TRUE );
    setSorting( -1, FALSE );
    dragging = FALSE;
}

ListView::~ListView()
{

}

void ListView::dragEnterEvent( QDragEnterEvent *e )
{
    if ( e->provides( "text/dragdemotag" ) )
	e->accept();
}

void ListView::dropEvent( QDropEvent *e )
{
    if ( !e->provides( "text/dragdemotag" ) )
         return;

    QString tag;

    if ( QTextDrag::decode( e, tag ) ) {
        IconItem item = ((DnDDemo*) parentWidget())->findItem( tag );
        QListViewItem *after = itemAt( viewport()->mapFromParent( e->pos() ) );
        ListViewItem *litem = new ListViewItem( this, after, item.name(), tag );
        litem->setPixmap( 0, *item.pixmap() );
    }
}

void ListView::mousePressEvent( QMouseEvent *e )
{
    QListView::mousePressEvent( e );
    dragging = TRUE;
    pressPos = e->pos();
}

void ListView::mouseMoveEvent( QMouseEvent *e )
{
    QListView::mouseMoveEvent( e );

    if ( ! dragging ) return;

    if ( !currentItem() ) return;

    if ( ( pressPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
        QTextDrag *drg = new QTextDrag( ((ListViewItem*)currentItem())->tag(), this );
        drg->setSubtype( "dragdemotag" );
        dragging = FALSE;
    }
}

void ListView::mouseReleaseEvent( QMouseEvent *e )
{
    QListView::mouseReleaseEvent( e );
    dragging = FALSE;
}


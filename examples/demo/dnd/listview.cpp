#include <iostream.h>
#include <qdragobject.h>
#include <qapplication.h>
#include "listview.h"

ListView::ListView( QWidget* parent, const char* name )
    : QListView( parent, name )
{
    setAcceptDrops( TRUE );
    dragging = FALSE;
}

ListView::~ListView()
{

}

void ListView::dragEnterEvent( QDragEnterEvent *e )
{
    if ( QTextDrag::canDecode( e ) )
	e->accept();
}

void ListView::dropEvent( QDropEvent *e )
{
    QString text;

    if ( QTextDrag::decode( e, text ) ) {
        QListViewItem *after = itemAt( viewport()->mapFromParent( e->pos() ) );
        if ( after )
            cout << "insert after:" << after->text( 0 ) << endl;
        QListViewItem *item = new QListViewItem( this, after, text );
        item->moveItem ( after );
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
        QDragObject *d = new QTextDrag( currentItem()->text( 0 ), this );
        d->dragCopy();
        dragging = FALSE;
    }
}

void ListView::mouseReleaseEvent( QMouseEvent *e )
{
    QListView::mouseReleaseEvent( e );
    dragging = FALSE;
}


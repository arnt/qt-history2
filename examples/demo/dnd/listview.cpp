/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qdragobject.h>
#include <qapplication.h>
#include <qevent.h>
#include "listview.h"
#include "dnd.h"

ListView::ListView( QWidget* parent, const char* name )
    : Q3ListView( parent, name )
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
        Q3ListViewItem *after = itemAt( viewport()->mapFromParent( e->pos() ) );
        ListViewItem *litem = new ListViewItem( this, after, item.name(), tag );
        litem->setPixmap( 0, *item.pixmap() );
    }
}

void ListView::contentsMousePressEvent( QMouseEvent *e )
{
    Q3ListView::contentsMousePressEvent( e );
    dragging = TRUE;
    pressPos = e->pos();
}

void ListView::contentsMouseMoveEvent( QMouseEvent *e )
{
    Q3ListView::contentsMouseMoveEvent( e );

    if ( ! dragging ) return;

    if ( !currentItem() ) return;

    if ( ( pressPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
        QTextDrag *drg = new QTextDrag( ((ListViewItem*)currentItem())->tag(), this );

        const QPixmap *p = ((ListViewItem*)currentItem())->pixmap( 0 );
        if (p)
            drg->setPixmap(*p);
        drg->setSubtype( "dragdemotag" );
        drg->dragCopy();
        dragging = FALSE;
    }
}

void ListView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    Q3ListView::contentsMouseReleaseEvent( e );
    dragging = FALSE;
}


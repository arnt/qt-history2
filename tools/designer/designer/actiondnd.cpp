#include "actiondnd.h"
#include <qaction.h>

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw )
    : QToolBar( mw )
{
    setAcceptDrops( TRUE );
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerToolBar::dragEnterEvent( QDragEnterEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
}

void QDesignerToolBar::dragMoveEvent( QDragMoveEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
}

void QDesignerToolBar::dragLeaveEvent( QDragLeaveEvent * )
{
}

void QDesignerToolBar::dropEvent( QDropEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else 
	return;
    QString s( e->encodedData( "application/x-designer-actions" ) );
    QAction *a = (QAction*)s.toLong(); // #### huha, that is evil
    a->addTo( this );
}

#endif

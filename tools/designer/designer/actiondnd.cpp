#include "actiondnd.h"
#include <qaction.h>
#include <qmainwindow.h>

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




QDesignerMenuBar::QDesignerMenuBar( QWidget *mw )
    : QMenuBar( mw, 0 )
{
    show();
    setAcceptDrops( TRUE );
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerMenuBar::dragEnterEvent( QDragEnterEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
}

void QDesignerMenuBar::dragMoveEvent( QDragMoveEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else
	return;
    int item = itemAtPos( e->pos() );
    activateItemAt( item );
    if ( item == -1 )
	hidePopups();
}

void QDesignerMenuBar::dragLeaveEvent( QDragLeaveEvent * )
{
}

void QDesignerMenuBar::dropEvent( QDropEvent * )
{
}

#endif




QDesignerPopupMenu::QDesignerPopupMenu( QWidget *w )
    : QPopupMenu( w, 0 )
{
    setAcceptDrops( TRUE );
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerPopupMenu::dragEnterEvent( QDragEnterEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
}

void QDesignerPopupMenu::dragMoveEvent( QDragMoveEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else
	return;
    // ### draw indicator stuff
}

void QDesignerPopupMenu::dragLeaveEvent( QDragLeaveEvent * )
{
}

void QDesignerPopupMenu::dropEvent( QDropEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else
	return;
    QString s( e->encodedData( "application/x-designer-actions" ) );
    QAction *a = (QAction*)s.toLong(); // #### huha, that is evil
    a->addTo( this );
    if ( indexOf( 42 ) != -1 )
	removeItem( 42 );
    ( (QDesignerMenuBar*)( (QMainWindow*)parentWidget() )->menuBar() )->hidePopups(); 
    ( (QDesignerMenuBar*)( (QMainWindow*)parentWidget() )->menuBar() )->activateItemAt( -1 ); 
}

#endif

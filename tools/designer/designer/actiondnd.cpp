#include "actiondnd.h"
#include <qaction.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qobjectlist.h>

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw )
    : QToolBar( mw ), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerToolBar::dragEnterEvent( QDragEnterEvent *e )
{
    lastIndicatorPos = QPoint( -1, -1 );
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
}

void QDesignerToolBar::dragMoveEvent( QDragMoveEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else 
	return;
    drawIndicator( calcIndicatorPos( e->pos() ) );
}

void QDesignerToolBar::dragLeaveEvent( QDragLeaveEvent * )
{
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	drawIndicator( QPoint( -1, -1 ) );
    insertAnchor = 0;
    afterAnchor = TRUE;
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
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	drawIndicator( QPoint( -1, -1 ) );
    insertAnchor = 0;
    afterAnchor = TRUE;
}

#endif

QPoint QDesignerToolBar::calcIndicatorPos( const QPoint &pos )
{
    if ( orientation() == Horizontal ) {
	QPoint pnt( width() - 2, 0 );
	insertAnchor = 0;
	afterAnchor = TRUE;
	if ( !children() )
	    return pnt;
	pnt = QPoint( 13, 0 );
	afterAnchor = FALSE;
	QObjectListIt it( *children() );
	QObject * obj;
	while( (obj=it.current()) != 0 ) {
	    ++it;
	    if ( obj->isWidgetType() &&
		 qstrcmp( "qt_dockwidget_internal", obj->name() ) != 0 ) {
		QWidget *w = (QWidget*)obj;
		if ( w->x() < pos.x() ) {
		    pnt.setX( w->x() + w->width() + 1 );
		    insertAnchor = w;
		    afterAnchor = TRUE;
		}
	    }
	}
	return pnt;
    } else {
    }
}

void QDesignerToolBar::drawIndicator( const QPoint &pos )
{
    if ( lastIndicatorPos == pos )
	return;
    if ( orientation() == Horizontal ) {
	setWFlags( WPaintUnclipped );
	QPainter p( this );
	clearWFlags( WPaintUnclipped );
	p.setPen( QPen( gray, 2 ) );
	p.setRasterOp( XorROP );
	if ( lastIndicatorPos != QPoint( -1, -1 ) )
	    p.drawLine( lastIndicatorPos.x(), 1, lastIndicatorPos.x(), height() - 1 );
	lastIndicatorPos = pos;
	if ( lastIndicatorPos != QPoint( -1, -1 ) )
	    p.drawLine( lastIndicatorPos.x(), 1, lastIndicatorPos.x(), height() - 1 );
    } else {
    }
}





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

#include "actiondnd.h"
#include <qaction.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qobjectlist.h>
#include <qapplication.h>
#include <qlayout.h>

static bool doReinsert = TRUE;

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw )
    : QToolBar( mw ), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
    insertingAction = 0;
}

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw, Dock dock )
    : QToolBar( QString::null, mw, dock), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
    insertingAction = 0;
}

void QDesignerToolBar::addAction( QAction *a )
{
    doReinsert = FALSE;
    insertingAction = a;
    actionList.append( a );
    QApplication::sendPostedEvents( this, QEvent::ChildInserted );
    doReinsert = TRUE;
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
    insertingAction = a;
    a->addTo( this );
    connect( a, SIGNAL( destroyed() ), this, SLOT( actionRemoved() ) );
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	drawIndicator( QPoint( -1, -1 ) );
}

#endif

void QDesignerToolBar::reInsert()
{
    doReinsert = FALSE;
    QAction *a = 0;
    actionMap.clear();
    clear();
    QApplication::sendPostedEvents();
    for ( a = actionList.first(); a; a = actionList.next() ) {
	insertingAction = a;
	a->addTo( this );
	QApplication::sendPostedEvents( this, QEvent::ChildInserted );
    }
    boxLayout()->invalidate();
    boxLayout()->activate();
    doReinsert = TRUE;
}

void QDesignerToolBar::actionRemoved()
{
    actionList.removeRef( (QAction*)sender() );
}

void QDesignerToolBar::childEvent( QChildEvent *e )
{
    if ( e->type() != QEvent::ChildInserted || !insertingAction || !e->child()->isWidgetType() )
	return;
    actionMap.insert( (QWidget*)e->child(), insertingAction );

    if ( doReinsert ) {
	int index = actionList.findRef( *actionMap.find( insertAnchor ) );
	if ( index != -1 && afterAnchor )
	    ++index;
	if ( !insertAnchor )
	    index = 0;
	if ( index == -1 )
	    actionList.append( insertingAction );
	else
	    actionList.insert( index, insertingAction );
	reInsert();
    }

    insertingAction = 0;
    insertAnchor = 0;
    afterAnchor = TRUE;
}

QPoint QDesignerToolBar::calcIndicatorPos( const QPoint &pos )
{
    if ( orientation() == Horizontal ) {
	QPoint pnt( width() - 2, 0 );
	insertAnchor = 0;
	afterAnchor = TRUE;
	if ( !children() )
	    return pnt;
	pnt = QPoint( 13, 0 );
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
	QPoint pnt( 0, height() - 2 );
	insertAnchor = 0;
	afterAnchor = TRUE;
	if ( !children() )
	    return pnt;
	pnt = QPoint( 0, 13 );
	QObjectListIt it( *children() );
	QObject * obj;
	while( (obj=it.current()) != 0 ) {
	    ++it;
	    if ( obj->isWidgetType() &&
		 qstrcmp( "qt_dockwidget_internal", obj->name() ) != 0 ) {
		QWidget *w = (QWidget*)obj;
		if ( w->y() < pos.y() ) {
		    pnt.setY( w->y() + w->height() + 1 );
		    insertAnchor = w;
		    afterAnchor = TRUE;
		}
	    }
	}
	return pnt;
    }

    return QPoint( -1, -1 );
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
	setWFlags( WPaintUnclipped );
	QPainter p( this );
	clearWFlags( WPaintUnclipped );
	p.setPen( QPen( gray, 2 ) );
	p.setRasterOp( XorROP );
	if ( lastIndicatorPos != QPoint( -1, -1 ) )
	    p.drawLine( 1, lastIndicatorPos.y(), width() - 1, lastIndicatorPos.y() );
	lastIndicatorPos = pos;
	if ( lastIndicatorPos != QPoint( -1, -1 ) )
	    p.drawLine( 1, lastIndicatorPos.y(), width() - 1, lastIndicatorPos.y() );
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

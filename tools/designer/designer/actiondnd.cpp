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
}

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw, Dock dock )
    : QToolBar( QString::null, mw, dock), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
}

void QDesignerToolBar::addAction( QDesignerAction *a )
{
    doReinsert = FALSE;
    actionList.append( a );
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
    QDesignerAction *a = (QDesignerAction*)s.toLong(); // #### huha, that is evil
    a->addTo( this );
    actionMap.insert( a->widget(), a );
    int index = actionList.findRef( *actionMap.find( insertAnchor ) );
    if ( index != -1 && afterAnchor )
	++index;
    if ( !insertAnchor )
	index = 0;
    if ( index == -1 )
	actionList.append( a );
    else
	actionList.insert( index, a );
    reInsert();
    connect( a, SIGNAL( destroyed() ), this, SLOT( actionRemoved() ) );
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	drawIndicator( QPoint( -1, -1 ) );
}

#endif

void QDesignerToolBar::reInsert()
{
    doReinsert = FALSE;
    QDesignerAction *a = 0;
    actionMap.clear();
    clear();
    for ( a = actionList.first(); a; a = actionList.next() ) {
	a->addTo( this );
	actionMap.insert( a->widget(), a );
    }
    boxLayout()->invalidate();
    boxLayout()->activate();
    doReinsert = TRUE;
}

void QDesignerToolBar::actionRemoved()
{
    actionList.removeRef( (QDesignerAction*)sender() );
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
    insertAt = -1;
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerPopupMenu::dragEnterEvent( QDragEnterEvent *e )
{
    lastIndicatorPos = QPoint( -1, -1 );
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
}

void QDesignerPopupMenu::dragMoveEvent( QDragMoveEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else
	return;
    drawIndicator( calcIndicatorPos( e->pos() ) );
}

void QDesignerPopupMenu::dragLeaveEvent( QDragLeaveEvent * )
{
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	drawIndicator( QPoint( -1, -1 ) );
    insertAt = -1;
}

void QDesignerPopupMenu::dropEvent( QDropEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else
	return;
    QString s( e->encodedData( "application/x-designer-actions" ) );
    QDesignerAction *a = (QDesignerAction*)s.toLong(); // #### huha, that is evil
    a->addTo( this );
    actionList.insert( insertAt, a );
    ( (QDesignerMenuBar*)( (QMainWindow*)parentWidget() )->menuBar() )->hidePopups();
    ( (QDesignerMenuBar*)( (QMainWindow*)parentWidget() )->menuBar() )->activateItemAt( -1 );
    reInsert();
}

#endif

void QDesignerPopupMenu::reInsert()
{
    clear();
    for ( QAction *a = actionList.first(); a; a = actionList.next() )
	a->addTo( this );
}

void QDesignerPopupMenu::drawIndicator( const QPoint &pos )
{
    if ( lastIndicatorPos == pos )
	return;
    setWFlags( WPaintUnclipped );
    QPainter p( this );
    clearWFlags( WPaintUnclipped );
    p.setPen( QPen( gray, 2 ) );
    p.setRasterOp( XorROP );
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	p.drawLine( 0, lastIndicatorPos.y(), width(), lastIndicatorPos.y() );
    lastIndicatorPos = pos;
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	p.drawLine( 0, lastIndicatorPos.y(), width(), lastIndicatorPos.y() );
}

QPoint QDesignerPopupMenu::calcIndicatorPos( const QPoint &pos )
{
    int h = frameWidth();
    insertAt = count();
    for ( int i = 0; i < (int)count(); ++i ) {
	QRect r = itemGeometry( i );
	if ( pos.y() < h + r.height() / 2 ) {
	    insertAt = i;
	    break;
	}
	h += r.height();
    }

    return QPoint( 0, h );
}

void QDesignerPopupMenu::addAction( QDesignerAction *a )
{
    actionList.append( a );
}

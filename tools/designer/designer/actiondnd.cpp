#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>
#include "actiondnd.h"
#include <qaction.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qobjectlist.h>
#include <qapplication.h>
#include <qlayout.h>
#include "metadatabase.h"
#include <qdragobject.h>
#include <qbitmap.h>

static bool doReinsert = TRUE;

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw )
    : QToolBar( mw ), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
    MetaDataBase::addEntry( this );
}

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw, Dock dock )
    : QToolBar( QString::null, mw, dock), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
    MetaDataBase::addEntry( this );
}

void QDesignerToolBar::addAction( QAction *a )
{
    doReinsert = FALSE;
    actionList.append( a );
    doReinsert = TRUE;
    connect( a, SIGNAL( destroyed() ), this, SLOT( actionRemoved() ) );
    // ####
    ( (QDesignerAction*)a )->widget()->installEventFilter( this );
}

bool QDesignerToolBar::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e || o == this )
	return QToolBar::eventFilter( o, e );

    if ( e->type() == QEvent::MouseButtonPress ) {
	QMouseEvent *ke = (QMouseEvent*)e;
	buttonMousePressEvent( ke, o );
	return TRUE;
    } else if ( e->type() == QEvent::MouseMove ) {
	QMouseEvent *ke = (QMouseEvent*)e;
	buttonMouseMoveEvent( ke, o );
	return TRUE;
    }
	
    return QToolBar::eventFilter( o, e );
}

void QDesignerToolBar::buttonMousePressEvent( QMouseEvent *e, QObject *o )
{
    if ( e->button() == MidButton )
	return;

    if ( e->button() == RightButton ) {
	QPopupMenu menu( this );
	menu.insertItem( tr( "Delete Item" ) );
	if ( menu.exec( e->globalPos() ) != -1 ) {
	    QAction *a = *actionMap.find( (QWidget*)o );
	    if ( !a )
		return;
	    actionList.remove( a );
	    a->removeFrom( this );
	}
	return;
    }

    dragStartPos = e->pos();
}

void QDesignerToolBar::buttonMouseMoveEvent( QMouseEvent *e, QObject *o )
{
    if ( QABS( QPoint( dragStartPos - e->pos() ).manhattanLength() ) < QApplication::startDragDistance() )
	return;
    QAction *a = *actionMap.find( (QWidget*)o );
    if ( !a )
	return;
    a->removeFrom( this );
    int idx = actionList.find( a );
    actionList.remove( a );

    QStoredDrag *drag = new QStoredDrag( "application/x-designer-actions", this );
    QString s = QString::number( (long)a ); // #### huha, that is evil
    drag->setEncodedData( QCString( s.latin1() ) );
    drag->setPixmap( a->iconSet().pixmap() );
    if ( !drag->drag() ) {
	actionList.insert( idx, a );
	reInsert();
    }
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerToolBar::dragEnterEvent( QDragEnterEvent *e )
{
    lastIndicatorPos = QPoint( -1, -1 );
    if ( e->provides( "application/x-designer-actions" ) || 
	 e->provides( "application/x-designer-actiongroup" ) )
	e->accept();
}

void QDesignerToolBar::dragMoveEvent( QDragMoveEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) ||
	 e->provides( "application/x-designer-actiongroup" ) )
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
    if ( e->provides( "application/x-designer-actions" ) ||
	 e->provides( "application/x-designer-actiongroup" ) )
	e->accept();
    else
	return;
    QString s;
    if ( e->provides( "application/x-designer-actiongroup" ) )
	s = QString( e->encodedData( "application/x-designer-actiongroup" ) );
    else
	s = QString( e->encodedData( "application/x-designer-actions" ) );

    if ( e->provides( "application/x-designer-actions" ) ) {
	QDesignerAction *a = (QDesignerAction*)s.toLong(); // #### huha, that is evil
	a->addTo( this );
	actionMap.insert( a->widget(), a );
	a->widget()->installEventFilter( this );
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
    } else {
	QDesignerActionGroup *a = (QDesignerActionGroup*)s.toLong(); // #### huha, that is evil
	a->addTo( this );
	// #######
	actionMap.insert( a->widget(), a );
	if ( a->widget() )
	    a->widget()->installEventFilter( this );
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
}

#endif

void QDesignerToolBar::reInsert()
{
    doReinsert = FALSE;
    QAction *a = 0;
    actionMap.clear();
    clear();
    for ( a = actionList.first(); a; a = actionList.next() ) {
	a->addTo( this );
	if ( a->inherits( "QActionGroup" ) ) {
	    actionMap.insert( ( (QDesignerActionGroup*)a )->widget(), a );
	    if ( ( (QDesignerActionGroup*)a )->widget() )
		( (QDesignerActionGroup*)a )->widget()->installEventFilter( this );
	} else {
	    actionMap.insert( ( (QDesignerAction*)a )->widget(), a );
	    ( (QDesignerAction*)a )->widget()->installEventFilter( this );
	}
    }
    boxLayout()->invalidate();
    boxLayout()->activate();
    doReinsert = TRUE;
}

void QDesignerToolBar::actionRemoved()
{
    actionList.removeRef( (QAction*)sender() );
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
    MetaDataBase::addEntry( this );
    itemNum = 0;
    mousePressed = FALSE;
    lastIndicatorPos = QPoint( -1, -1 );
    insertAt = -1;
}

void QDesignerMenuBar::mousePressEvent( QMouseEvent *e )
{
    lastIndicatorPos = QPoint( -1, -1 );
    insertAt = -1;
    mousePressed = TRUE;
    if ( e->button() == MidButton )
	return;

    if ( e->button() == RightButton ) {
	int itm = itemAtPos( e->pos() );
	if ( itm == -1 )
	    return;
	QPopupMenu menu( this );
	menu.insertItem( tr( "Delete Item" ) );
	if ( menu.exec( e->globalPos() ) != -1 ) {
	    removeItemAt( itm );
	    // #### need to do a proper invalidate and re-layout
	    parentWidget()->layout()->invalidate();
	    parentWidget()->layout()->activate();
	}
	return;
    }

    dragStartPos = e->pos();
    QMenuBar::mousePressEvent( e );
}

void QDesignerMenuBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed ) {
	QMenuBar::mouseMoveEvent( e );
	return;
    }
    if ( QABS( QPoint( dragStartPos - e->pos() ).manhattanLength() ) < QApplication::startDragDistance() ) {
	QMenuBar::mouseMoveEvent( e );
	return;
    }
    hidePopups();
    activateItemAt( -1 );
    int itm = itemAtPos( dragStartPos );
    if ( itm == -1 )
	return;
    QPopupMenu *popup = findItem( idAt( itm ) )->popup();
    QString txt = findItem( idAt( itm ) )->text();
    removeItemAt( itm );

    QStoredDrag *drag = new QStoredDrag( "application/x-designer-menuitem", this );
    QString s = QString::number( (long)popup );
    s += "/" + txt;
    drag->setEncodedData( QCString( s.latin1() ) );
    QSize sz( fontMetrics().boundingRect( txt ).size() );
    QPixmap pix( sz.width() + 20, sz.height() * 2 );
    pix.fill( white );
    QPainter p( &pix, this );
    p.drawText( 2, 0, pix.width(), pix.height(), 0, txt );
    p.end();
    pix.setMask( pix.createHeuristicMask() );
    drag->setPixmap( pix );
    QApplication::sendPostedEvents();
    if ( !drag->drag() ) {
	insertItem( txt, popup, -1, itm );
    }
    mousePressed = FALSE;
}

void QDesignerMenuBar::mouseReleaseEvent( QMouseEvent *e )
{
    QMenuBar::mouseReleaseEvent( e );
    mousePressed = FALSE;
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerMenuBar::dragEnterEvent( QDragEnterEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    if ( e->provides( "application/x-designer-menuitem" ) )
	e->accept();
    lastIndicatorPos = QPoint( -1, -1 );
    insertAt = -1;
}

void QDesignerMenuBar::dragMoveEvent( QDragMoveEvent *e )
{
    if ( e->provides( "application/x-designer-actions" ) ||
	 e->provides( "application/x-designer-menuitem" ) )
	e->accept();
    else
	return;
    if ( e->provides( "application/x-designer-actions" ) ) {
	int item = itemAtPos( e->pos() );
	activateItemAt( item );
	if ( item == -1 )
	    hidePopups();
    } else {
	drawIndicator( calcIndicatorPos( e->pos() ) );
    }
}

void QDesignerMenuBar::dragLeaveEvent( QDragLeaveEvent * )
{
    mousePressed = FALSE;
    lastIndicatorPos = QPoint( -1, -1 );
    insertAt = -1;
}

void QDesignerMenuBar::dropEvent( QDropEvent *e )
{
    mousePressed = FALSE;
    if ( !e->provides( "application/x-designer-menuitem" ) )
	return;
    e->accept();
    QString s( e->encodedData( "application/x-designer-menuitem" ) );
    QString s1 = s.left( s.find( "/" ) );
    QString s2 = s.mid( s.find( "/" ) + 1 );
    QPopupMenu *popup = (QPopupMenu*)s1.toLong();  // #### huha, that is evil
    QString txt = s2;
    insertItem( txt, popup, -1, insertAt );
}

#endif

QPoint QDesignerMenuBar::calcIndicatorPos( const QPoint &pos )
{
    int w = frameWidth();
    insertAt = count();
    for ( int i = 0; i < (int)count(); ++i ) {
	QRect r = itemRect( i );
	if ( pos.x() < w + r.width() / 2 ) {
	    insertAt = i;
	    break;
	}
	w += r.width();
    }

    return QPoint( w, 0 );
}

void QDesignerMenuBar::drawIndicator( const QPoint &pos )
{
    if ( lastIndicatorPos == pos )
	return;
    setWFlags( WPaintUnclipped );
    QPainter p( this );
    clearWFlags( WPaintUnclipped );
    p.setPen( QPen( gray, 2 ) );
    p.setRasterOp( XorROP );
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	p.drawLine( lastIndicatorPos.x(), 0, lastIndicatorPos.x(), height() );
    lastIndicatorPos = pos;
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	p.drawLine( lastIndicatorPos.x(), 0, lastIndicatorPos.x(), height() );
}

void QDesignerMenuBar::setItemNumber( int num )
{
    itemNum = num;
}

int QDesignerMenuBar::itemNumber() const
{
    return itemNum;
}

void QDesignerMenuBar::setItemText( const QString &s )
{
    if ( itemNum < 0 || itemNum >= (int)count() )
	return;
    changeItem( idAt( itemNum ), s );
}

QString QDesignerMenuBar::itemText() const
{
    if ( itemNum < 0 || (int)itemNum >= (int)count() )
	return QString::null;
    return text( idAt( itemNum ) );
}



QDesignerPopupMenu::QDesignerPopupMenu( QWidget *w )
    : QPopupMenu( w, 0 )
{
    setAcceptDrops( TRUE );
    insertAt = -1;
    mousePressed = FALSE;
}

void QDesignerPopupMenu::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    if ( e->button() == MidButton )
	return;

    if ( e->button() == RightButton ) {
	int itm = itemAtPos( e->pos() );
	if ( itm == -1 )
	    return;
	QPopupMenu menu( this );
	menu.insertItem( tr( "Delete Item" ) );
	if ( menu.exec( e->globalPos() ) != -1 ) {
	    removeItemAt( itm );
	    actionList.remove( itm );
	}
	return;
    }

    dragStartPos = e->pos();
    QPopupMenu::mousePressEvent( e );
}

void QDesignerPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed ) {
	QPopupMenu::mouseMoveEvent( e );
	return;
    }
    if ( QABS( QPoint( dragStartPos - e->pos() ).manhattanLength() ) < QApplication::startDragDistance() ) {
	QPopupMenu::mouseMoveEvent( e );
	return;
    }
    int itm = itemAtPos( dragStartPos );
    if ( itm == -1 )
	return;
    QAction *a = actionList.at( itm );
    if ( !a )
	return;
    a->removeFrom( this );
    actionList.remove( itm );
    
    QStoredDrag *drag = new QStoredDrag( "application/x-designer-actions", this );
    QString s = QString::number( (long)a ); // #### huha, that is evil
    drag->setEncodedData( QCString( s.latin1() ) );
    drag->setPixmap( a->iconSet().pixmap() );
    if ( !drag->drag() ) {
	actionList.insert( itm, a );
	reInsert();
    }
    mousePressed = FALSE;
}

void QDesignerPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    mousePressed = FALSE;
    QPopupMenu::mouseReleaseEvent( e );
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerPopupMenu::dragEnterEvent( QDragEnterEvent *e )
{
    mousePressed = FALSE;
    lastIndicatorPos = QPoint( -1, -1 );
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
}

void QDesignerPopupMenu::dragMoveEvent( QDragMoveEvent *e )
{
    mousePressed = FALSE;
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else
	return;
    drawIndicator( calcIndicatorPos( e->pos() ) );
}

void QDesignerPopupMenu::dragLeaveEvent( QDragLeaveEvent * )
{
    mousePressed = FALSE;
    if ( lastIndicatorPos != QPoint( -1, -1 ) )
	drawIndicator( QPoint( -1, -1 ) );
    insertAt = -1;
}

void QDesignerPopupMenu::dropEvent( QDropEvent *e )
{
    mousePressed = FALSE;
    if ( e->provides( "application/x-designer-actions" ) )
	e->accept();
    else
	return;
    QString s( e->encodedData( "application/x-designer-actions" ) );
    // #####
    QDesignerAction *a = (QDesignerAction*)s.toLong(); // #### huha, that is evil
    a->addTo( this );
    actionList.insert( insertAt, a );
    ( (QDesignerMenuBar*)( (QMainWindow*)parentWidget() )->menuBar() )->hidePopups();
    ( (QDesignerMenuBar*)( (QMainWindow*)parentWidget() )->menuBar() )->activateItemAt( -1 );
    reInsert();
    connect( a, SIGNAL( destroyed() ), this, SLOT( actionRemoved() ) );
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

void QDesignerPopupMenu::addAction( QAction *a )
{
    actionList.append( a );
    connect( a, SIGNAL( destroyed() ), this, SLOT( actionRemoved() ) );
}

void QDesignerPopupMenu::actionRemoved()
{
    actionList.removeRef( (QAction*)sender() );
}

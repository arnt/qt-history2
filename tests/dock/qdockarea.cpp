#include "qdockarea.h"
#include "qdockwidget.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qvector.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qwidgetlist.h>

class QDockAreaHandle : public QWidget
{
public:
    QDockAreaHandle( Qt::Orientation o, QDockArea *parent, const QWidgetList &wl, const char* name=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    QSize sizeHint() const;

    void append( QWidget *w ) { widgetList.append( w ); }
    
protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    void startLineDraw();
    void endLineDraw();
    void drawLine( const QPoint &globalPos );

private:
    Qt::Orientation orient;
    QDockArea *s;
    QWidgetList widgetList;
    bool mousePressed;
    QPainter *unclippedPainter;
    QPoint lastPos, firstPos;

};

QDockAreaHandle::QDockAreaHandle( Qt::Orientation o, QDockArea *parent, const QWidgetList &wl, const char * name )
    : QWidget( parent, name ), widgetList( wl ), mousePressed( FALSE ), unclippedPainter( 0 )
{
    s = parent;
    setOrientation( o );
}

QSize QDockAreaHandle::sizeHint() const
{
    int sw = style().splitterWidth();
    return QSize(sw,sw).expandedTo( QApplication::globalStrut() );
}

void QDockAreaHandle::setOrientation( Qt::Orientation o )
{
    orient = o;
    if ( o == QDockArea::Horizontal ) {
	setCursor( splitVCursor );
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    } else {
	setCursor( splitHCursor );
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
    }
}

void QDockAreaHandle::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    startLineDraw();
    lastPos = firstPos = e->globalPos();
    drawLine( e->globalPos() );
}

void QDockAreaHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    drawLine( lastPos );
    lastPos = e->globalPos();
    drawLine( e->globalPos() );
}

void QDockAreaHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( mousePressed ) {
	drawLine( lastPos );
	endLineDraw();
	if ( orientation() == Horizontal ) {
	    int dy = e->globalPos().y() - firstPos.y();
	    if ( s->orientation() == orientation() ) {
		for ( QWidget *w = widgetList.first(); w; w = widgetList.next() ) {
		    ( (QDockWidget*)w )->unsetSizeHint();
		    ( (QDockWidget*)w )->setSizeHint( QSize( w->width(), w->height() + dy ) );
		}
	    } else {
		int dy = e->globalPos().y() - firstPos.y();
		QDockWidget *dw = (QDockWidget*)widgetList.first();
		dw->unsetSizeHint();
		dw->setSizeHint( QSize( dw->width(), dw->height() + dy ) );
		dw = (QDockWidget*)widgetList.next();
		if ( dw )
		    dw->setSizeHint( QSize( dw->width(), dw->height() - dy ) );
	    }
	} else {
	    int dx = e->globalPos().x() - firstPos.x();
	    if ( s->orientation() == orientation() ) {
		for ( QWidget *w = widgetList.first(); w; w = widgetList.next() ) {
		    ( (QDockWidget*)w )->unsetSizeHint();
		    ( (QDockWidget*)w )->setSizeHint( QSize( w->width() + dx, w->height() ) );
		}
	    } else {
		int dx = e->globalPos().x() - firstPos.x();
		QDockWidget *dw = (QDockWidget*)widgetList.first();
		dw->unsetSizeHint();
		dw->setSizeHint( QSize( dw->width() + dx, dw->height() ) );
		dw = (QDockWidget*)widgetList.next();
		if ( dw )
		    dw->setSizeHint( QSize( dw->width() - dx, dw->height() ) );
	    }
	}
    }

    s->QWidget::layout()->invalidate();
    s->QWidget::layout()->activate();

    mousePressed = FALSE;
}

void QDockAreaHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    style().drawSplitter( &p, 0, 0, width(), height(), colorGroup(), orientation() == Horizontal ? Vertical : Horizontal );
}

void QDockAreaHandle::startLineDraw()
{
    if ( unclippedPainter )
	endLineDraw();
    bool unclipped = QApplication::desktop()->testWFlags( WPaintUnclipped );
    ( (QDockAreaHandle*)QApplication::desktop() )->setWFlags( WPaintUnclipped );
    unclippedPainter = new QPainter;
    unclippedPainter->begin( QApplication::desktop() );
    if ( !unclipped )
	( (QDockAreaHandle*)QApplication::desktop() )->clearWFlags( WPaintUnclipped );
    unclippedPainter->setPen( QPen( gray, orientation() == Horizontal ? height() : width() ) );
    unclippedPainter->setRasterOp( XorROP );
}

void QDockAreaHandle::endLineDraw()
{
    if ( !unclippedPainter )
	return;
    delete unclippedPainter;
    unclippedPainter = 0;
}

void QDockAreaHandle::drawLine( const QPoint &globalPos )
{
    QPoint start = mapToGlobal( QPoint( 0, 0 ) );
    if ( orientation() == Horizontal )
	unclippedPainter->drawLine( start.x(), globalPos.y(), start.x() + width(), globalPos.y() );
    else
	unclippedPainter->drawLine( globalPos.x(), start.y(), globalPos.x(), start.y() + height() );
}




QDockArea::QDockArea( Orientation o, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o ), layout( 0 ), sections( 0 )
{
    insertedSplitters.setAutoDelete( TRUE );
    dockWidgets.setAutoDelete( TRUE );
    setMinimumSize( 3, 3 );
}

void QDockArea::moveDockWidget( QDockWidget *w, const QPoint &, const QRect &, bool swap )
{
    QDockWidgetData *dockData = 0;
    int i = findDockWidget( w );
    if ( i != -1 ) {
	dockData = dockWidgets.at( i );
    } else {
	dockData = new QDockWidgetData;
	dockData->section = 0;
	dockData->dockWidget = w;
	dockData->dockWidget->reparent( this, QPoint( 0, 0 ), TRUE );
	if ( swap )
	    dockData->dockWidget->resize( dockData->dockWidget->height(), dockData->dockWidget->width() );
	dockWidgets.append( dockData );
    }
    sections = 1;
    setupLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Fixed,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Fixed ) );
}

void QDockArea::removeDockWidget( QDockWidget *w, bool makeFloating, bool swap )
{
    QDockWidgetData *dockData = 0;
    int i = findDockWidget( w );
    if ( i == -1 )
	return;
    dockData = dockWidgets.at( i );
    if ( makeFloating )
	dockData->dockWidget->reparent( 0, WStyle_Customize | WStyle_NoBorderEx, QPoint( 0, 0 ), FALSE );
    if ( swap )
	dockData->dockWidget->resize( dockData->dockWidget->height(), dockData->dockWidget->width() );
    dockWidgets.remove( i );
    sections = 1;
    setupLayout();
    if ( dockWidgets.isEmpty() )
	setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
}

int QDockArea::findDockWidget( QDockWidget *w )
{
    int i = 0;
    for ( QDockWidgetData *d = dockWidgets.first(); d; d = dockWidgets.next() ) {
	if ( d->dockWidget == w )
	    return i;
	++i;
    }
    return -1;
}

void QDockArea::setupLayout()
{
    delete layout;
    layout = 0;
    insertedSplitters.clear();

    if ( !sections )
	return;

    if ( orientation() == Horizontal )
	layout = new QBoxLayout( this, QBoxLayout::TopToBottom );
    else
	layout = new QBoxLayout( this, QBoxLayout::LeftToRight );

    QVector<QBoxLayout> layouts;
    layouts.resize( sections );
    QVector<QBoxLayout> splitters;
    splitters.resize( sections );
    QArray<bool> resizeable( sections );
    QVector<QWidgetList> wlv;
    wlv.setAutoDelete( TRUE );
    wlv.resize( sections );
    int i = 0;
    for ( i = 0; i < sections; ++i ) {
	if ( orientation() == Horizontal ) {
	    layouts.insert( i, new QHBoxLayout( layout ) );
	    splitters.insert( i, new QHBoxLayout( layout ) );
	} else {
	    layouts.insert( i, new QVBoxLayout( layout ) );
	    splitters.insert( i, new QVBoxLayout( layout ) );
	}
	resizeable[ i ] = FALSE;
	wlv.insert( i, new QWidgetList );
    }

    QDockAreaHandle *lastHandle = 0;
    
    for ( QDockWidgetData *d = dockWidgets.first(); d; d = dockWidgets.next() ) {
	resizeable[ d->section ] = d->dockWidget->isResizeEnabled();
	if ( d->dockWidget->isResizeEnabled() ) {
	    if ( lastHandle )
		lastHandle->append( d->dockWidget );
	    layouts[ d->section ]->addWidget( d->dockWidget );
	    QWidgetList wl;
	    wl.append( d->dockWidget );
	    lastHandle = new QDockAreaHandle( orientation() == Horizontal ? Vertical : Horizontal, this, wl );
	    QWidget *w = lastHandle;
	    w->show();
	    insertedSplitters.append( w );
	    layouts[ d->section ]->addWidget( w );
	    wlv[ d->section ]->append( d->dockWidget );
	} else {
	    lastHandle = 0;
	    layouts[ d->section ]->addWidget( d->dockWidget, 0, AlignLeft | AlignTop );
	}
    }

    for ( i = 0; i < sections; ++i ) {
	if ( resizeable[ i ] ) {
	    QWidget *w = new QDockAreaHandle( orientation(), this, *wlv[ i ] );
	    splitters[ i ]->addWidget( w );
	    w->show();
	    insertedSplitters.append( w );
	}
    }

    layout->activate();
}

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

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    Qt::Orientation orient;
    QDockArea *s;
    QWidgetList widgetList;

};

QDockAreaHandle::QDockAreaHandle( Qt::Orientation o, QDockArea *parent, const QWidgetList &wl, const char * name )
    : QWidget( parent, name ), widgetList( wl )
{
    s = parent;
    setOrientation(o);
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
}

void QDockAreaHandle::mouseMoveEvent( QMouseEvent *e )
{
}

void QDockAreaHandle::mouseReleaseEvent( QMouseEvent *e )
{
}

void QDockAreaHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    style().drawSplitter( &p, 0, 0, width(), height(), colorGroup(), orientation() );
}






QDockArea::QDockArea( Orientation o, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o ), layout( 0 ), sections( 0 )
{
    insertedSplitters.setAutoDelete( TRUE );
    dockWidgets.setAutoDelete( TRUE );
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
    if ( orientation() == Horizontal )
	setupHorizontalLayout();
    else
	setupVerticalLayout();
}

void QDockArea::setupHorizontalLayout()
{
    delete layout;
    layout = 0;
    insertedSplitters.clear();

    if ( !sections )
	return;

    layout = new QBoxLayout( this, QBoxLayout::TopToBottom );

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
	layouts.insert( i, new QHBoxLayout( layout ) );
	splitters.insert( i, new QHBoxLayout( layout ) );
	resizeable[ i ] = FALSE;
	wlv.insert( i, new QWidgetList );
    }

    for ( QDockWidgetData *d = dockWidgets.first(); d; d = dockWidgets.next() ) {
	layouts[ d->section ]->addWidget( d->dockWidget );
	resizeable[ d->section ] = d->dockWidget->isResizeEnabled();
	if ( d->dockWidget->isResizeEnabled() ) {
	    QWidgetList wl;
	    wl.append( d->dockWidget );
	    QWidget *w = new QDockAreaHandle( Vertical, this, wl );
	    w->show();
	    insertedSplitters.append( w );
	    layouts[ d->section ]->addWidget( w );
	    wlv[ d->section ]->append( d->dockWidget );
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

void QDockArea::setupVerticalLayout()
{
    delete layout;
    layout = 0;
    insertedSplitters.clear();

    if ( !sections )
	return;

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
	layouts.insert( i, new QVBoxLayout( layout ) );
	splitters.insert( i, new QVBoxLayout( layout ) );
	resizeable[ i ] = FALSE;
	wlv.insert( i, new QWidgetList );
    }

    for ( QDockWidgetData *d = dockWidgets.first(); d; d = dockWidgets.next() ) {
	layouts[ d->section ]->addWidget( d->dockWidget );
	resizeable[ d->section ] = d->dockWidget->isResizeEnabled();
	if ( d->dockWidget->isResizeEnabled() ) {
	    QWidgetList wl;
	    wl.append( d->dockWidget );
	    QWidget *w = new QDockAreaHandle( Horizontal, this, wl );
	    w->show();
	    insertedSplitters.append( w );
	    layouts[ d->section ]->addWidget( w );
	    wlv[ d->section ]->append( d->dockWidget );
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

#include "qdockarea.h"
#include "qdockwidget.h"

QDockArea::QDockArea( Orientation o, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o )
{
    dockWidgets.setAutoDelete( TRUE );
}

void QDockArea::moveDockWidget( QDockWidget *w /*some other args will come here*/ )
{
    QDockWidgetData *dockData = 0;
    int i = findDockWidget( w );
    if ( i != -1 ) {
	dockData = dockWidgets.at( i );
    } else {
	dockData = new QDockWidgetData;
	dockData->dockWidget = w;
	dockData->dockWidget->reparent( this, QPoint( 0, 0 ), TRUE );
	dockWidgets.append( dockData );
    }
}

void QDockArea::removeDockWidget( QDockWidget *w, bool makeFloating )
{
    QDockWidgetData *dockData = 0;
    int i = findDockWidget( w );
    if ( i == -1 )
	return;
    dockData = dockWidgets.at( i );
    if ( makeFloating )
	dockData->dockWidget->reparent( 0, WStyle_Customize | WStyle_NoBorderEx, QPoint( 0, 0 ), FALSE );
    dockWidgets.remove( i );
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

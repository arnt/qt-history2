#include "qdockarea.h"
#include "qdockwidget.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qvector.h>

QDockArea::QDockArea( Orientation o, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o ), layout( 0 ), sections( 0 )
{
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
    if ( !sections )
	return;
    
    if ( orientation() == Horizontal )
	setupHorizontalLayout();
    else
	setupVerticalLayout();
}

void QDockArea::setupHorizontalLayout()
{
    delete layout;
    layout = new QBoxLayout( this, QBoxLayout::TopToBottom );

    QVector<QBoxLayout> layouts;
    layouts.resize( sections );
    for ( int i = 0; i < sections; ++i )
	layouts.insert( i, new QVBoxLayout( layout ) );
    
    for ( QDockWidgetData *d = dockWidgets.first(); d; d = dockWidgets.next() )
	layouts[ d->section ]->addWidget( d->dockWidget );
}

void QDockArea::setupVerticalLayout()
{
    delete layout;
    layout = new QBoxLayout( this, QBoxLayout::TopToBottom );

    QVector<QBoxLayout> layouts;
    layouts.resize( sections );
    for ( int i = 0; i < sections; ++i )
	layouts.insert( i, new QHBoxLayout( layout ) );
    
    for ( QDockWidgetData *d = dockWidgets.first(); d; d = dockWidgets.next() )
	layouts[ d->section ]->addWidget( d->dockWidget );
}

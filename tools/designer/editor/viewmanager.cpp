 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "viewmanager.h"
#include "editor.h"
#include "markerwidget.h"
#include <qlayout.h>
#include <qrichtext_p.h>
#include <qdockarea.h>
#include "paragdata.h"
#include <qobjectlist.h>

ViewManager::ViewManager( QWidget *parent, const char *name )
    : QWidget( parent, name ), curView( 0 )
{
    layout = new QHBoxLayout( this );
    markerWidget = new MarkerWidget( this );
    connect( markerWidget, SIGNAL( markersChanged() ),
	     this, SIGNAL( markersChanged() ) );
    markerWidget->setFixedWidth( 20 );
    dockArea = new QDockArea( Qt::Vertical, QDockArea::Normal, this );
    layout->addWidget( dockArea );
    dockArea->setMinimumWidth( 5 );
    layout->addWidget( markerWidget );

    QObjectList *l = topLevelWidget()->queryList( "QToolBar" );
    for ( QObject *o = l->first(); o; o = l->next() )
	dockArea->setAcceptDockWindow( ( (QDockWindow*)o ), FALSE );
    delete l;
}

void ViewManager::addView( QWidget *view )
{
    layout->addWidget( view );
    curView = view;
    connect( ( (Editor*)curView )->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
	     markerWidget, SLOT( doRepaint() ) );
    connect( (Editor*)curView, SIGNAL( textChanged() ),
	     markerWidget, SLOT( doRepaint() ) );
    connect( (Editor*)curView, SIGNAL( clearErrorMarker() ),
	     this, SLOT( clearErrorMarker() ) );
}

QWidget *ViewManager::currentView() const
{
    return curView;
}

void ViewManager::childEvent( QChildEvent *e )
{
    if ( !e->child()->isWidgetType() )
	return;
    if ( e->type() == QEvent::ChildInserted ) {
	if ( e->child()->inherits( "Editor" ) )
	    addView( (QWidget*)e->child() );
    }
    QWidget::childEvent( e );
}

void ViewManager::setError( int line )
{
    QTextParag *p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
	paragData->marker = ParagData::Error;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
	( (Editor*)curView )->setErrorSelection( line );
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->viewport()->setFocus();
    }
}

void ViewManager::setStep( int line )
{
    QTextParag *p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
	// #### we should set an arrow as Setp here, but have a stack of markers, so that breakpoints do not get lost
	// 	paragData->marker = ParagData::Step;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
	( (Editor*)curView )->setStepSelection( line );
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->viewport()->setFocus();
    }
}

void ViewManager::clearStep()
{
    ( (Editor*)curView )->clearStepSelection();
}

void ViewManager::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    markerWidget->doRepaint();
}

void ViewManager::clearErrorMarker()
{
    QTextParag *p = ( (Editor*)curView )->document()->firstParag();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->marker = ParagData::NoMarker;
	p = p->next();
    }
    markerWidget->doRepaint();
}

void ViewManager::setBreakPoints( const QValueList<int> &l )
{
    QTextParag *p = ( (Editor*)curView )->document()->firstParag();
    int i = 0;
    while ( p ) {
	if ( l.find( i ) != l.end() ) {
	    if ( !p->extraData() ) {
		ParagData *data = new ParagData;
		p->setExtraData( data );
	    }
	    ParagData *data = (ParagData*)p->extraData();
	    data->marker = ParagData::Breakpoint;
	} else if ( p->extraData() ) {
	    ParagData *data = (ParagData*)p->extraData();
	    data->marker = ParagData::NoMarker;
	}
	p = p->next();
	++i;
    }
    markerWidget->doRepaint();
}

QValueList<int> ViewManager::breakPoints() const
{
    QValueList<int> l;
    int i = 0;
    QTextParag *p = ( (Editor*)curView )->document()->firstParag();
    while ( p ) {
	if ( p->extraData() &&
	     ( (ParagData*)p->extraData() )->marker == ParagData::Breakpoint )
	    l << i;
	p = p->next();
	++i;
    }
    return l;
}

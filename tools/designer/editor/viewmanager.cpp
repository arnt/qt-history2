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
#include <qlabel.h>
#include <qtimer.h>

ViewManager::ViewManager( QWidget *parent, const char *name )
    : QWidget( parent, name ), curView( 0 )
{
    QHBoxLayout *l = new QHBoxLayout( this );
    markerWidget = new MarkerWidget( this, "editor_markerwidget" );
    connect( markerWidget, SIGNAL( markersChanged() ),
	     this, SIGNAL( markersChanged() ) );
    connect( markerWidget, SIGNAL( collapseFunction( QTextParag * ) ),
	     this, SIGNAL( collapseFunction( QTextParag * ) ) );
    connect( markerWidget, SIGNAL( expandFunction( QTextParag * ) ),
	     this, SIGNAL( expandFunction( QTextParag * ) ) );
    connect( markerWidget, SIGNAL( collapse( bool ) ),
	     this, SIGNAL( collapse( bool ) ) );
    connect( markerWidget, SIGNAL( expand( bool ) ),
	     this, SIGNAL( expand( bool ) ) );
    connect( markerWidget, SIGNAL( editBreakPoints() ),
	     this, SIGNAL( editBreakPoints() ) );
    connect( markerWidget, SIGNAL( isBreakpointPossible( bool&, const QString &, int ) ),
	     this, SIGNAL( isBreakpointPossible( bool&, const QString &, int ) ) );
    connect( markerWidget, SIGNAL( showMessage( const QString & ) ),
	     this, SLOT( showMessage( const QString & ) ) );
    messageTimer = new QTimer( this );
    connect( messageTimer, SIGNAL( timeout() ), this, SLOT( clearStatusBar() ) );
    markerWidget->setFixedWidth( 35 );
    //dockArea = new QDockArea( Qt::Vertical, QDockArea::Normal, this );
    //layout->addWidget( dockArea );
    //dockArea->setMinimumWidth( 5 );
    l->addWidget( markerWidget );
    layout = new QVBoxLayout( l );

//     QObjectList *l = topLevelWidget()->queryList( "QToolBar" );
//     for ( QObject *o = l->first(); o; o = l->next() )
// 	dockArea->setAcceptDockWindow( ( (QDockWindow*)o ), FALSE );
//     delete l;
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
    posLabel = new QLabel( this, "editor_poslabel" );
    posLabel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    posLabel->setText( " Line: 1 Col: 1" );
    posLabel->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    posLabel->setLineWidth( 1 );
    posLabel->setFixedHeight( posLabel->fontMetrics().height() );
    layout->addWidget( posLabel );
    connect( curView, SIGNAL( cursorPositionChanged( int, int ) ),
	     this, SLOT( cursorPositionChanged( int, int ) ) );
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
	( (Editor*)curView )->setErrorSelection( line );
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->viewport()->setFocus();
	( (Editor*)curView )->makeFunctionVisible( p );
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
	paragData->marker = ParagData::Error;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::setStep( int line )
{
    QTextParag *p = ( (Editor*)curView )->document()->firstParag();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->step = FALSE;
	p = p->next();
    }
    p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	( (Editor*)curView )->setStepSelection( line );
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->viewport()->setFocus();
	( (Editor*)curView )->makeFunctionVisible( p );
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
 	paragData->step = TRUE;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::clearStep()
{
    ( (Editor*)curView )->clearStepSelection();
    QTextParag *p = ( (Editor*)curView )->document()->firstParag();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->step = FALSE;
	p = p->next();
    }
    markerWidget->doRepaint();
}

void ViewManager::setStackFrame( int line )
{
    QTextParag *p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	( (Editor*)curView )->sync();
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->ensureCursorVisible();
	( (Editor*)curView )->viewport()->setFocus();
	( (Editor*)curView )->makeFunctionVisible( p );
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
 	paragData->stackFrame = TRUE;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::clearStackFrame()
{
    QTextParag *p = ( (Editor*)curView )->document()->firstParag();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->stackFrame = FALSE;
	p = p->next();
    }
    markerWidget->doRepaint();
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

void ViewManager::showMarkerWidget( bool b )
{
    if ( b )
	markerWidget->show();
    else
	markerWidget->hide();
}

void ViewManager::emitMarkersChanged()
{
    emit markersChanged();
}

void ViewManager::cursorPositionChanged( int row, int col )
{
    posLabel->setText( QString( " Line: %1 Col: %1" ).arg( row + 1 ).arg( col + 1 ) );
}

void ViewManager::showMessage( const QString &msg )
{
    int row;
    int col;
    ( (QTextEdit*)currentView() )->getCursorPosition( &row, &col );
    posLabel->setText( msg );
    messageTimer->start( 1000, TRUE );
}

void ViewManager::clearStatusBar()
{
    int row;
    int col;
    ( (QTextEdit*)currentView() )->getCursorPosition( &row, &col );
    posLabel->setText( QString( " Line: %1 Col: %1" ).arg( row + 1 ).arg( col + 1 ) );
}

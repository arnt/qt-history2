#include "viewmanager.h"
#include "editor.h"
#include "iconloader.h"
#include "markerwidget.h"
#include <qlayout.h>
#include <qrichtext_p.h>
#include "paragdata.h"

ViewManager::ViewManager( QWidget *parent, const char *name )
    : QWidget( parent, name ), curView( 0 )
{
    layout = new QHBoxLayout( this );
    markerWidget = new MarkerWidget( this );
    markerWidget->setFixedWidth( 20 );
    layout->addWidget( markerWidget );
}

void ViewManager::addView( QWidget *view )
{
    layout->addWidget( view );
    curView = view;
    connect( (Editor*)curView, SIGNAL( contentsMoving( int, int ) ),
	     markerWidget, SLOT( doRepaint() ) );
    connect( (Editor*)curView, SIGNAL( textChanged() ),
	     markerWidget, SLOT( doRepaint() ) );
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
	p->setExtraData( (char*)paragData );
	markerWidget->doRepaint();
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->viewport()->setFocus();
    }
}

void ViewManager::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    markerWidget->doRepaint();
}

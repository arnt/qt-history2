#include "lhelp.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>

LHelp::LHelp( const QString& home_, const QString& path, QWidget* parent = 0, const char *name=0 )
    : QMainWindow( parent, name )
{

    browser = new QMLBrowser( this );
    browser->provider()->setPath( path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( contentsChanged() ),
	     this, SLOT( contentsChanged() ) );

    setCentralWidget( browser );

    browser->setDocument( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
	     statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( tr("&Close"), this, SLOT( close() ) );

    QPopupMenu* navigate = new QPopupMenu( this );
    backwardId = navigate->insertItem( tr("&Backward"), browser, SLOT( backward() ) );
    forwardId = navigate->insertItem( tr("&Forward"), browser, SLOT( forward() ) );
    navigate->insertItem( tr("&Home"), browser, SLOT( home() ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Navigate"), navigate );

    menuBar()->setItemEnabled( forwardId, FALSE);
    menuBar()->setItemEnabled( backwardId, FALSE);
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( setForwardAvailable( bool ) ) );


    QToolBar* toolbar = new QToolBar( this );
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( QPixmap("back.xpm"), tr("Backward"), "", browser, SLOT(backward()), toolbar );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( QPixmap("forward.xpm"), tr("Forward"), "", browser, SLOT(forward()), toolbar );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( QPixmap("home.xpm"), tr("Home"), "", browser, SLOT(home()), toolbar );

    browser->setFocus();
}


void LHelp::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void LHelp::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void LHelp::contentsChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( tr("QBrowser") );
    else
	setCaption( browser->documentTitle() ) ;
}

LHelp::~LHelp()
{
}

#include <qassistantclient.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>

#include "helpdemo.h"

HelpDemo::HelpDemo( QWidget *parent, const char *name )
    : HelpDemoBase( parent, name )
{
    checkOnlyExampleDoc->hide();
    leFileName->setText( "./doc/index.html" );
    assistant = new QAssistantClient( qInstallPathBins(), this );
    widgets.insert( (QWidget*)openQAButton, "./doc/manual.html#openqabutton" );
    widgets.insert( (QWidget*)closeQAButton, "./doc/manual.html#closeqabutton" );
    widgets.insert( (QWidget*)checkOnlyExampleDoc, "./doc/manual.html#onlydoc" );
    widgets.insert( (QWidget*)checkHide, "./doc/manual.html#hide" );
    widgets.insert( (QWidget*)leFileName, "./doc/manual.html#lineedit" );
    widgets.insert( (QWidget*)displayButton, "./doc/manual.html#displaybutton" );
    widgets.insert( (QWidget*)closeButton, "./doc/manual.html#closebutton" );

    menu = new QPopupMenu( this );

    QAction *helpAction = new QAction( "Show Help", QKeySequence( tr("F1" ) ), this );
    helpAction->addTo( menu );

    connect( assistant, SIGNAL( error( const QString& ) ),
	this, SLOT( showAssistantErrors( const QString& ) ) );
}

HelpDemo::~HelpDemo()
{
}

void HelpDemo::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_F1 )
	showHelp( lookForWidget() );
}

void HelpDemo::contextMenuEvent( QContextMenuEvent *e )
{
    QWidget *w = lookForWidget();
    if ( menu->exec( e->globalPos() ) != -1 )
	showHelp( w );
}

QWidget* HelpDemo::lookForWidget()
{
    QPtrDictIterator<char> it( widgets );
    QWidget *w;
    while ( (w = (QWidget*)(it.currentKey())) != 0 ) {
	++it;
	if ( w->hasMouse() )
	    return w;
    }
    return 0;
}

void HelpDemo::showHelp( QWidget *w )
{
    if ( w )
	assistant->showPage( QString( widgets[w] ) );
    else
	assistant->showPage( "./doc/index.html" );
}

void HelpDemo::setAssistantArguments()
{
    QStringList cmdLst;
    if ( checkHide->isChecked() )
	cmdLst << "-hideSidebar";
    if ( checkOnlyExampleDoc->isChecked() )
        cmdLst << "-profile" << "HelpExample";
    assistant->setArguments( cmdLst );
}

void HelpDemo::openAssistant()
{
    if ( !assistant->isOpen() )
	assistant->openAssistant();
    else
	QMessageBox::information( this, "Help", "Qt Assistant is already open!" );
}

void HelpDemo::closeAssistant()
{
    if ( assistant->isOpen() )
	assistant->closeAssistant();
    else
	QMessageBox::information( this, "Help", "Qt Assistant is not open!" );
}

void HelpDemo::displayPage()
{
    assistant->showPage( leFileName->text() );
}

void HelpDemo::showAssistantErrors( const QString &err )
{
    QString errorMsg = err;
    if ( err.startsWith( "Profile" ) ) {
	errorMsg += "\n\n";
	errorMsg += "First you have to install this profile. Quit\n";
	errorMsg += "the application, and start Qt Assistant with:\n\n";
	errorMsg += "assistant -addprofile helpdemo.adp /Help/Demo/Path\n\n";
	errorMsg += "and restart this example. Have a look at the\n";
	errorMsg += "example documentation for further information.\n";
    }
    QMessageBox::critical( this, "Error", errorMsg );
}

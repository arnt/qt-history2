#include <qassistantclient.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qprocess.h>
#include <qpushbutton.h>

#include "helpdemo.h"

HelpDemo::HelpDemo( QWidget *parent, const char *name )
    : HelpDemoBase( parent, name )
{
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

    connect( assistant, SIGNAL( assistantOpened() ),
	     this, SLOT( assistantOpened() ) );
    connect( assistant, SIGNAL( assistantClosed() ),
	     this, SLOT( assistantClosed() ));
    closeQAButton->setEnabled( FALSE );
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
}

void HelpDemo::closeAssistant()
{
    if ( assistant->isOpen() )
	assistant->closeAssistant();
}

class ProcessListener : public QObject {
    Q_OBJECT
public:
    ProcessListener( QProcess *pr, QWidget *w ) : p( pr ), widget( w ) { }
    ~ProcessListener() {
	delete p;
    }

public slots:
    void processExited() {
	deleteLater();
	QMessageBox::information( widget,
				  "HelpDemo documentation installed",
				  "The HelpDemo documentation was successfully installed" );
    }

private:
    QProcess *p;
    QWidget *widget;
};


void HelpDemo::installExampleDocs()
{

    QStringList lst;
    lst << "assistant"
	<< "-addProfile"
	<< "helpdemo.adp"
	<< ".";

    QProcess *proc = new QProcess( lst );

    ProcessListener *l = new ProcessListener( proc, this );

    QObject::connect( proc, SIGNAL( processExited() ), l, SLOT( processExited() ) );

    if( !proc->start() ) {
	delete l;
	showAssistantErrors( "Failed to install documentation profile\n" );
    }
}

void HelpDemo::displayPage()
{
    assistant->showPage( leFileName->text() );
}

void HelpDemo::showAssistantErrors( const QString &err )
{
    if ( err.startsWith( "Profile" ) ) {
	QString errorMsg =
	    "The documentation for the HelpDemo example has not yet\n"
	    "been installed. Press the 'Install Example Documentation'\n"
	    "button to install the documentation and retry.\n\n"
	    "If you want to see how the installation of the documentation\n"
	    "works, check out the HelpDemo::installExampleDocs()\n"
	    "function in the file helpdemo.cpp";
	QMessageBox::information( this, "Documentation not installed", errorMsg );
	return;
    }
    QMessageBox::critical( this, "Assistant Error", err );

}

void HelpDemo::assistantOpened()
{
    closeQAButton->setEnabled( TRUE );
    openQAButton->setEnabled( FALSE );
}

void HelpDemo::assistantClosed()
{
    closeQAButton->setEnabled( FALSE );
    openQAButton->setEnabled( TRUE );
}

#include "helpdemo.moc"

#include "outputwindow.h"
#include "designerappiface.h"

#include <qlistview.h>
#include <qtextedit.h>
#include <qapplication.h>
#include <qheader.h>
#include <stdlib.h>
#include <stdio.h>

QTextEdit *OutputWindow::debugView = 0;
QListView *OutputWindow::errorView = 0;

OutputWindow::OutputWindow( QWidget *parent )
    : QTabWidget( parent, "output_window" )
{
    setupDebug();
    setupError();
}

OutputWindow::~OutputWindow()
{
    debugView = 0;
    errorView = 0;
    qInstallMsgHandler( oldMsgHandler );
}

void OutputWindow::setupError()
{
    errorView = new QListView( this, "OutputWindow::errorView" );
    addTab( errorView, tr( "Error Messages" ) );
    errorView->addColumn( tr( "Message" ) );
    errorView->addColumn( tr( "Line" ) );
    errorView->header()->setFullSize( TRUE );
    errorView->setAllColumnsShowFocus( TRUE );
}

void debugMessageOutput( QtMsgType type, const char *msg )
{
    QString s;
    s = msg;
    if ( type != QtFatalMsg ) {
	if ( OutputWindow::debugView )
	    OutputWindow::debugView->append( s );
    } else {
	fprintf( stderr, msg );
	abort();
    }
    qApp->flushX();
}

void OutputWindow::setupDebug()
{
    debugView = new QTextEdit( this, "OutputWindow::debugView" );
    addTab( debugView, "Debug Output" );
    oldMsgHandler = qInstallMsgHandler( debugMessageOutput );
}

void OutputWindow::setErrorMessages( const QStringList &errors, const QValueList<int> &lines, bool clear )
{
    if ( clear )
	errorView->clear();
    QStringList::ConstIterator mit = errors.begin();
    QValueList<int>::ConstIterator lit = lines.begin();
    for ( ; lit != lines.end() && mit != errors.end(); ++lit, ++mit )
	(void)new QListViewItem( errorView, *mit, QString::number( *lit ) );
    setCurrentPage( 1 );
}

DesignerOutputDock *OutputWindow::iFace()
{
    return new DesignerOutputDockImpl( this );
}

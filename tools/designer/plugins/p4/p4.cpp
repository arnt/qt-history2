#include "p4.h"
#include <qprocess.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include "../designerinterface.h"

QDict<P4Info> P4Info::files = QDict<P4Info>(53);

P4Action::P4Action( const QString &filename )
    : QObject(), file( filename ), process( 0 )
{
}

P4Action::~P4Action()
{
    delete process;
}

void P4Action::run( const QString &command )
{
    if ( !process ) {
	process = new QProcess;
	connect( process, SIGNAL( dataStdout( const QString & ) ),
		 this, SLOT( newData( const QString & ) ) );
	connect( process, SIGNAL( processExited() ),
		 this, SLOT( processExited() ) );
    }
    process->setArguments( QStringList::split( ' ', command ) );
    process->start();
}

void P4Action::newData( const QString &s )
{
    p4Data += s;
}

P4FStat::P4FStat( const QString& filename )
: P4Action( filename )
{
}

void P4FStat::fstat()
{
    run( QString("p4 fstat \"%1\"").arg( fileName() ) );
}

void P4FStat::processExited()
{
    P4Info* p4i = new P4Info;

    if ( data().find( "clientFile" ) == -1 ) {
	p4i->controlled = FALSE;
	p4i->opened = FALSE;
    } else {
	p4i->controlled = TRUE;
	int dpfIndex = data().find( "depotFile" );
	dpfIndex = data().find( ' ', dpfIndex, TRUE ) + 1;
	int dpfEnd = data().find( ' ', dpfIndex, TRUE );
	p4i->depotFile = data().mid( dpfIndex, dpfEnd-dpfIndex );
	if ( data().find( "... action edit" ) == -1 )
	    p4i->opened = FALSE;
	else
	    p4i->opened = TRUE;
    }
    emit finished( fileName(), p4i );

    delete this;
}

P4Edit::P4Edit( const QString &filename, QComponentInterface *iface, bool s )
    : P4Action( filename ), silent( s )
{
    QComponentInterface *sbIface = 0;
    if ( ( sbIface = iface->queryInterface( "DesignerStatusBarInterface" ) ) ) {
	sbIface->requestConnect( this, SIGNAL( showStatusBarMessage( const QString &, int ) ),
				 SLOT( message( const QString &, int ) ) );
    }
}

void P4Edit::edit()
{
    P4Info* p4i = P4Info::files[fileName()];
    if ( !p4i ) {
	P4FStat* fstat = new P4FStat( fileName() );
	fstat->fstat();
	connect( fstat, SIGNAL(finished( const QString&, P4Info* )), this, SLOT(fStatResults(const QString&,P4Info*) ) );
	run( QString( "p4 fstat \"%1\"").arg( fileName() ) );
    } fStatResults( fileName(), p4i );
}

void P4Edit::processExited()
{
    P4Info* p4i = P4Info::files[ fileName() ];
    if ( p4i )
	p4i->opened = TRUE;

    emit finished( fileName(), p4i );
    emit showStatusBarMessage( tr( "P4: File %1 opened for edit" ).arg( fileName() ), 3000 );

    delete this;
}

void P4Edit::fStatResults( const QString& filename, P4Info *p4i)
{
    P4Info::files.remove( filename );
    P4Info::files.insert( filename, p4i );
    if ( !p4i->controlled ) {
	if ( !silent )
	    QMessageBox::information( 0, tr( "P4 Edit" ), tr( "Opening the file\n%1\nfor edit failed!" ).arg( fileName() ) );
	else
	    emit showStatusBarMessage( tr( "P4: Opening file %1 for edit failed!" ).arg( fileName() ), 3000 );
	return;
    } 
    if ( !p4i->opened ) {
	if ( !silent ) {
	    if ( QMessageBox::information( 0, tr( "P4 Edit" ), tr( "The file\n%1\nis under Perforce Source Control and not " 
								   "opened for edit.\n"
								   "Do you want to open it for edit?" ).
					   arg( fileName() ),
					   tr( "&Yes" ), tr( "&No" ) ) == 1 ) {
		return;
	    }
	} else {
	    emit showStatusBarMessage( tr( "P4: Opening file %1 for edit..." ).arg( fileName() ), 0 );
	    run( QString("p4 edit \"%1\"").arg( fileName() ) );
	}
    }
}

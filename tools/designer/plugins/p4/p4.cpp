#include "p4.h"
#include <qprocess.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include "../designerinterface.h"

P4Edit::P4Edit( const QString &filename, QComponentInterface *iface, bool s )
    : QObject( iface ), fileName( filename ), process( 0 ), mwIface( iface ), silent( s )
{
    state = Done;
    QComponentInterface *sbIface = 0;
    if ( ( sbIface = mwIface->queryInterface( "DesignerStatusBarInterface" ) ) ) {
	sbIface->requestConnect( this, SIGNAL( showStatusBarMessage( const QString &, int ) ),
				 SLOT( message( const QString &, int ) ) );
    }
}

P4Edit::~P4Edit()
{
    delete process;
}

void P4Edit::edit()
{
    if ( state != Done )
	return;
    state = FStat;
    process = new QProcess;
    connect( process, SIGNAL( dataStdout( const QString & ) ),
	     this, SLOT( newData( const QString & ) ) );
    connect( process, SIGNAL( processExited() ),
	     this, SLOT( processExited() ) );
    QStringList args;
    args << "p4" << "fstat" << fileName;
    process->setArguments( args );
    process->start();
}

void P4Edit::newData( const QString &s )
{
    fstatData += s;
}

void P4Edit::processExited()
{
    if ( state == FStat ) {
	state = Edit;
	if ( fstatData.find( "clientFile" ) == -1 ) {
	    if ( !silent )
		QMessageBox::information( 0, tr( "P4 Edit" ), tr( "Opening the file\n%1\nfor edit failed!" ).arg( fileName ) );
	    else
		emit showStatusBarMessage( tr( "P4: Opening file %1 for edit failed!" ).arg( fileName ), 3000 );
	    state = Done;
	    return;
	}
	if ( fstatData.find( "... action edit" ) == -1 ) {
	    if ( !silent ) {
		if ( QMessageBox::information( 0, tr( "P4 Edit" ), tr( "The file\n%1\nis under Perforce Source Control and not " 
								       "opened for edit.\n"
								       "Do you want to open it for edit?" ).
					       arg( fileName ),
					       tr( "&Yes" ), tr( "&No" ) ) == 1 ) {
		    state = Done;
		    return;
		}
	    } else {
		emit showStatusBarMessage( tr( "P4: Opening file %1 for edit..." ).arg( fileName ), 0 );
	    }
	    QStringList args;
	    args << "p4" << "edit" << fileName;
	    process->setArguments( args );
	    process->start();
	} else {
	    state = Done;
	}
    } else if ( state == Edit ) {
	state = Done;
	{
	    emit showStatusBarMessage( tr( "P4: File %1 opened for edit" ).arg( fileName ), 3000 );
	}
    }
}

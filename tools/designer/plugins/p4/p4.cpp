#include "p4.h"
#include <qprocess.h>
#include <qmessagebox.h>
#include <qtextview.h>
#include "diffdialog.h"

QDict<P4Info> P4Info::files = QDict<P4Info>(53);

P4Action::P4Action( const QString &filename )
    : QObject(), file( filename ), process( 0 )
{
}

P4Action::~P4Action()
{
    delete process;
}

bool P4Action::run( const QString &command )
{
    if ( !process ) {
	process = new QProcess( this );
	connect( process, SIGNAL( dataStdout( const QString & ) ),
		 this, SLOT( newData( const QString & ) ) );
	connect( process, SIGNAL( processExited() ),
		 this, SLOT( processExited() ) );
    }
    process->setArguments( QStringList::split( ' ', command ) );
    return process->start();
}

void P4Action::newData( const QString &s )
{
    p4Data += s;
}

bool P4Action::success()
{
    return process && process->normalExit();
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

P4Sync::P4Sync( const QString &filename )
: P4Action( filename )
{
}

void P4Sync::sync()
{
    run( QString("p4 sync %1").arg( fileName() ) );
}

void P4Sync::processExited()
{
    emit showStatusBarMessage( data() );
    if ( success() ) {
	P4Info* p4i = P4Info::files[ fileName() ];
	if ( p4i )
	    p4i->controlled = TRUE;

	emit finished( fileName(), p4i );
    }

    delete this;
}

P4Edit::P4Edit( const QString &filename, bool s )
    : P4Action( filename ), silent( s )
{
}

void P4Edit::edit()
{
    P4Info* p4i = P4Info::files[fileName()];
    if ( !p4i ) {
	P4FStat* fstat = new P4FStat( fileName() );
	fstat->fstat();
	connect( fstat, SIGNAL(finished( const QString&, P4Info* )), this, SLOT(fStatResults(const QString&,P4Info*) ) );
	run( QString( "p4 fstat \"%1\"").arg( fileName() ) );
    }  else {
	fStatResults( fileName(), p4i );
    }
}

void P4Edit::processExited()
{
    emit showStatusBarMessage( data() );
    if ( success() ) {
	P4Info* p4i = P4Info::files[ fileName() ];
	if ( p4i )
	    p4i->opened = TRUE;

	emit finished( fileName(), p4i );
    }

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
	    emit showStatusBarMessage( tr( "P4: Opening file %1 for edit failed!" ).arg( fileName() ) );
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
	    emit showStatusBarMessage( tr( "P4: Opening file %1 for edit..." ).arg( fileName() ) );
	    run( QString("p4 edit \"%1\"").arg( fileName() ) );
	}
    }
}

P4Submit::P4Submit( const QString &filename )
: P4Action( filename )
{
}

void P4Submit::submit()
{
    run( QString("p4 submit %1").arg( fileName() ) );
}

void P4Submit::processExited()
{
    emit showStatusBarMessage( data() );
    if ( success() ) {
	P4Info* p4i = P4Info::files[ fileName() ];
	if ( p4i )
	    p4i->opened = FALSE;

	emit finished( fileName(), p4i );
    }

    delete this;
}


P4Revert::P4Revert( const QString &filename )
: P4Action( filename )
{
}

void P4Revert::revert()
{
    run( QString("p4 revert %1").arg( fileName() ) );
}

void P4Revert::processExited()
{
    emit showStatusBarMessage( data() );
    if ( success() ) {
	P4Info* p4i = P4Info::files[ fileName() ];
	if ( p4i )
	    p4i->opened = FALSE;

	emit finished( fileName(), p4i );
    }

    delete this;
}

P4Add::P4Add( const QString &filename )
: P4Action( filename )
{
}

void P4Add::add()
{
    run( QString("p4 add %1").arg( fileName() ) );
}

void P4Add::processExited()
{
    emit showStatusBarMessage( data() );
    if ( success() ) {
	P4Info* p4i = P4Info::files[ fileName() ];
	if ( p4i )
	    p4i->controlled = TRUE;

	emit finished( fileName(), p4i );
    }

    delete this;
}

P4Delete::P4Delete( const QString &filename )
: P4Action( filename )
{
}

void P4Delete::del()
{
    run( QString("p4 delete %1").arg( fileName() ) );
}

void P4Delete::processExited()
{
    emit showStatusBarMessage( data() );
    if ( success() ) {
	P4Info* p4i = P4Info::files[ fileName() ];
	if ( p4i )
	    p4i->controlled = FALSE;

	emit finished( fileName(), p4i );
    }

    delete this;
}

P4Diff::P4Diff( const QString &filename )
: P4Action( filename )
{
}

void P4Diff::diff()
{
    run( QString("p4 diff %1").arg( fileName() ) );
}

void P4Diff::processExited()
{
    if ( success() ) {
	int fstEndl = data().find( '\n' );
	QString caption = data().mid( 4, fstEndl-9 );
	caption = caption.replace( QRegExp("===="), "" );
	QString diff = data().mid( fstEndl+1 );
	if ( !!diff ) {
	    diff = diff.replace( QRegExp( "<" ), "&lt;" );
	    diff = diff.replace( QRegExp( ">"), "&gt;" );
	    diff = diff.replace( QRegExp( "\\n\\&lt;" ), "</pre><font color=\"red\"><pre>" );
	    diff = diff.replace( QRegExp( "\\n\\&gt;" ), "</pre><font color=\"blue\"><pre>" );
	    diff = "<font face=\"Courier\">" + diff + "</font>";
	    DiffDialog* dialog = new DiffDialog( 0, 0, TRUE );
	    dialog->setCaption( caption );
	    dialog->view->setText( diff );
	    dialog->exec();
	}
    }

    delete this;
}

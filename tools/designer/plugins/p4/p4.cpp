#include "p4.h"
#include <qprocess.h>
#include <qmessagebox.h>
#include <qtextview.h>
#include <qlistview.h>
#include <qheader.h>
#include <qmultilineedit.h>
#include "diffdialog.h"
#include "submitdialog.h"

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

void P4Action::updateStats()
{
    emit showStatusBarMessage( data() );
    P4FStat* fstat = new P4FStat( fileName() );
    connect( fstat, SIGNAL(finished(const QString&,P4Info*)), this, SLOT(newStats(const QString&,P4Info*)) );
    fstat->execute();
}

void P4Action::newData( const QString &s )
{
    p4Data += s;
}

void P4Action::newStats( const QString &s, P4Info *p4i )
{
    emit finished( s, p4i );
}

bool P4Action::success()
{
    return process && process->exitStatus() == 0;
}

P4FStat::P4FStat( const QString& filename )
: P4Action( filename )
{
}

bool P4FStat::execute()
{
    return run( QString("p4 fstat \"%1\"").arg( fileName() ) );
}

void P4FStat::processExited()
{
    P4Info *old = P4Info::files[ fileName() ];
    if ( old ) {
	P4Info::files.remove( fileName() );
	delete old;
    }

    P4Info* p4i = new P4Info;
    QStringList entries = QStringList::split( '\n', data() );

    if ( data().find( "clientFile" ) == -1 ) {
	p4i->controlled = FALSE;
	p4i->action = P4Info::None;
    } else {
	QStringList dfEntry = entries.grep( "depotFile" );
	if ( dfEntry.count() ) {
	    p4i->controlled = TRUE;
	    p4i->action = P4Info::None;
	    p4i->depotFile = QStringList::split( ' ', dfEntry[0] )[2];
	    QStringList actionEntry = entries.grep( "... action" );
	    if ( actionEntry.count() ) {
		QString act = QStringList::split( ' ', actionEntry[0] )[2];
		act.contains( "edit", FALSE );
		if ( act.contains( "edit", FALSE ) )
		    p4i->action = P4Info::Edit;
		else if ( act.contains( "add", FALSE ) )
		    p4i->action = P4Info::Add;
		else if ( act.contains( "delete", FALSE ) )
		    p4i->action = P4Info::Delete;
	    }
	}
    }
    P4Info::files.insert( fileName(), p4i );

    emit finished( fileName(), p4i );
    delete this;
}

P4Sync::P4Sync( const QString &filename )
: P4Action( filename )
{
}

bool P4Sync::execute()
{
    return run( QString("p4 sync %1").arg( fileName() ) );
}

void P4Sync::processExited()
{
    updateStats();
}

P4Edit::P4Edit( const QString &filename, bool s )
    : P4Action( filename ), silent( s )
{
}

bool P4Edit::execute()
{
    P4Info* p4i = P4Info::files[fileName()];
    if ( !p4i ) {
	P4FStat* fstat = new P4FStat( fileName() );
	connect( fstat, SIGNAL(finished( const QString&, P4Info* )), this, SLOT(fStatResults(const QString&,P4Info*) ) );
	return fstat->execute();
    }  else {
	fStatResults( fileName(), p4i );
    }
    return TRUE;
}

void P4Edit::processExited()
{
    updateStats();
}

void P4Edit::fStatResults( const QString& filename, P4Info *p4i)
{
    P4Info::files.remove( filename );
    P4Info::files.insert( filename, p4i );
    if ( !p4i->controlled ) {
	if ( silent )
	    QMessageBox::information( 0, tr( "P4 Edit" ), tr( "Opening the file\n%1\nfor edit failed!" ).arg( fileName() ) );
	else
	    emit showStatusBarMessage( tr( "P4: Opening file %1 for edit failed!" ).arg( fileName() ) );
	return;
    } 
    if ( p4i->action == P4Info::None ) {
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
	}
	run( QString("p4 edit \"%1\"").arg( fileName() ) );
    }
}

P4Submit::P4Submit( const QString &filename )
: P4Action( filename )
{
}

bool P4Submit::execute()
{
    SubmitDialog *dialog = new SubmitDialog( 0, 0, TRUE );
    dialog->fileList->header()->hide();

    QDictIterator<P4Info> it( P4Info::files );
    while ( it.current() ) {
	if ( it.current()->controlled && it.current()->action != P4Info::None )
	    (void)new QCheckListItem( dialog->fileList, it.currentKey(), QCheckListItem::CheckBox );
	++it;
    }

    if ( dialog->exec() != QDialog::Accepted )
	return FALSE;

    QString description = dialog->description->text().replace( QRegExp("\\n"), "\n\t" );
    QString buffer = "Description:\n\t";
    buffer += description + "\n";
    buffer += "Files:\n";

    QListViewItemIterator lvit( dialog->fileList );
    while ( lvit.current() ) {
	QCheckListItem* item = (QCheckListItem*)lvit.current();
	if ( !item->isOn() )
	    continue;
	P4Info* p4i = P4Info::files[ item->text( 0 ) ];
	if ( !p4i )
	    continue;
	buffer += "\t" + p4i->depotFile + "\n";
	++lvit;
    }

    qDebug( buffer );

    return TRUE;
//    run( QString("p4 submit -i %1 < %2").arg( fileName() ).arg( buffer ) );
}

void P4Submit::processExited()
{
    updateStats();
}


P4Revert::P4Revert( const QString &filename )
: P4Action( filename )
{
}

bool P4Revert::execute()
{
    if ( QMessageBox::information( 0, tr( "P4 Submit" ), tr( "Reverting will overwrite all changes to the file\n%1!\n"
					    "Proceed with revert?" ).
					    arg( fileName() ),
					    tr( "&Yes" ), tr( "&No" ) ) == 1 )
	return FALSE;

    return run( QString("p4 revert %1").arg( fileName() ) );
}

void P4Revert::processExited()
{
    updateStats();
}

P4Add::P4Add( const QString &filename )
: P4Action( filename )
{
}

bool P4Add::execute()
{
    return run( QString("p4 add %1").arg( fileName() ) );
}

void P4Add::processExited()
{
    updateStats();
}

P4Delete::P4Delete( const QString &filename )
: P4Action( filename )
{
}

bool P4Delete::execute()
{
    if ( QMessageBox::information( 0, tr( "P4 Submit" ), tr( "The file\n%1\nwill be deleted by the next sync.\n"
					    "Proceed with delete?" ).
					    arg( fileName() ),
					    tr( "&Yes" ), tr( "&No" ) ) == 1 )
	return FALSE;

    return run( QString("p4 delete %1").arg( fileName() ) );
}

void P4Delete::processExited()
{
    updateStats();
}

P4Diff::P4Diff( const QString &filename )
: P4Action( filename )
{
}

bool P4Diff::execute()
{
    return run( QString("p4 diff %1").arg( fileName() ) );
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

#include <qapplication.h>
#include <qlabel.h>
#include <qdir.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "uninstallimpl.h"
#include "../environment.h"

QApplication* app;
UninstallDlg* progress;

void rmDirRecursive( const QDir &dir )
{
    const QFileInfoList* list = dir.entryInfoList( QDir::All | QDir::System | QDir::Hidden );
    if ( list ) {
	QFileInfoListIterator it( *list );
	QFileInfo* fi;

	while( ( fi = it.current() ) ) {
	    if( ( fi->fileName() != "." ) && ( fi->fileName() != ".." ) ){
		progress->filesDisplay->append( fi->absFilePath() + "\n" );
		progress->filesDisplay->scrollToBottom();
		app->processEvents();
		if( fi->isDir() )
		    rmDirRecursive( QDir(fi->absFilePath()) );
		else
		    QFile::remove( fi->absFilePath() );
	    }
	    ++it;
	}
    }
    // Remove this dir as well
    dir.rmdir( dir.absPath() );
}

int main( int argc, char** argv )
{
    app = new QApplication( argc, argv );
    progress = new UninstallDlgImpl( 0, 0, 0, Qt::WStyle_Customize|Qt::WStyle_NormalBorder|Qt::WStyle_Title);
    
    if( argc != 4 )
	qFatal( "Incorrect parameters" );

    if( !QMessageBox::information( 0,
		QString( "Uninstalling Qt %1" ).arg(argv[3]),
		QString("Are you sure you want to uninstall Qt %1?\n"
		"This will remove the directory this version\n"
		"of Qt was installed to, along with ALL its contents.").arg(argv[3]),
		"Yes", "No" ) )
    {
	progress->setCaption( QString( "Uninstalling Qt " ) + argv[ 3 ] );
	progress->show();
    
	app->setMainWidget( progress );

	// Delete the two directories we have written files to during the installation.
	// The OK button is disabled at this point.
	// Messages will be processed during the delete process.
	
	// Check if moc.exe exists, if not this could potentially be a 
	// corrupted registry setting
	
	if ( QFile::exists( QString( argv[1] ) + QString( "\\bin\\moc.exe" ) ) )
	    rmDirRecursive( QDir(argv[1]) );
	else 
	    QMessageBox::warning( 0, "Uninstalling failed", "Qt could not be uninstalled, you will "
				  "need to remove Qt manually" );

	rmDirRecursive( QDir(argv[2]) );
    
	progress->okButton->setEnabled( true );
	progress->cleanRegButton->setEnabled( true );
	/*
	** Just hang around until someone clicks the "OK" button
	*/
	app->exec();
#if defined(Q_OS_WIN32)
	QEnvironment::removeUninstall( QString( "Qt " ) + argv[ 3 ] );
	QString qtEnv = QEnvironment::getEnv( "QTDIR", QEnvironment::LocalEnv );
	QString pathEnv = QEnvironment::getEnv( "PATH", QEnvironment::PersistentEnv );
	if ( qtEnv == QString(argv[1]) )
	    QEnvironment::removeEnv( "QTDIR", QEnvironment::LocalEnv | QEnvironment::PersistentEnv );
	else
	    qtEnv = argv[1];
	
	qtEnv.append("\\bin;");
	pathEnv.replace( qtEnv, "" );
	QEnvironment::putEnv( "PATH", pathEnv, QEnvironment::PersistentEnv );
#endif
    }

    return 0;
}

#include <qapplication.h>
#include <qlabel.h>
#include <qdir.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "uninstall.h"
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
    progress = new UninstallDlg;
    
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
	rmDirRecursive( QDir(argv[1]) );
	rmDirRecursive( QDir(argv[2]) );
    
	progress->okButton->setEnabled( true );
	/*
	** Just hang around until someone clicks the "OK" button
	*/
	app->exec();
	QEnvironment::removeUninstall( QString( "Qt " ) + argv[ 3 ] );
    }

    return 0;
}

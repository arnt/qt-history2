#include <qapplication.h>
#include <qlabel.h>
#include <qdir.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "uninstall.h"
#include "../environment.h"

QApplication* app;
UninstallDlg* progress;

void rmDirRecursive( QString dirPath )
{
    QDir dir( dirPath );
    const QFileInfoList* list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo* fi;

    while( ( fi = it.current() ) ) {
	if( ( fi->fileName() != "." ) && ( fi->fileName() != ".." ) ){
	    QString fileName = dirPath + QString( "\\" ) + fi->fileName();
	    progress->filesDisplay->append( fileName + "\n" );
	    app->processEvents();
	    if( fi->isDir() )
		rmDirRecursive( fileName );
	    else
		QFile::remove( fileName );
	}
	++it;
    }
    // Remove this dir as well
    dir.rmdir( dirPath );
}

int main( int argc, char** argv )
{
    app = new QApplication( argc, argv );
    progress = new UninstallDlg;
    
    if( argc != 4 )
	qFatal( "Incorrect parameters" );

    if( !QMessageBox::information( 0, QString( "Uninstalling Qt " ) + argv[ 3 ], "Are you sure you want to uninstall Qt?\nThis will remove the directory this version\nof Qt was installed to, along with ALL its contents.", "Yes", "No" ) )
    {
	progress->setCaption( QString( "Uninstalling Qt " ) + argv[ 3 ] );
	progress->show();
    
	app->setMainWidget( progress );

	// Delete the two directories we have written files to during the installation.
	// The OK button is disabled at this point.
	// Messages will be processed during the delete process.
	rmDirRecursive( argv[ 1 ] );
	rmDirRecursive( argv[ 2 ] );
    
	progress->okButton->setEnabled( true );
	/*
	** Just hang around until someone clicks the "OK" button
	*/
	app->exec();
	QEnvironment::removeUninstall( QString( "Qt " ) + argv[ 3 ] );
    }

    return 0;
}

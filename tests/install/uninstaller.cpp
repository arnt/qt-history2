#include <qapplication.h>
#include <qlabel.h>
#include <qdir.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include "confirmdlg.h"
#include "uninstall.h"
#include "environment.h"

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
	    progress->filesDisplay->append( fileName );
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
    ConfirmDlg dlg( progress );
    
    if( argc != 4 )
	qFatal( "Incorrect parameters" );

    progress->setCaption( QString( "Uninstalling Qt " ) + argv[ 3 ] );
    dlg.setCaption( QString( "Uninstalling Qt " ) + argv[ 3 ] );
    progress->show();
    dlg.confirmText->setText( "Do you really want to remove this version of Qt from your computer?" );
    if( !dlg.exec() )
	return 0;
    
    app->setMainWidget( progress );

    rmDirRecursive( argv[ 1 ] );
    rmDirRecursive( argv[ 2 ] );
    
    QEnvironment::removeUninstall( QString( "Qt " ) + argv[ 3 ] );
    progress->okButton->setEnabled( true );
    /*
    ** Just hang around until someone clicks the "OK" button
    */
    app->exec();

    return 0;
}

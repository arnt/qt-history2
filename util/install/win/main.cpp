#include <qapplication.h>
#include <qmessagebox.h>
#include "setupwizardimpl.h"
#include "resource.h"

#if defined(Q_OS_WIN32)
static bool addArchive( const QString& name )
{
    QByteArray ba ;

    // Copy the install.exe first, since we can't update our own application
    char aName[512];
    if ( GetModuleFileNameA( 0, aName, 512 ) == 0 ) { // we don't need wide character versions
	QMessageBox::critical( 0,
		"Could not add archive",
		QString( "Could not add archive %1.\n"
		    "Could not get the name of the application.").arg(name)
		);
	return FALSE;
    }
    QFile fromFile( aName );
    if ( !fromFile.open( IO_ReadOnly ) ) {
	QMessageBox::critical( 0,
		"Could not add archive",
		QString("Could not copy executable %1.\n").arg(aName)
		);
	return FALSE;
    }
    QString destinationName = name;
    if ( destinationName.right(4) == ".arq" ) {
	destinationName =destinationName.left( destinationName.length()-4 );
    }
    destinationName += ".exe";
    QFile toFile( destinationName );
    if ( !toFile.open( IO_WriteOnly ) ) {
	QMessageBox::critical( 0,
		"Could not add archive",
		QString("Could not copy executable %1 to %2.\n").arg(aName).arg(destinationName)
		);
	return FALSE;
    }
    ba = fromFile.readAll();
    toFile.writeBlock( ba );
    toFile.close();

    // load the .arq file
    QFile fArq( name );
    if ( !fArq.open( IO_ReadOnly ) ) {
	QMessageBox::critical( 0,
		"Could not add archive",
		QString("Could not open archive %1.\n").arg(name)
		);
	return FALSE;
    }
    ba = fArq.readAll();

    // update the binary res
    ResourceSaver res( destinationName );
    QString errorMsg;
    if ( !res.setData( "QT_ARQ", ba, &errorMsg ) ) {
	QMessageBox::critical( 0,
		"Could not add archive",
		QString("Could not add archive %1.\n").arg(name) + errorMsg
		);
	return FALSE;
    }

    QMessageBox::information( 0,
	    "Archive added",
	    QString("Added the archive %1.\n").arg(name) + errorMsg
	    );
    return TRUE;
}
#endif


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    SetupWizardImpl* w;
    bool reconfig( false );
    int res( -1 );

    for( int i = 0; i < app.argc(); i++ ) {
	if( QString( app.argv()[i] ) == "-reconfig" ) {
	    reconfig = true;
	    break;
#if defined(Q_OS_WIN32)
	} else if ( QString( app.argv()[i] ) == "-add-archive" ) {
	    // -add-archive is an internal option to add the
	    // binary resource QT_ARQ
	    if ( ++i < app.argc() ) {
		if ( addArchive( app.argv()[i] ) )
		    return 0;
	    }
	    return res;
#endif
	}
    }

    if( w = new SetupWizardImpl( NULL, NULL, false, 0, reconfig ) ) {
	w->show();

	app.setMainWidget( w );

	res = app.exec();

	w->stopProcesses();
    }

    return res;
}

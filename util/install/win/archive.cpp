#include "archive.h"
#include "resource.h"

#include <qfile.h>
#include <qmessagebox.h>
#include <windows.h>

bool addArchive( const QString& name )
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

#if 0
    QMessageBox::information( 0,
	    "Archive added",
	    QString("Added the archive %1.\n").arg(name) + errorMsg
	    );
#endif
    return TRUE;
}

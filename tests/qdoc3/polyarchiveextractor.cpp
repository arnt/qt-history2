/*
  polyarchiveextractor.cpp
*/

#include <qprocess.h>

#include "polyarchiveextractor.h"

PolyArchiveExtractor::PolyArchiveExtractor( const QStringList& extensions,
					    const QString& commandLine )
    : correctExts( extensions ), cmd( commandLine )
{
}

PolyArchiveExtractor::~PolyArchiveExtractor()
{
}

bool PolyArchiveExtractor::recognizeExtension( const QString& ext )
{
    return correctExts.contains( ext );
}

void PolyArchiveExtractor::extractArchive( const Location& location,
					   const QString& filePath,
					   const QString& outputDir )
{
    QString realCommand = cmd;
    for ( int i = realCommand.length() - 1; i >= 0; i-- ) {
	if ( realCommand[i].unicode() == 1 ) {
	    realCommand.replace( i, 1, filePath );
	} else if ( realCommand[i].unicode() == 2 ) {
	    realCommand.replace( i, 1, outputDir );	
	}
    }
    QStringList args = QStringList::split( ' ', realCommand );

    QProcess process( args );
    if ( !process.start() )
	location.fatal( tr("Couldn't launch the %1 tool").arg(args[0]),
			tr("Make sure the tool is installed and in the"
			   " path.") );
    while ( process.isRunning() )
	;

    /*
      The qsauncompress tool performs no error checking. The safest
      way to find out if something went wrong is to read standard
      error for QFile errors.
    */
    QByteArray errors = process.readStderr();
    if ( !errors.isEmpty() )
	location.fatal( tr("The %1 tool encountered some problems")
			.arg(args[0]) );
}

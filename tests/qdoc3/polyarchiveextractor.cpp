/*
  polyarchiveextractor.cpp
*/

#include <qprocess.h>

#include "polyarchiveextractor.h"

PolyArchiveExtractor::PolyArchiveExtractor( const QStringList& extensions,
					    const QString& commandLine )
    : ArchiveExtractor( extensions ), cmd( commandLine )
{
}

PolyArchiveExtractor::~PolyArchiveExtractor()
{
}

void PolyArchiveExtractor::extractArchive( const Location& location,
					   const QString& filePath,
					   const QString& outputDir )
{
    QString actualCommand = cmd;
    for ( int i = actualCommand.length() - 1; i >= 0; i-- ) {
	if ( actualCommand[i].unicode() == 1 ) {
	    actualCommand.replace( i, 1, filePath );
	} else if ( actualCommand[i].unicode() == 2 ) {
	    actualCommand.replace( i, 1, outputDir );
	}
    }
    QString toolName = actualCommand;
    int space = toolName.find( " " );
    if ( space != -1 )
	toolName.truncate( space );

    QProcess process( QStringList() << "sh" << "-c" << actualCommand );
    if ( !process.start() )
	location.fatal( tr("Couldn't launch the '%1' archive extractor")
			.arg(toolName),
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
	location.fatal( tr("The '%1' archive extractor encountered some"
			   " problems")
			.arg(toolName) );
}

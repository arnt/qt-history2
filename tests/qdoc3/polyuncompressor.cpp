/*
  polyarchiveextractor.cpp
*/

#include <qprocess.h>

#include "polyuncompressor.h"

PolyUncompressor::PolyUncompressor( const QStringList& extensions,
				    const QString& commandLine )
    : Uncompressor( extensions ), cmd( commandLine )
{
}

PolyUncompressor::~PolyUncompressor()
{
}

QString PolyUncompressor::uncompressedFilePath( const QString& filePath )
{
    QStringList::ConstIterator e = fileExtensions().begin();
    while ( e != fileExtensions().end() ) {
	QString dotExt = "." + *e;
	if ( filePath.endsWith(dotExt) )
	    return filePath.left( filePath.length() - dotExt.length() );
	++e;
    }
    return filePath + ".out"; // doesn't really matter
}

void PolyUncompressor::uncompressFile( const Location& location,
				       const QString& filePath,
				       const QString& outputFilePath )
{
    QString actualCommand = cmd;
    for ( int i = actualCommand.length() - 1; i >= 0; i-- ) {
	if ( actualCommand[i].unicode() == 1 ) {
	    actualCommand.replace( i, 1, filePath );
	} else if ( actualCommand[i].unicode() == 2 ) {
	    actualCommand.replace( i, 1, outputFilePath );
	}
    }
    QString toolName = actualCommand;
    int space = toolName.find( " " );
    if ( space != -1 )
	toolName.truncate( space );

    QProcess process( QStringList() << "sh" << "-c" << actualCommand );
    if ( !process.start() )
	location.fatal( tr("Couldn't launch the '%1' uncompressor")
			.arg(toolName),
			tr("Make sure the tool is installed and in the"
			   " path.") );
    while ( process.isRunning() )
	;

    QByteArray errors = process.readStderr();
    if ( !errors.isEmpty() )
	location.fatal( tr("The '%1' uncompressor encountered some problems")
			.arg(toolName) );
}

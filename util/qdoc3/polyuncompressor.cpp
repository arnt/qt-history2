/*
  polyarchiveextractor.cpp
*/

#include "command.h"
#include "polyuncompressor.h"

PolyUncompressor::PolyUncompressor( const QStringList& extensions,
				    const QString& commandFormat )
    : Uncompressor( extensions ), cmd( commandFormat )
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
    executeCommand( location, cmd,
		    QStringList() << filePath << outputFilePath );
}

/*
  polyarchiveextractor.cpp
*/

#include "command.h"
#include "polyarchiveextractor.h"

PolyArchiveExtractor::PolyArchiveExtractor( const QStringList& extensions,
					    const QString& commandFormat )
    : ArchiveExtractor( extensions ), cmd( commandFormat )
{
}

PolyArchiveExtractor::~PolyArchiveExtractor()
{
}

void PolyArchiveExtractor::extractArchive( const Location& location,
					   const QString& filePath,
					   const QString& outputDir )
{
    executeCommand( location, cmd, QStringList() << filePath << outputDir );
}

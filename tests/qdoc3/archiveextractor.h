/*
  archiveextractor.h
*/

#ifndef ARCHIVEEXTRACTOR_H
#define ARCHIVEEXTRACTOR_H

#include "location.h"

class ArchiveExtractor
{
public:
    ArchiveExtractor();
    virtual ~ArchiveExtractor();

    virtual bool recognizeExtension( const QString& ext ) = 0;
    virtual void extractArchive( const Location& location,
				 const QString& filePath,
				 const QString& outputDir ) = 0;

    static ArchiveExtractor *extractorForFileName( const QString& fileName );

private:
    static QValueList<ArchiveExtractor *> extractors;
};

#endif

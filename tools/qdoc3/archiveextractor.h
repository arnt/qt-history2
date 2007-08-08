/*
  archiveextractor.h
*/

#ifndef ARCHIVEEXTRACTOR_H
#define ARCHIVEEXTRACTOR_H

#include <qstringlist.h>

#include "location.h"

class ArchiveExtractor
{
public:
    ArchiveExtractor( const QStringList& extensions );
    virtual ~ArchiveExtractor();

    virtual void extractArchive( const Location& location,
				 const QString& filePath,
				 const QString& outputDir ) = 0;

    static ArchiveExtractor *extractorForFileName( const QString& fileName );
    static ArchiveExtractor *uncompressorForFileName( const QString& fileName );

protected:
    const QStringList& fileExtensions() { return fileExts; }

private:
    QStringList fileExts;

    static QList<ArchiveExtractor *> extractors;
};

#endif

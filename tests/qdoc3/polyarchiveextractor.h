/*
  polyarchiveextractor.h
*/

#ifndef POLYARCHIVEEXTRACTOR_H
#define POLYARCHIVEEXTRACTOR_H

#include "archiveextractor.h"

class PolyArchiveExtractor : public ArchiveExtractor
{
public:
    PolyArchiveExtractor( const QStringList& extensions,
			  const QString& commandFormat );
    ~PolyArchiveExtractor();

    virtual void extractArchive( const Location& location,
				 const QString& filePath,
				 const QString& outputDir );

private:
    QString cmd;
};

#endif

/*
  archiveextractor.cpp
*/

#include "archiveextractor.h"

QValueList<ArchiveExtractor *> ArchiveExtractor::extractors;

ArchiveExtractor::ArchiveExtractor()
{
    extractors.prepend( this );
}

ArchiveExtractor::~ArchiveExtractor()
{
    extractors.remove( this );
}

ArchiveExtractor *ArchiveExtractor::extractorForFileName(
	const QString& fileName )
{
    QString ext;
    int dot = fileName.findRev( '.' );
    if ( dot != -1 )
	ext = fileName.mid( dot + 1 );

    QValueList<ArchiveExtractor *>::ConstIterator e = extractors.begin();
    while ( e != extractors.end() ) {
	if ( (*e)->recognizeExtension(ext) )
	    return *e;
	++e;
    }
    return 0;
}

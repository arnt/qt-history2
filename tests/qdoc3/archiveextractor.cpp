/*
  archiveextractor.cpp
*/

#include "archiveextractor.h"

QList<ArchiveExtractor *> ArchiveExtractor::extractors;

ArchiveExtractor::ArchiveExtractor( const QStringList& extensions )
    : fileExts( extensions )
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
    int dot = -1;
    while ( (dot = fileName.indexOf(".", dot + 1)) != -1 ) {
	QString ext = fileName.mid( dot + 1 );
	QList<ArchiveExtractor *>::ConstIterator e = extractors.begin();
	while ( e != extractors.end() ) {
	    if ( (*e)->fileExtensions().contains(ext) )
		return *e;
	    ++e;
	}
    }
    return 0;
}

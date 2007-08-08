/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    extractors.removeAll( this );
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

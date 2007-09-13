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
  archiveextractor.h
*/

#ifndef ARCHIVEEXTRACTOR_H
#define ARCHIVEEXTRACTOR_H

#include <qstringlist.h>

#include "location.h"

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

#endif

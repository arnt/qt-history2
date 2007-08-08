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

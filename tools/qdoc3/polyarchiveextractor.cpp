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
  polyarchiveextractor.cpp
*/

#include "command.h"
#include "polyarchiveextractor.h"

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

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
  uncompressor.h
*/

#ifndef UNCOMPRESSOR_H
#define UNCOMPRESSOR_H

#include <qstringlist.h>

#include "location.h"

QT_BEGIN_NAMESPACE

class Uncompressor
{
public:
    Uncompressor( const QStringList& extensions );
    virtual ~Uncompressor();

    virtual QString uncompressedFilePath( const QString& filePath ) = 0;
    virtual void uncompressFile( const Location& location,
				 const QString& filePath,
				 const QString& outputFilePath ) = 0;

    static Uncompressor *uncompressorForFileName( const QString& fileName );

protected:
    const QStringList& fileExtensions() const { return fileExts; }

private:
    QStringList fileExts;

    static QList<Uncompressor *> uncompressors;
};

QT_END_NAMESPACE

#endif

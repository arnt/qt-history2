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
  polyuncompressor.h
*/

#ifndef POLYUNCOMPRESSOR_H
#define POLYUNCOMPRESSOR_H

#include "uncompressor.h"

QT_BEGIN_NAMESPACE

class PolyUncompressor : public Uncompressor
{
public:
    PolyUncompressor( const QStringList& extensions,
		      const QString& commandFormat );
    ~PolyUncompressor();

    virtual QString uncompressedFilePath( const QString& filePath );
    virtual void uncompressFile( const Location& location,
				 const QString& filePath,
				 const QString& outputFilePath );

private:
    QString cmd;
};

QT_END_NAMESPACE

#endif

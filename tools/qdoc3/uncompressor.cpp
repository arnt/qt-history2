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
  uncompressor.cpp
*/

#include "uncompressor.h"

QList<Uncompressor *> Uncompressor::uncompressors;

Uncompressor::Uncompressor( const QStringList& extensions )
    : fileExts( extensions )
{
    uncompressors.prepend( this );
}

Uncompressor::~Uncompressor()
{
    uncompressors.removeAll( this );
}

Uncompressor *Uncompressor::uncompressorForFileName( const QString& fileName )
{
    int dot = -1;
    while ( (dot = fileName.indexOf(".", dot + 1)) != -1 ) {
        QString ext = fileName.mid( dot + 1 );
        QList<Uncompressor *>::ConstIterator u = uncompressors.begin();
        while ( u != uncompressors.end() ) {
            if ( (*u)->fileExtensions().contains(ext) )
                return *u;
            ++u;
        }
    }
    return 0;
}

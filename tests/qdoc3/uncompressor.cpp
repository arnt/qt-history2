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

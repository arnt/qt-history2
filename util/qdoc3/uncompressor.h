/*
  uncompressor.h
*/

#ifndef UNCOMPRESSOR_H
#define UNCOMPRESSOR_H

#include <qstringlist.h>

#include "location.h"

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

#endif

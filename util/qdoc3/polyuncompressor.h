/*
  polyuncompressor.h
*/

#ifndef POLYUNCOMPRESSOR_H
#define POLYUNCOMPRESSOR_H

#include "uncompressor.h"

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

#endif

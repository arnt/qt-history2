/*
  binarywriter.h
*/

#ifndef BINARYWRITER_H
#define BINARYWRITER_H

#include <qglobal.h>

#include <stdio.h>

class QString;

/*
  The BinaryWriter class provides binary output to file. The data can be
  provided as plain bytes or in Base64 encoding.
*/
class BinaryWriter
{
public:
    BinaryWriter( const QString& fileName );
    ~BinaryWriter();

    void puts( const char *str );
    void putsBase64( const char *str );

private:
#if defined(Q_DISABLE_COPY)
    BinaryWriter( const BinaryWriter& );
    BinaryWriter& operator=( const BinaryWriter& );
#endif

    static int toBase64Digit( int ch );

    FILE *out;
    uint buf;
    int nbits;
};

#endif

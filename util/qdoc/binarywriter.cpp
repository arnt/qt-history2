/*
  binarywriter.cpp
*/

#include <qfile.h>

#include "binarywriter.h"
#include "config.h"
#include "messages.h"

BinaryWriter::BinaryWriter( const QString& fileName )
{
    QString filePath = config->outputDir() + QChar( '/' ) + fileName;
    out = fopen( QFile::encodeName(filePath), "w" );
    buf = 0;
    nbits = 0;

    if ( out == 0 ) {
	syswarning( "Cannot open '%s' for writing binary data",
		    filePath.latin1() );
	return;
    }
}

BinaryWriter::~BinaryWriter()
{
    if ( out != 0 )
	fclose( out );
}

void BinaryWriter::puts( const char *str )
{
    if ( out == 0 )
	return;

    fputs( str, out );
}


void BinaryWriter::putsBase64( const char *str )
{
    if ( out == 0 )
	return;

    int ch;
    while ( (ch = *str++) != '\0' ) {
	int d = toBase64Digit( ch );
	if ( d != -1 ) {
	    buf = ( buf << 6 ) | d;
	    nbits += 6;
	    if ( nbits >= 8 ) {
		nbits -= 8;
		putc( buf >> nbits, out );
		buf &= ( 1 << nbits ) - 1;
	    }
	}
    }
}

int BinaryWriter::toBase64Digit( int ch )
{
    /*
      This is slow but sufficient for qdoc.
    */
    if ( ch >= 'A' && ch <= 'Z' )
	return ch - 'A';
    else if ( ch >= 'a' && ch <= 'z' )
	return ch - 'a' + 26;
    else if ( ch >= '0' && ch <= '9' )
	return ch - '0' + 52;
    else if ( ch == '+' )
	return 62;
    else if ( ch == '/' )
	return 63;
    else
	return -1;
}

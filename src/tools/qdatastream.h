/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatastream.h#1 $
**
** Definition of QStream class
**
** Author  : Haavard Nord
** Created : 930831
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QSTREAM_H
#define QSTREAM_H

#include "qglobal.h"


// Stream mode encoding

#define Stream_Data		1		// stream format
#define Stream_Data7bit		2
#define Stream_TypeMask		0x0f

#define Stream_ReadOnly		0x10		// stream access
#define Stream_WriteOnly	0x20
#define Stream_ReadWrite	0x30
#define Stream_AccessMask	0xf0

#define Stream_Null		0


class QStream					// stream class
{
public:
    QStream();
    virtual ~QStream();

    int		 mode()	  const { return smode; }
    int		 format() const { return smode & Stream_TypeMask; }
    int		 access() const { return smode & Stream_AccessMask; }

    virtual bool open( int mode ) = 0;		// open stream in Stream_Mode
    virtual bool close() = 0;			// close stream
    virtual bool flush() = 0;			// flush stream

    virtual long size() = 0;			// get stream size
    virtual long at();				// get stream pointer
    virtual bool at( long );			// set stream pointer
    virtual bool atEnd();			// test if at end of stream
    bool	 reset() { return at(0); }	// reset data pointer

    virtual QStream& _read( char *p, uint len ) = 0;
    virtual QStream& _write( const char *p, uint len ) = 0;

    virtual int	 getch() = 0;			// get next char
    virtual int	 putch( int ) = 0;		// put char
    virtual int	 ungetch( int ) = 0;		// put back char

    QStream	&read( INT8 & );		// read integer
    QStream	&read( UINT8 &i )  { return read((INT8&)i); }
    QStream	&read( INT16 & );
    QStream	&read( UINT16 &i ) { return read((INT16&)i); }
    QStream	&read( INT32 & );
    QStream	&read( UINT32 &i ) { return read((INT32&)i); }
    QStream	&read( int & );
    QStream	&read( uint & );

    QStream	&read( float & );		// read floating point
    QStream	&read( double & );

    QStream	&read( char *& );		// read char array
    QStream	&read( char *&, uint &len );
    QStream	&readBytes( char *, uint len );

    QStream	&write( INT8 );			// write integer
    QStream	&write( UINT8 i )  { return write((INT8)i); }
    QStream	&write( INT16 );
    QStream	&write( UINT16 i ) { return write((INT16)i); }
    QStream	&write( INT32 );
    QStream	&write( UINT32 i ) { return write((INT32)i); }
    QStream	&write( int i )	   { return write((INT32)i); }
    QStream	&write( uint i )   { return write((UINT32)i); }

    QStream	&write( float );		// write floating point
    QStream	&write( double );

    QStream	&write( const char * );		// write char array
    QStream	&write( const char *, uint len );
    QStream	&writeBytes( const char *, uint len );

protected:
    int		 smode;				// stream mode
    long	 ptr;				// stream pointer
    bool	 setMode( int );		// set stream mode
};


// --------------------------------------------------------------------------
// iostream-like operators for QStream read/write
//

inline QStream &operator>>( QStream &s, INT8 &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, UINT8 &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, INT16 &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, UINT16 &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, INT32 &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, UINT32 &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, int &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, uint &i )
{
    return s.read( i );
}

inline QStream &operator>>( QStream &s, float &f )
{
    return s.read( f );
}

inline QStream &operator>>( QStream &s, double &f )
{
    return s.read( f );
}

inline QStream &operator>>( QStream &s, char *&str )
{
    return s.read( str );
}

inline QStream &operator<<( QStream &s, INT8 i )
{
    return s.write( i );
}

inline QStream &operator<<( QStream &s, UINT8 i )
{
    return s.write( i );
}

inline QStream &operator<<( QStream &s, INT16 i )
{
    return s.write( i );
}

inline QStream &operator<<( QStream &s, UINT16 i )
{
    return s.write( i );
}

inline QStream &operator<<( QStream &s, INT32 i )
{
    return s.write( i );
}

inline QStream &operator<<( QStream &s, UINT32 i )
{
    return s.write( i );
}

inline QStream &operator<<( QStream &s, int i )
{
    return s.write( (INT32)i );
}

inline QStream &operator<<( QStream &s, uint i )
{
    return s.write( (UINT32)i );
}

inline QStream &operator<<( QStream &s, float f )
{
    return s.write( f );
}

inline QStream &operator<<( QStream &s, double f )
{
    return s.write( f );
}

inline QStream &operator<<( QStream &s, const char *str )
{
    return s.write( str );
}


#endif // QSTREAM_H

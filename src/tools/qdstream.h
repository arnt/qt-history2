/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdstream.h#2 $
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

#include "qiodev.h"


// Stream format

#define Stream_Data		1		// 8 bit data (default)
#define Stream_Data7bit		2		// 7 bit printable


class QStream					// stream class
{
public:
    QStream();
    QStream( QIODevice * );
    virtual ~QStream();

    QIODevice 	*device() const;		// get current stream device
    void	 setDevice( QIODevice * );	// set stream device

    int	  	 format() const;		// get stream format
    void  	 setFormat( int );		// set stream format

    long	 size()  const;			// get stream device size
    long	 at()    const;			// get stream device index
    bool	 at( long );			// set stream device index
    bool	 atEnd() const;			// test if at end of device
    bool	 reset();			// reset stream device index

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

private:
    QIODevice   *dev;				// I/O device
    int	  	 frmt;				// stream format
};


// --------------------------------------------------------------------------
// QStream inline functions
//

inline QIODevice *QStream::device() const	{ return dev; }
inline void QStream::setDevice( QIODevice *d )	{ dev = d; }

inline int QStream::format() const		{ return frmt; }
inline void QStream::setFormat( int f )		{ frmt = f; }

inline long QStream::size()  const		{ return dev->size(); }
inline long QStream::at()    const		{ return dev->at(); }
inline bool QStream::at( long index )		{ return dev->at(index); }
inline bool QStream::atEnd() const		{ return dev->atEnd(); }
inline bool QStream::reset()			{ return dev->reset(); }


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

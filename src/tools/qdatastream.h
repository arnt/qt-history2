/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatastream.h#10 $
**
** Definition of QDataStream class
**
** Author  : Haavard Nord
** Created : 930831
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDSTREAM_H
#define QDSTREAM_H

#include "qiodev.h"
#include "qstring.h"


class QDataStream				// data stream class
{
public:
    QDataStream();
    QDataStream( QIODevice * );
    QDataStream( QByteArray, int mode );
    QDataStream( QByteArray );
    virtual ~QDataStream();

    QIODevice	*device() const;		// get current stream device
    void	 setDevice( QIODevice * );	// set stream device
    void	 unsetDevice();			// set NULL stream device

    bool	 eos() const;			// end of stream data?

    enum ByteOrder { BigEndian, LittleEndian };
    int		 byteOrder()	const;
    void	 setByteOrder( int );

    bool	 isPrintableData() const;	// using printable data
    void	 setPrintableData( bool );	// set printable data on/off

    QDataStream &operator>>( INT8 &i );
    QDataStream &operator>>( UINT8 &i );
    QDataStream &operator>>( INT16 &i );
    QDataStream &operator>>( UINT16 &i );
    QDataStream &operator>>( INT32 &i );
    QDataStream &operator>>( UINT32 &i );
    QDataStream &operator>>( int &i );
    QDataStream &operator>>( uint &i );
    QDataStream &operator>>( float &f );
    QDataStream &operator>>( double &f );
    QDataStream &operator>>( char *&str );
    QDataStream &operator<<( INT8 i );
    QDataStream &operator<<( UINT8 i );
    QDataStream &operator<<( INT16 i );
    QDataStream &operator<<( UINT16 i );
    QDataStream &operator<<( INT32 i );
    QDataStream &operator<<( UINT32 i );
    QDataStream &operator<<( int i );
    QDataStream &operator<<( uint i );
    QDataStream &operator<<( float f );
    QDataStream &operator<<( double f );
    QDataStream &operator<<( const char *str );

    QDataStream &readBytes( char *&, uint &len );
    QDataStream &readRawBytes( char *, uint len );

    QDataStream &writeBytes( const char *, uint len );
    QDataStream &writeRawBytes( const char *, uint len );

private:
    QIODevice	*dev;				// I/O device
    bool	 owndev;			// internally created device
    int		 byteorder;			// serialization byte order
    bool	 printable;			// printable data
    bool	 noswap;			// byte swapping not needed
};


// --------------------------------------------------------------------------
// QDataStream inline functions
//

inline QIODevice *QDataStream::device() const
{ return dev; }

inline bool QDataStream::eos() const
{ return dev ? dev->atEnd() : TRUE; }

inline int QDataStream::byteOrder() const
{ return byteorder; }

inline bool QDataStream::isPrintableData() const
{ return printable; }

inline void QDataStream::setPrintableData( bool p )
{ printable = p; }

inline QDataStream &QDataStream::operator>>( UINT8 &i )
{ return *this >> (INT8&)i; }

inline QDataStream &QDataStream::operator>>( UINT16 &i )
{ return *this >> (INT16&)i; }

inline QDataStream &QDataStream::operator>>( UINT32 &i )
{ return *this >> (INT32&)i; }

inline QDataStream &QDataStream::operator<<( UINT8 i )
{ return *this << (INT8)i; }

inline QDataStream &QDataStream::operator<<( UINT16 i )
{ return *this << (INT16)i; }

inline QDataStream &QDataStream::operator<<( UINT32 i )
{ return *this << (INT32)i; }

inline QDataStream &QDataStream::operator<<( int i )
{ return *this << (INT32)i; }

inline QDataStream &QDataStream::operator<<( uint i )
{ return *this << (UINT32)i; }


#endif // QDSTREAM_H

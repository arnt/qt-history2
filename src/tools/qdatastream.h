/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatastream.h#20 $
**
** Definition of QDataStream class
**
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
    virtual ~QDataStream();

    QIODevice	*device() const;
    void	 setDevice( QIODevice * );
    void	 unsetDevice();

    bool	 eof() const;

    enum ByteOrder { BigEndian, LittleEndian };
    int		 byteOrder()	const;
    void	 setByteOrder( int );

    bool	 isPrintableData() const;
    void	 setPrintableData( bool );

    QDataStream &operator>>( INT8 &i );
    QDataStream &operator>>( UINT8 &i );
    QDataStream &operator>>( INT16 &i );
    QDataStream &operator>>( UINT16 &i );
    QDataStream &operator>>( INT32 &i );
    QDataStream &operator>>( UINT32 &i );
    QDataStream &operator>>( float &f );
    QDataStream &operator>>( double &f );
    QDataStream &operator>>( char *&str );

    QDataStream &operator<<( INT8 i );
    QDataStream &operator<<( UINT8 i );
    QDataStream &operator<<( INT16 i );
    QDataStream &operator<<( UINT16 i );
    QDataStream &operator<<( INT32 i );
    QDataStream &operator<<( UINT32 i );
    QDataStream &operator<<( float f );
    QDataStream &operator<<( double f );
    QDataStream &operator<<( const char *str );

    QDataStream &readBytes( char *&, uint &len );
    QDataStream &readRawBytes( char *, uint len );

    QDataStream &writeBytes( const char *, uint len );
    QDataStream &writeRawBytes( const char *, uint len );

private:
    QIODevice	*dev;
    bool	 owndev;
    int		 byteorder;
    bool	 printable;
    bool	 noswap;

private:	// Disabled copy constructor and operator=
    QDataStream( const QDataStream & ) {}
    QDataStream &operator=( const QDataStream & ) { return *this; }
};


/*****************************************************************************
  QDataStream inline functions
 *****************************************************************************/

inline QIODevice *QDataStream::device() const
{ return dev; }

inline bool QDataStream::eof() const
{ return dev ? dev->atEnd() : FALSE; }

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


#endif // QDSTREAM_H

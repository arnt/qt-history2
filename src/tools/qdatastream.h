/****************************************************************************
**
** Definition of QDataStream class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDATASTREAM_H
#define QDATASTREAM_H

#ifndef QT_H
#include <qiodevice.h>
#endif // QT_H

template<typename T> class QList;
template<typename T> class QLinkedList;

#ifndef QT_NO_DATASTREAM
class Q_EXPORT QDataStream				// data stream class
{
public:
    QDataStream();
    QDataStream( QIODevice * );
    QDataStream( QByteArray &, int mode );
    QDataStream( const QByteArray &, int mode );
    virtual ~QDataStream();

    QIODevice	*device() const;
    void	 setDevice( QIODevice * );
    void	 unsetDevice();

    bool	 atEnd() const;
    bool	 eof() const;

    enum ByteOrder { BigEndian, LittleEndian };
    int		 byteOrder()	const;
    void	 setByteOrder( int );

    bool	 isPrintableData() const;
    void	 setPrintableData( bool );

    int		 version() const;
    void	 setVersion( int );

    QDataStream &operator>>( Q_INT8 &i );
    QDataStream &operator>>( Q_UINT8 &i );
    QDataStream &operator>>( Q_INT16 &i );
    QDataStream &operator>>( Q_UINT16 &i );
    QDataStream &operator>>( Q_INT32 &i );
    QDataStream &operator>>( Q_UINT32 &i );
    QDataStream &operator>>( Q_INT64 &i );
    QDataStream &operator>>( Q_UINT64 &i );
    QDataStream &operator>>( Q_LONG &i );
    QDataStream &operator>>( Q_ULONG &i );

    QDataStream &operator>>( float &f );
    QDataStream &operator>>( double &f );
    QDataStream &operator>>( char *&str );

    QDataStream &operator<<( Q_INT8 i );
    QDataStream &operator<<( Q_UINT8 i );
    QDataStream &operator<<( Q_INT16 i );
    QDataStream &operator<<( Q_UINT16 i );
    QDataStream &operator<<( Q_INT32 i );
    QDataStream &operator<<( Q_UINT32 i );
    QDataStream &operator<<( Q_INT64 i );
    QDataStream &operator<<( Q_UINT64 i );
    QDataStream &operator<<( Q_LONG i );
    QDataStream &operator<<( Q_ULONG i );
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
    bool	 printable;
    bool	 noswap;
    int		 byteorder;
    int		 ver;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDataStream( const QDataStream & );
    QDataStream &operator=( const QDataStream & );
#endif
};


/*****************************************************************************
  QDataStream inline functions
 *****************************************************************************/

inline QIODevice *QDataStream::device() const
{ return dev; }

inline bool QDataStream::atEnd() const
{ return dev ? dev->atEnd() : TRUE; }

inline bool QDataStream::eof() const
{ return atEnd(); }

inline int QDataStream::byteOrder() const
{ return byteorder; }

inline bool QDataStream::isPrintableData() const
{ return printable; }

inline void QDataStream::setPrintableData( bool p )
{ printable = p; }

inline int QDataStream::version() const
{ return ver; }

inline void QDataStream::setVersion( int v )
{ ver = v; }

inline QDataStream &QDataStream::operator>>( Q_UINT8 &i )
{ return *this >> (Q_INT8&)i; }

inline QDataStream &QDataStream::operator>>( Q_UINT16 &i )
{ return *this >> (Q_INT16&)i; }

inline QDataStream &QDataStream::operator>>( Q_UINT32 &i )
{ return *this >> (Q_INT32&)i; }

inline QDataStream &QDataStream::operator>>( Q_UINT64 &i )
{ return *this >> (Q_INT64&)i; }

inline QDataStream &QDataStream::operator>>( Q_ULONG &i )
{ return *this >> (Q_LONG&)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT8 i )
{ return *this << (Q_INT8)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT16 i )
{ return *this << (Q_INT16)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT32 i )
{ return *this << (Q_INT32)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT64 i )
{ return *this << (Q_INT64)i; }

inline QDataStream &QDataStream::operator<<( Q_ULONG i )
{ return *this << (Q_LONG)i; }


template <typename T>
QDataStream& operator>>( QDataStream& s, QList<T>& l )
{
    l.clear();
    Q_UINT32 c;
    s >> c;
    for( Q_UINT32 i = 0; i < c; ++i )
    {
	T t;
	s >> t;
	l.append( t );
	if ( s.atEnd() )
	    break;
    }
    return s;
}

template <typename T>
QDataStream& operator<<( QDataStream& s, const QList<T>& l )
{
    s << (Q_UINT32)l.size();
    for (int i = 0; i < l.size(); ++i)
	s << l.at(i);
    return s;
}

template <typename T>
QDataStream& operator>>( QDataStream& s, QLinkedList<T>& l )
{
    l.clear();
    Q_UINT32 c;
    s >> c;
    for( Q_UINT32 i = 0; i < c; ++i )
    {
	T t;
	s >> t;
	l.append( t );
	if ( s.atEnd() )
	    break;
    }
    return s;
}

template <typename T>
QDataStream& operator<<( QDataStream& s, const QLinkedList<T>& l )
{
    s << (Q_UINT32)l.size();
    typename QLinkedList<T>::ConstIterator it = l.constBegin();
    for( ; it != l.constEnd(); ++it )
	s << *it;
    return s;
}

#endif // QT_NO_DATASTREAM
#endif // QDATASTREAM_H

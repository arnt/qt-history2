/****************************************************************************
**
** Definition of QBuffer class.
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

#ifndef QBUFFER_H
#define QBUFFER_H

#ifndef QT_H
#include "qiodevice.h"
#include "qbytearray.h"
#endif // QT_H


class Q_CORE_EXPORT QBuffer : public QIODevice
{
public:
    QBuffer();
    QBuffer( QByteArray &b );
    QBuffer( const QByteArray &b );
   ~QBuffer();

    QByteArray &buffer() const;
    bool  setBuffer( QByteArray &b );

    bool  open( int );
    void  close();
    void  flush();

    Offset size() const;
    Offset at() const;
    bool  at( Offset );

    Q_LONG	  readBlock( char *p, Q_ULONG );
    Q_LONG	  writeBlock( const char *p, Q_ULONG );
    Q_LONG	  writeBlock( const QByteArray& data )
	      { return QIODevice::writeBlock(data); }
    Q_LONG	  readLine( char *p, Q_ULONG );

    int	  getch();
    int	  putch( int );
    int	  ungetch( int );

private:
    QByteArray b, *p;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QBuffer( const QBuffer & );
    QBuffer &operator=( const QBuffer & );
#endif
};


inline QByteArray &QBuffer::buffer() const
{ return const_cast<QByteArray &>(b); }

inline QIODevice::Offset QBuffer::size() const
{ return (Offset)b.size(); }

inline QIODevice::Offset QBuffer::at() const
{ return ioIndex; }


#endif // QBUFFER_H

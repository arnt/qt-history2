/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbuffer.h#15 $
**
** Definition of QBuffer class
**
** Created : 930812
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBUFFER_H
#define QBUFFER_H

#ifndef QT_H
#include "qiodev.h"
#include "qstring.h"
#endif // QT_H


class QBuffer : public QIODevice		// buffer I/O device class
{
public:
    QBuffer();
    QBuffer( QByteArray );
   ~QBuffer();

    QByteArray buffer() const;
    bool  setBuffer( QByteArray );

    bool  open( int );
    void  close();
    void  flush();

    uint  size() const;
    int	  at()	 const;
    bool  at( int );

    int	  readBlock( char *p, uint );
    int	  writeBlock( const char *p, uint );
    int	  readLine( char *p, uint );

    int	  getch();
    int	  putch( int );
    int	  ungetch( int );

protected:
    QByteArray a;

private:
    uint  a_len;
    uint  a_inc;

private:	// Disabled copy constructor and operator=
    QBuffer( const QBuffer & );
    QBuffer &operator=( const QBuffer & );
};


inline QByteArray QBuffer::buffer() const
{ return a; }

inline uint QBuffer::size() const
{ return a.size(); }

inline int QBuffer::at() const
{ return index; }


#endif // QBUFFER_H

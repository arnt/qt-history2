/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbuffer.h#21 $
**
** Definition of QBuffer class
**
** Created : 930812
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QBUFFER_H
#define QBUFFER_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QBuffer : public QIODevice
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
#if defined(Q_DISABLE_COPY)
    QBuffer( const QBuffer & );
    QBuffer &operator=( const QBuffer & );
#endif
};


inline QByteArray QBuffer::buffer() const
{ return a; }

inline uint QBuffer::size() const
{ return a.size(); }

inline int QBuffer::at() const
{ return index; }


#endif // QBUFFER_H

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbuffer.h#23 $
**
** Definition of QBuffer class
**
** Created : 930812
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the tools
** module and therefore may only be used if the tools module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
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
{ return ioIndex; }


#endif // QBUFFER_H

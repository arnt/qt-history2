/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbuffer.h#2 $
**
** Definition of QBuffer class
**
** Author  : Haavard Nord
** Created : 930812
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QBUFFER_H
#define QBUFFER_H

#include "qiodev.h"
#include "qstring.h"


class QBuffer : public QIODevice		// buffer I/O device class
{
public:
    QBuffer();
    QBuffer( QByteArray );
   ~QBuffer();

    QByteArray buffer() const { return a; }	// get buffer
    bool  setBuffer( QByteArray );		// set buffer

    bool  open( int );				// open buffer
    void  close();				// close buffer
    void  flush();				// flush buffer

    long  size() const { return a.size(); }	// get buffer size
    long  at()	 const { return index; }	// get buffer index
    bool  at( long );				// set buffer index

    int   readBlock( char *p, uint uint );
    int   writeBlock( const char *p, uint uint );

    int	  getch();				// get next char
    int	  putch( int );				// put char
    int	  ungetch( int ) ;			// put back char

protected:
    QByteArray a;				// byte array

private:
    uint  a_len;				// byte array real length
    uint  a_inc;				// byte array increment
};


#endif // QBUFFER_H

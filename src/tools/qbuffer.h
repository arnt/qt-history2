/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbuffer.h#1 $
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

#include "qstream.h"
#include "qstring.h"


class QBuffer : public QStream			// buffer class
{
public:
    QBuffer();
    QBuffer( QByteArray );
   ~QBuffer();

    QByteArray buffer() const { return a; }	// get buffer
    bool  setBuffer( QByteArray );		// set buffer

    bool  open( int );				// open buffer in Stream_Mode
    bool  close();				// close buffer
    bool  flush();				// flush buffer

    long  size();				// get buffer size
    long  at()	{ return ptr; }			// get buffer pointer
    bool  at( long );				// set buffer pointer

    QStream& _read( char *p, uint len );	// read data from buffer
    QStream& _write( const char *p, uint len ); // write data into buffer

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

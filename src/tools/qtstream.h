/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtstream.h#1 $
**
** Definition of QTextStream class
**
** Author  : Haavard Nord
** Created : 940922
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QTSTREAM_H
#define QTSTREAM_H

#include "qiodev.h"


class QTextStream;

typedef void (QTextStream::*QTSMF)();		// member function pointer
typedef QTextStream & (*QTSFUNC)(QTextStream &);

class QTextStream				// text stream class
{
public:
    QTextStream();
    QTextStream( QIODevice * );
    virtual ~QTextStream();

    QIODevice 	*device() const;		// get current stream device
    void	 setDevice( QIODevice * );	// set stream device
    void	 resetDevice();			// set NULL stream device

    QTextStream &operator>>( char & );
    QTextStream &operator>>( signed short & );
    QTextStream &operator>>( unsigned short & );
    QTextStream &operator>>( signed int & );
    QTextStream &operator>>( unsigned int & );
    QTextStream &operator>>( signed long & );
    QTextStream &operator>>( unsigned long & );
    QTextStream &operator>>( float & );
    QTextStream &operator>>( double & );
    QTextStream &operator>>( char *& );

    QTextStream &operator<<( char );
    QTextStream &operator<<( signed short );
    QTextStream &operator<<( unsigned short );
    QTextStream &operator<<( signed int );
    QTextStream &operator<<( unsigned int );
    QTextStream &operator<<( signed long );
    QTextStream &operator<<( unsigned long );
    QTextStream &operator<<( float );
    QTextStream &operator<<( double );
    QTextStream &operator<<( const char * );
    QTextStream &operator<<( QTSMF );
    QTextStream &operator<<( QTSFUNC );

    void	 bin();
    void	 oct();
    void	 dec();
    void	 hex();
    void	 endl();
    void	 flush();

private:
    QTextStream &output_int( int, ulong, bool );
    QIODevice   *dev;				// I/O device
    int		 base;
};

/*
typedef QTextStream QTS;

QTextStream &hex( QTextStream &s );
*/

// --------------------------------------------------------------------------
// QTextStream inline functions
//

inline QIODevice *QTextStream::device() const	  { return dev; }
inline void QTextStream::setDevice(QIODevice *d ) { dev = d; }
inline void QTextStream::resetDevice()		  { dev = 0; }


#endif // QTSTREAM_H

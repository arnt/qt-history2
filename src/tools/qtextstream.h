/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtextstream.h#4 $
**
** Definition of QTextStream class
**
** Author  : Haavard Nord
** Created : 940922
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTSTREAM_H
#define QTSTREAM_H

#include "qiodev.h"
#include <stdio.h>


class QTextStream				// text stream class
{
public:
    QTextStream();
    QTextStream( QIODevice * );
    QTextStream( FILE * );
    virtual ~QTextStream();

    QIODevice 	*device() const;		// get current stream device
    void	 setDevice( QIODevice * );	// set stream device
    void	 unsetDevice();			// set NULL stream device

    bool	 eos() const;			// end of stream data?

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
    QTextStream &operator<<( void * );		// any pointer

    enum {
	skipws	  = 0x0001,			// skip whitespace on input
	left	  = 0x0002,			// left-adjust output
	right	  = 0x0004,			// right-adjust output
	internal  = 0x0008,			// pad after sign
	bin	  = 0x0010,			// binary format integer
	oct	  = 0x0020,			// octal format integer
	dec	  = 0x0040,			// decimal format integer
	hex	  = 0x0080,			// hex format integer
	showbase  = 0x0100,			// show base indicator
	showpoint = 0x0200,			// force decimal point (float)
	uppercase = 0x0400,			// upper-case hex output
	showpos	  = 0x0800,			// add '+' to positive integers
	scientific= 0x1000,			// scientific float output
	fixed	  = 0x2000,			// fixed float output
    };
    
    static const long basefield;		// bin | oct | dec | hex
    static const long adjustfield;		// left | right | internal
    static const long floatfield;		// scientific | fixed

    long  flags() const;			// get/set flags
    long  flags( long f );
    long  setf( long bits );
    long  setf( long bits, long mask );
    long  unsetf( long bits );

    void  reset();				// set default flags

    int   width() const;			// get/set field width
    int   width( int );
    int   fill() const;				// get/set fill char
    int   fill( int );
    int   precision() const;			// get/set float precision
    int   precision( int );

private:
    long	 input_int();
    QTextStream &output_int( int, ulong, bool );
    QIODevice   *dev;				// I/O device
    long	 fflags;			// formatting flags
    int		 fwidth;			// field width
    int		 fillchar;			// fill char
    int		 fprec;				// float precision
    bool	 fstrm;				// using file stream (cheat flag)
    bool	 owndev;			// self-created device
};

typedef QTextStream QTS;


// --------------------------------------------------------------------------
// QTextStream inline functions
//

inline QIODevice *QTextStream::device() const
{ return dev; }

inline bool QTextStream::eos() const
{ return dev ? dev->atEnd() : TRUE; }

inline long QTextStream::flags() const
{ return fflags; }

inline long QTextStream::flags( long newf )
{ long f = fflags;  fflags = newf;  return f; }

inline long QTextStream::setf( long bits )
{ long f = fflags;  fflags |= bits;  return f; }

inline long QTextStream::setf( long bits, long mask )
{ long f = fflags;  fflags = (fflags & ~mask) | (bits & mask);  return f; }

inline long QTextStream::unsetf( long mask )
{ long f = fflags;  fflags &= ~mask;  return f; }

inline int QTextStream::width() const
{ return fwidth; }

inline int QTextStream::width( int w )
{ int oldw = fwidth;  fwidth = w;  return oldw;  }

inline int QTextStream::fill() const
{ return fillchar; }

inline int QTextStream::fill( int f )
{ int oldc = fillchar;  fillchar = f;  return oldc;  }

inline int QTextStream::precision() const
{ return fprec; }

inline int QTextStream::precision( int p )
{ int oldp = fprec;  fprec = p;  return oldp;  }


// --------------------------------------------------------------------------
// QTextStream manipulators
//

typedef QTextStream & (*QTSFUNC)(QTextStream &);// manipulator function
typedef int (QTextStream::*QTSMFI)(int);	// manipulator w/int argument

class QTSManip {				// text stream manipulator
public:
    QTSManip( QTSMFI m, int a )	{ mf=m; arg=a; }
    void exec( QTextStream &s ) { (s.*mf)(arg); }
private:
    QTSMFI mf;					// QTextStream member function
    int	   arg;					// member function argument
};

inline QTextStream &operator>>( QTextStream &s, QTSFUNC f )
{ return (*f)( s ); }

inline QTextStream &operator<<( QTextStream &s, QTSFUNC f )
{ return (*f)( s ); }

inline QTextStream &operator<<( QTextStream &s, QTSManip m )
{ m.exec(s); return s; }

QTextStream &bin( QTextStream &s );		// set bin notation
QTextStream &oct( QTextStream &s );		// set oct notation
QTextStream &dec( QTextStream &s );		// set dec notation
QTextStream &hex( QTextStream &s );		// set hex notation
QTextStream &endl( QTextStream &s );		// insert EOL ('\n')
QTextStream &flush( QTextStream &s );		// flush output
QTextStream &ws( QTextStream &s );		// eat whitespace on input
QTextStream &reset( QTextStream &s );		// set default flags

inline QTSManip setw( int w )
{ return QTSManip(QTextStream::width,w); }

inline QTSManip setfill( int f )
{ return QTSManip(QTextStream::fill,f); }

inline QTSManip setprecision( int p )
{ return QTSManip(QTextStream::precision,p); }


#endif // QTSTREAM_H

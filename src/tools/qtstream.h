/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtstream.h#14 $
**
** Definition of QTextStream class
**
** Author  : Haavard Nord
** Created : 940922
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTSTREAM_H
#define QTSTREAM_H

#include "qiodev.h"
#include "qstring.h"
#include <stdio.h>


class QTextStream				// text stream class
{
public:
    QTextStream();
    QTextStream( QIODevice * );
    QTextStream( QByteArray, int mode );
    QTextStream( FILE *, int mode );
    virtual ~QTextStream();

    QIODevice	*device() const;
    void	 setDevice( QIODevice * );
    void	 unsetDevice();

    bool	 eof() const;

    QTextStream &operator>>( char & );
    QTextStream &operator>>( signed short & );
    QTextStream &operator>>( unsigned short & );
    QTextStream &operator>>( signed int & );
    QTextStream &operator>>( unsigned int & );
    QTextStream &operator>>( signed long & );
    QTextStream &operator>>( unsigned long & );
    QTextStream &operator>>( float & );
    QTextStream &operator>>( double & );
    QTextStream &operator>>( char * );
    QTextStream &operator>>( QString & );

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

    QTextStream &readRawBytes( char *, uint len );
    QTextStream &writeRawBytes( const char *, uint len );

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
	fixed	  = 0x2000			// fixed float output
    };

    static const int basefield;			// bin | oct | dec | hex
    static const int adjustfield;		// left | right | internal
    static const int floatfield;		// scientific | fixed

    int	  flags() const;
    int	  flags( int f );
    int	  setf( int bits );
    int	  setf( int bits, int mask );
    int	  unsetf( int bits );

    void  reset();

    int	  width()	const;
    int	  width( int );
    int	  fill()	const;
    int	  fill( int );
    int	  precision()	const;
    int	  precision( int );

private:
    long	 input_int();
    QTextStream &output_int( int, ulong, bool );
    QIODevice	*dev;
    int		 fflags;
    int		 fwidth;
    int		 fillchar;
    int		 fprec;
    bool	 fstrm;
    bool	 owndev;

private:	//Disabled copy constructor and operator=
    QTextStream( const QTextStream & ) {}
    QTextStream &operator=( const QTextStream & ) { return *this; }
};

typedef QTextStream QTS;


// --------------------------------------------------------------------------
// QTextStream inline functions
//

inline QIODevice *QTextStream::device() const
{ return dev; }

inline bool QTextStream::eof() const
{ return dev ? dev->atEnd() : FALSE; }

inline int QTextStream::flags() const
{ return fflags; }

inline int QTextStream::flags( int f )
{ int oldf = fflags;  fflags = f;  return oldf; }

inline int QTextStream::setf( int bits )
{ int oldf = fflags;  fflags |= bits;  return oldf; }

inline int QTextStream::setf( int bits, int mask )
{ int oldf = fflags;  fflags = (fflags & ~mask) | (bits & mask); return oldf; }

inline int QTextStream::unsetf( int bits )
{ int oldf = fflags;  fflags &= ~bits;	return oldf; }

inline int QTextStream::width() const
{ return fwidth; }

inline int QTextStream::width( int w )
{ int oldw = fwidth;  fwidth = w;  return oldw;	 }

inline int QTextStream::fill() const
{ return fillchar; }

inline int QTextStream::fill( int f )
{ int oldc = fillchar;	fillchar = f;  return oldc;  }

inline int QTextStream::precision() const
{ return fprec; }

inline int QTextStream::precision( int p )
{ int oldp = fprec;  fprec = p;	 return oldp;  }


// --------------------------------------------------------------------------
// QTextStream manipulators
//

typedef QTextStream & (*QTSFUNC)(QTextStream &);// manipulator function
typedef int (QTextStream::*QTSMFI)(int);	// manipulator w/int argument

class QTSManip {				// text stream manipulator
public:
    QTSManip( QTSMFI m, int a ) { mf=m; arg=a; }
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
{ return QTSManip(&QTextStream::width,w); }

inline QTSManip setfill( int f )
{ return QTSManip(&QTextStream::fill,f); }

inline QTSManip setprecision( int p )
{ return QTSManip(&QTextStream::precision,p); }


#endif // QTSTREAM_H

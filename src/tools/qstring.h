/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.h#97 $
**
** Definition of the QString class, extended char array operations,
** and QByteArray and QCString classes
**
** Created : 920609
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

#ifndef QSTRING_H
#define QSTRING_H

#ifndef QT_H
#include "qcstring.h"
#endif // QT_H


/*****************************************************************************
  QString class
 *****************************************************************************/

class QRegExp;


class Q_EXPORT QChar {
public:
    // The alternatives just avoid order-of-construction warnings.
#if defined(_WS_X11_) || defined(_OS_WIN32_BYTESWAP_)
    QChar() : row(0), cell(0) { }
    QChar( char c ) : row(0), cell(c) { }
    QChar( uchar c ) : row(0), cell(c) { }
    QChar( uchar c, uchar r ) : row(r), cell(c) { }
    QChar( const QChar& c ) : row(c.row), cell(c.cell) { }
    QChar( ushort rc ) : row((rc>>8)&0xff), cell(rc&0xff) { }
    QChar( short rc ) : row((rc>>8)&0xff), cell(rc&0xff) { }
    QChar( uint rc ) : row((rc>>8)&0xff), cell(rc&0xff) { }
    QChar( int rc ) : row((rc>>8)&0xff), cell(rc&0xff) { }
#else
    QChar() : cell(0), row(0) { }
    QChar( char c ) : cell(c), row(0) { }
    QChar( uchar c ) : cell(c), row(0) { }
    QChar( uchar c, uchar r ) : cell(c), row(r) { }
    QChar( const QChar& c ) : cell(c.cell), row(c.row) { }
    QChar( ushort rc ) : cell(rc&0xff), row((rc>>8)&0xff) { }
    QChar( short rc ) : cell(rc&0xff), row((rc>>8)&0xff) { }
    QChar( uint rc ) : cell(rc&0xff), row((rc>>8)&0xff) { }
    QChar( int rc ) : cell(rc&0xff), row((rc>>8)&0xff) { }
#endif

    QT_STATIC_CONST QChar null;            // 0000
    QT_STATIC_CONST QChar replacement;     // FFFD
    QT_STATIC_CONST QChar byteOrderMark;     // FEFF
    QT_STATIC_CONST QChar byteOrderSwapped;     // FFFE

    bool isSpace() const;

    operator char() const { return row?0:cell; }

    friend int operator==( const QChar& c1, const QChar& c2 );
    friend int operator==( const QChar& c1, char c );
    friend int operator==( char ch, const QChar& c );
    friend int operator!=( const QChar& c1, const QChar& c2 );
    friend int operator!=( const QChar& c, char ch );
    friend int operator!=( char ch, const QChar& c );
    friend int operator<=( const QChar& c1, const QChar& c2 );
    friend int operator<=( const QChar& c1, char c );
    friend int operator<=( char ch, const QChar& c );
    friend int operator>=( const QChar& c1, const QChar& c2 );
    friend int operator>=( const QChar& c, char ch );
    friend int operator>=( char ch, const QChar& c );
    friend int operator<( const QChar& c1, const QChar& c2 );
    friend int operator<( const QChar& c1, char c );
    friend int operator<( char ch, const QChar& c );
    friend int operator>( const QChar& c1, const QChar& c2 );
    friend int operator>( const QChar& c, char ch );
    friend int operator>( char ch, const QChar& c );

#if defined(_WS_X11_) || defined(_OS_WIN32_BYTESWAP_)
    // XChar2b on X11, ushort on _OS_WIN32_BYTESWAP_
    uchar row;
    uchar cell;
    enum { networkOrdered = 1 }; // ### Net... or net...?
#else
    // ushort on _OS_WIN32_
    uchar cell;
    uchar row;
    enum { networkOrdered = 0 };
#endif
};

inline int operator==( char ch, const QChar& c )
{
    return ch == c.cell && !c.row;
}

inline int operator==( const QChar& c, char ch )
{
    return ch == c.cell && !c.row;
}

inline int operator==( const QChar& c1, const QChar& c2 )
{
    return c1.cell == c2.cell
	&& c1.row == c2.row;
}

inline int operator!=( const QChar& c1, const QChar& c2 )
{
    return c1.cell != c2.cell
	|| c1.row != c2.row;
}

inline int operator!=( char ch, const QChar& c )
{
    return ch != c.cell || c.row;
}

inline int operator!=( const QChar& c, char ch )
{
    return ch != c.cell || c.row;
}

inline int operator<=( const QChar& c, char ch )
{
    return !(ch < c.cell || c.row);
}

inline int operator<=( char ch, const QChar& c )
{
    return ch <= c.cell || c.row;
}

inline int operator<=( const QChar& c1, const QChar& c2 )
{
    return c1.row > c2.row
	? FALSE
	: c1.row < c2.row
	    ? TRUE
	    : c1.row <= c2.row;
}

inline int operator>=( const QChar& c, char ch ) { return ch <= c; }
inline int operator>=( char ch, const QChar& c ) { return c <= ch; }
inline int operator>=( const QChar& c1, const QChar& c2 ) { return c2 <= c1; }
inline int operator<( const QChar& c, char ch ) { return !(ch<=c); }
inline int operator<( char ch, const QChar& c ) { return !(c<=ch); }
inline int operator<( const QChar& c1, const QChar& c2 ) { return !(c2<=c1); }
inline int operator>( const QChar& c, char ch ) { return !(ch>=c); }
inline int operator>( char ch, const QChar& c ) { return !(c>=ch); }
inline int operator>( const QChar& c1, const QChar& c2 ) { return !(c2>=c1); }


class Q_EXPORT QString
{
public:
    QString();					// make null string
    QString( const QChar& );			// one-char string
    QString( const QString & );			// impl-shared copy
    QString( const QByteArray& );		// deep copy
    QString( QChar* unicode, uint length );	// deep copy
    QString( const char *str );			// deep copy
    QString( const char *str, uint maxSize );	// deep copy, max length
    ~QString();

    QString    &operator=( const QString & );	// impl-shared copy
    QString    &operator=( const char * );	// deep copy
    QString    &operator=( const QByteArray& );	// deep copy

    QT_STATIC_CONST QString null;

    bool	isNull()	const;
    bool	isEmpty()	const;
    uint	length()	const;
    void	truncate( uint pos );
    void	resize( uint pos ); // OBS
    void	fill( QChar c, int len = -1 );

    QString	copy()	const;

    QString arg(int a, int fieldwidth=0) const;
    QString arg(uint a, int fieldwidth=0) const;
    QString arg(char a, int fieldwidth=0) const;
    QString arg(QChar a, int fieldwidth=0) const;
    QString arg(const QString& a, int fieldwidth=0) const;
    QString arg(double a, int fieldwidth=0, char fmt='g', int prec=-1);

    QString    &sprintf( const char* format, ... )
#if defined(_CC_GNU_)
	__attribute__ ((format (printf, 2, 3)))
#endif
	;

    int		find( QChar c, int index=0, bool cs=TRUE ) const;
    int		find( char c, int index=0, bool cs=TRUE ) const
		    { return find(QChar(c), index, cs); }
    int		find( const QString &str, int index=0, bool cs=TRUE ) const;
    int		find( const QRegExp &, int index=0 ) const;
    int		find( const char* str, int index=0 ) const
		    { return find(QString(str), index); }
    int		findRev( QChar c, int index=-1, bool cs=TRUE) const;
    int		findRev( char c, int index=-1, bool cs=TRUE) const
		    { return findRev( QChar(c), index, cs ); }
    int		findRev( const QString &str, int index=-1, bool cs=TRUE) const;
    int		findRev( const QRegExp &, int index=-1 ) const;
    int		findRev( const char* str, int index=-1 ) const
		    { return findRev(QString(str), index); }
    int		contains( QChar c, bool cs=TRUE ) const;
    int		contains( char c, bool cs=TRUE ) const
		    { return contains(QChar(c), cs); }
    int		contains( const char* str, bool cs=TRUE ) const;
    int		contains( const QString &str, bool cs=TRUE ) const;
    int		contains( const QRegExp & ) const;

    QString	left( uint len )  const;
    QString	right( uint len ) const;
    QString	mid( uint index, uint len=0xffffffff) const;

    QString	leftJustify( uint width, QChar fill=' ', bool trunc=FALSE)const;
    QString	rightJustify( uint width, QChar fill=' ',bool trunc=FALSE)const;

    QString	lower() const;
    QString	upper() const;

    QString	stripWhiteSpace()	const;
    QString	simplifyWhiteSpace()	const;

    QString    &insert( uint index, const QString & );
    QString    &insert( uint index, const QChar*, uint len );
    QString    &insert( uint index, QChar );
    QString    &insert( uint index, char c ) { return insert(index,QChar(c)); }
    QString    &append( char );
    QString    &append( const QString & );
    QString    &prepend( char );
    QString    &prepend( const QString & );
    QString    &remove( uint index, uint len );
    QString    &replace( uint index, uint len, const QString & );
    QString    &replace( uint index, uint len, const QChar*, uint clen );
    QString    &replace( const QRegExp &, const QString & );

    short	toShort( bool *ok=0 )	const;
    ushort	toUShort( bool *ok=0 )	const;
    int		toInt( bool *ok=0 )	const;
    uint	toUInt( bool *ok=0 )	const;
    long	toLong( bool *ok=0 )	const;
    ulong	toULong( bool *ok=0 )	const;
    float	toFloat( bool *ok=0 )	const;
    double	toDouble( bool *ok=0 )	const;

    QString    &setStr( const char* );
    QString    &setNum( short );
    QString    &setNum( ushort );
    QString    &setNum( int );
    QString    &setNum( uint );
    QString    &setNum( long );
    QString    &setNum( ulong );
    QString    &setNum( float, char f='g', int prec=6 );
    QString    &setNum( double, char f='g', int prec=6 );

    void	setExpand( uint index, QChar c );

    QString    &operator+=( const QString &str );
    QString    &operator+=( QChar c );
    QString    &operator+=( char c );

    // Your compiler is smart enough to use the const one if it can.
    const QChar& at( uint i ) const
	{ return i<d->len ? unicode()[i] : QChar::null; }
    QChar& at( uint i )
	{ // Optimized for easy-inlining by simple compilers.
	    if (d->count!=1 || i>=d->len)
		subat(i);
	    d->dirtyascii=1;
	    return d->unicode[i];
	}
    const QChar& operator[]( int i ) const { return at((uint)i); }
    QChar& operator[]( int i ) { return at((uint)i); }

    const QChar* unicode() const { return d->unicode; }
    const char* ascii() const;
    QCString utf8() const;
    static QString fromUtf8(const char*, int len=-1);
    operator const char *() const { return ascii(); }

    static QChar* asciiToUnicode( const char*, uint * len, uint maxlen=(uint)-1 );
    static QChar* asciiToUnicode( const QByteArray&, uint * len );
    static char* unicodeToAscii( const QChar*, uint len );
    int compare( const QString& s ) const;
    static int compare( const QString& s1, const QString& s2 )
	{ return s1.compare(s2); }

    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QString & );

#ifndef QT_NO_COMPAT
    const char* data() const { return ascii(); }
    void detach() { }
    uint size() const;
#endif

private:
    QString( int size );			// allocate size incl. \0
    void deref();
    void real_detach();
    void setLength( uint pos );
    void subat( uint );
    bool findArg(int& pos, int& len) const;

    struct Data : public QShared {
	Data() :
	    unicode(0), ascii(0), len(0), maxl(0), dirtyascii(0) { ref(); }
	Data(QChar *u, uint l, uint m) :
	    unicode(u), ascii(0), len(l), maxl(m), dirtyascii(0) { }
	~Data() { if ( unicode ) delete [] unicode;
		  if ( ascii ) delete [] ascii; }
	QChar *unicode;
	char *ascii;
	uint len;
	uint maxl:30;
	uint dirtyascii:1;
    };
    Data *d;
    static Data* shared_null;
    friend int ucstrcmp( const QString &a, const QString &b );

    friend class QConstString;
    QString(Data* dd) : d(dd) { }
};

class QConstString : private QString {
public:
    QConstString( QChar* unicode, uint length );
    ~QConstString();
    const QString& string() const { return *this; }
};


/*****************************************************************************
  QString stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QString & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QString & );


/*****************************************************************************
  QString inline functions
 *****************************************************************************/

// No safe way to pre-init shared_null on ALL compilers/linkers.
inline QString::QString() :
    d(shared_null ? shared_null : shared_null=new Data)
{
    d->ref();
}

//inline QString &QString::operator=( const QString &s )
//{ return (const QString &)assign( s ); }

//inline QString &QString::operator=( const char *str )
//{ return (const QString &)duplicate( str, strlen(str)+1 ); }

inline bool QString::isNull() const
{ return unicode() == 0; }

inline uint QString::length() const
{ return d->len; }

#ifndef QT_NO_COMPAT
inline uint QString::size() const
{ return length()+1; }
#endif

inline bool QString::isEmpty() const
{ return length() == 0; }

inline QString QString::copy() const
{ return QString( *this ); }

inline QString &QString::prepend( const QString & s )
{ return insert(0,s); }

inline QString &QString::prepend( char c )
{ return insert(0,c); }

inline QString &QString::append( const QString & s )
{ return operator+=(s); }

inline QString &QString::append( char c )
{ return operator+=(c); }

inline QString &QString::setNum( short n )
{ return setNum((long)n); }

inline QString &QString::setNum( ushort n )
{ return setNum((ulong)n); }

inline QString &QString::setNum( int n )
{ return setNum((long)n); }

inline QString &QString::setNum( uint n )
{ return setNum((ulong)n); }

inline QString &QString::setNum( float n, char f, int prec )
{ return setNum((double)n,f,prec); }



/*****************************************************************************
  QString non-member operators
 *****************************************************************************/

Q_EXPORT bool operator==( const QString &s1, const QString &s2 );
Q_EXPORT bool operator==( const QString &s1, const char *s2 );
Q_EXPORT bool operator==( const char *s1, const QString &s2 );
Q_EXPORT bool operator!=( const QString &s1, const QString &s2 );
Q_EXPORT bool operator!=( const QString &s1, const char *s2 );
Q_EXPORT bool operator!=( const char *s1, const QString &s2 );
Q_EXPORT bool operator<( const QString &s1, const QString &s2 );
Q_EXPORT bool operator<( const QString &s1, const char *s2 );
Q_EXPORT bool operator<( const char *s1, const QString &s2 );
Q_EXPORT bool operator<=( const QString &s1, const QString &s2 );
Q_EXPORT bool operator<=( const QString &s1, const char *s2 );
Q_EXPORT bool operator<=( const char *s1, const QString &s2 );
Q_EXPORT bool operator>( const QString &s1, const QString &s2 );
Q_EXPORT bool operator>( const QString &s1, const char *s2 );
Q_EXPORT bool operator>( const char *s1, const QString &s2 );
Q_EXPORT bool operator>=( const QString &s1, const QString &s2 );
Q_EXPORT bool operator>=( const QString &s1, const char *s2 );
Q_EXPORT bool operator>=( const char *s1, const QString &s2 );

Q_EXPORT inline QString operator+( const QString &s1, const QString &s2 )
{
    QString tmp( s1 );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QString operator+( const QString &s1, const char *s2 )
{
    QString tmp( s1 );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QString operator+( const char *s1, const QString &s2 )
{
    QString tmp( s1 );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QString operator+( const QString &s1, QChar c2 )
{
    QString tmp( s1 );
    tmp += c2;
    return tmp;
}

Q_EXPORT inline QString operator+( const QString &s1, char c2 )
{
    QString tmp( s1 );
    tmp += c2;
    return tmp;
}

Q_EXPORT inline QString operator+( QChar c1, const QString &s2 )
{
    QString tmp;
    tmp += c1;
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QString operator+( char c1, const QString &s2 )
{
    QString tmp;
    tmp += c1;
    tmp += s2;
    return tmp;
}



#if defined(_OS_WIN32_)
extern Q_EXPORT QString qt_winQString(void*);
extern Q_EXPORT const void* qt_winTchar(const QString& str, bool addnul);
extern Q_EXPORT void* qt_winTchar_new(const QString& str);
#endif

#endif // QSTRING_H

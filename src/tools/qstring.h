/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.h#26 $
**
** Definition of extended char array operations, and QByteArray and
** QString classes
**
** Author  : Haavard Nord
** Created : 920609
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSTRING_H
#define QSTRING_H

#include "qarray.h"
#include <string.h>


// --------------------------------------------------------------------------
// Safe and portable C string functions; extensions to standard string.h
//

void *qmemmove( void *dst, const void *src, uint len );

#if defined(_OS_SUN_) || defined(_CC_OC_)
#define memmove qmemmove
#endif

char *qstrdup( const char * );
#undef	strdup
#define strdup qstrdup

inline uint cstrlen( const char *str )
{ return strlen(str); }

inline uint qstrlen( const char *str )
{ return str ? strlen(str) : 0; }

#undef  strlen
#define strlen qstrlen

inline char *cstrcpy( char *dst, const char *src )
{ return strcpy(dst,src); }

inline char *qstrcpy( char *dst, const char *src )
{ return src ? strcpy(dst, src) : 0; }

#undef  strcpy
#define strcpy qstrcpy

char *qstrncpy( char *dst, const char *src, uint len );

inline int cstrcmp( const char *str1, const char *str2 )
{ return strcmp(str1,str2); }

inline int qstrcmp( const char *str1, const char *str2 )
{ return (str1 && str2) ? strcmp(str1,str2) : (int)((long)str2 - (long)str1); }

#undef  strcmp
#define strcmp qstrcmp

inline int cstrncmp( const char *str1, const char *str2, uint len )
{ return strncmp(str1,str2,len); }

inline int qstrncmp( const char *str1, const char *str2, uint len )
{ return (str1 && str2) ? strncmp(str1,str2,len) :
			  (int)((long)str2 - (long)str1); }

#undef  strncmp
#define strncmp qstrncmp

int qstricmp( const char *, const char * );
int qstrnicmp( const char *, const char *, uint len );

#undef  stricmp
#define stricmp  qstricmp
#undef  strnicmp
#define strnicmp qstrnicmp


// qchecksum: Internet checksum

UINT16 qchecksum( const char *s, uint len );


// --------------------------------------------------------------------------
// QByteArray class
//

declare(QArrayM,char);

#define QByteArray QArrayM(char)		// byte array class


// --------------------------------------------------------------------------
// QByteArray stream functions
//

QDataStream &operator<<( QDataStream &, const QByteArray & );
QDataStream &operator>>( QDataStream &, QByteArray & );


// --------------------------------------------------------------------------
// QString class
//

class QRegExp;

class QString : public QByteArray		// string class
{
public:
    QString() {}				// make null string
    QString( int size );			// allocate size incl. \0
    QString( const QString &s ) : QByteArray( s ) {}
    QString( const char *str );			// deep copy

    QString    &operator=( const QString &s )	// shallow copy
	{ return (QString&)assign( s ); }
    QString    &operator=( const char *str )	// deep copy
	{ return (QString&)duplicate( str, strlen(str)+1 ); }

    bool	isEmpty() const { return QGArray::size() <= 1; }
    uint	length()  const;		// length of QString excl. \0
    bool	resize( uint newlen );		// resize incl. \0 terminator
    bool	truncate( uint pos );		// truncate excl. \0 terminator
    bool	fill( char c, int len = -1 );	// resize and fill string

    QString	copy() const			// deep copy
		  { QString tmp( (const char *)this->data() ); return tmp; }

    QString    &sprintf( const char *format, ... );

    int		find( char c, int index=0, bool cs=TRUE ) const;
    int		find( const char *str, int index=0, bool cs=TRUE ) const;
    int		find( const QRegExp &, int index=0 ) const;
    int		findRev( char c, int index=-1, bool cs=TRUE) const;
    int		findRev( const char *str, int index=-1, bool cs=TRUE) const;
    int		findRev( const QRegExp &, int index=-1 ) const;
    int		contains( char c, bool cs=TRUE ) const;
    int		contains( const char *str, bool cs=TRUE ) const;
    int		contains( const QRegExp & ) const;

    QString	left( uint len )	const;
    QString	right( uint len )	const;
    QString	mid( uint index, uint len) const;

    QString	leftJustify( uint width, char fill=' ', bool trunc=FALSE)const;
    QString	rightJustify( uint width, char fill=' ',bool trunc=FALSE)const;

    QString     lower() const;
    QString     upper()	const;

    QString     stripWhiteSpace()	const;
    QString     simplifyWhiteSpace()	const;

    QString    &insert( uint index, const char * );
    QString    &insert( uint index, char );
    QString    &remove( uint index, uint len );
    QString    &replace( uint index, uint len, const char * );
    QString    &replace( const QRegExp &, const char * );

    short	toShort( bool *ok=0 )	const;
    ushort	toUShort( bool *ok=0 )	const;
    int		toInt( bool *ok=0 )	const;
    uint	toUInt( bool *ok=0 )	const;
    long	toLong( bool *ok=0 )	const;
    ulong	toULong( bool *ok=0 )	const;
    float	toFloat( bool *ok=0 )	const;
    double	toDouble( bool *ok=0 )	const;

    QString    &setStr( const char *s );
    QString    &setNum( short );
    QString    &setNum( ushort );
    QString    &setNum( int );
    QString    &setNum( uint );
    QString    &setNum( long );
    QString    &setNum( ulong );
    QString    &setNum( float, char f='g', int prec=6 );
    QString    &setNum( double, char f='g', int prec=6 );

    bool	setExpand( uint index, char c );

		operator char *() const	      { return data(); }
		operator const char *() const { return (pcchar)data(); }
    bool	operator!() const	      { return isNull(); }
    QString    &operator+=( const char *str );
    QString    &operator+=( char c );

    bool operator==( const QString &s ) const;
    bool operator!=( const QString &s ) const;
    bool operator==( const char *s ) 	const;
    bool operator!=( const char *s ) 	const;
    bool operator==( char *s ) 		const;
    bool operator!=( char *s ) 		const;
    bool operator<( const char *s )  	const;
    bool operator>( const char *s )  	const;
    bool operator<=( const char *s ) 	const;
    bool operator>=( const char *s ) 	const;
};


// --------------------------------------------------------------------------
// QString stream functions
//

QDataStream &operator<<( QDataStream &, const QString & );
QDataStream &operator>>( QDataStream &, QString & );


// --------------------------------------------------------------------------
// QString inline functions
//

inline uint QString::length() const
{ return strlen( data() ); }

inline bool QString::truncate( uint pos )
{ return resize(pos+1); }

inline short QString::toShort( bool *ok ) const
{ return (short)toLong(ok); }

inline ushort QString::toUShort( bool *ok ) const
{ return (ushort)toULong(ok); }

inline int QString::toInt( bool *ok ) const
{ return (int)toLong(ok); }

inline uint QString::toUInt( bool *ok ) const
{ return (uint)toLong(ok); }

inline float QString::toFloat( bool *ok ) const
{ return (float)toDouble(ok); }

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

inline bool QString::operator==( const QString &s ) const
{ return strcmp(data(),s.data()) == 0; }

inline bool QString::operator!=( const QString &s ) const
{ return strcmp(data(),s.data()) != 0; }

inline bool QString::operator==( const char *s ) const
{ return strcmp(data(),s) == 0; }

inline bool QString::operator!=( const char *s ) const
{ return strcmp(data(),s) != 0; }

inline bool QString::operator==( char *s ) const
{ return strcmp(data(),s) == 0; }

inline bool QString::operator!=( char *s ) const
{ return strcmp(data(),s) != 0; }

inline bool QString::operator<( const char *s )  const
{ return strcmp(data(),s) < 0; }

inline bool QString::operator>( const char *s )  const
{ return strcmp(data(),s) > 0; }

inline bool QString::operator<=( const char *s ) const
{ return strcmp(data(),s) <= 0; }

inline bool QString::operator>=( const char *s ) const
{ return strcmp(data(),s) >= 0; }


// --------------------------------------------------------------------------
// QString non-member operators
//

inline QString operator+( const QString &s1, const QString &s2 )
{
    QString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

inline QString operator+( const QString &s1, const char *s2 )
{
    QString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

inline QString operator+( const char *s1, const QString &s2 )
{
    QString tmp( s1 );
    tmp += s2;
    return tmp;
}


#endif // QSTRING_H

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.h#7 $
**
** Definition of extended char array operations, and QByteArray and
** QString classes
**
** Author  : Haavard Nord
** Created : 920609
**
** Copyright (C) 1992-1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSTRING_H
#define QSTRING_H

#include "qarray.h"
#include <string.h>


// --------------------------------------------------------------------------
// Safe and portable C string functions; extensions to standard string.h
//

#if defined(_OS_SUN_) && !defined(_CC_GNU_)
#include <strings.h>
inline int stricmp( const char *s1, const char *s2 )
{ return strcasecmp( (char*)s1, (char*)s2 ); }	// use native strcasecmp
inline int strnicmp( const char *s1, const char *s2, uint sz )
{ return strncasecmp( (char*)s1, (char*)s2, sz ); }

#elif defined(_CC_GNU_)
extern "C" int strcasecmp( const char *, const char * );
extern "C" int strncasecmp( const char *, const char *, uint );
#define stricmp	 strcasecmp			// use native strcasecmp
#define strnicmp strncasecmp

#elif defined(_OS_MAC_) || defined(VXWORKS)
#define stricmp	 qstricmp			// use our own qstricmp
#define strnicmp qstrnicmp			// use our own qstrnicmp

#elif defined(_CC_MSC_)
#define stricmp	 _stricmp
#define strnicmp _strnicmp

#elif defined(_CC_BOR_)
extern "C" int stricmp( const char *, const char * );
extern "C" int strnicmp( const char *, const char *, uint );

#endif


void *qmemmove( void *dst, const void *src, uint len );

#if defined(_OS_SUN_) || defined(_CC_OC_)
#define memmove qmemmove
#endif

char *q_strdup( const char *src );		// safe duplicate string
#define strdup q_strdup

inline int q_strlen( const char *src )		// safe get string length
{
    return src ? strlen( src ) : 0;
}

#define strlen q_strlen

inline char *q_strcpy( char *dest, const char *src )
{						// safe copy string
    return src ? strcpy(dest, src) : 0;
}

#define strcpy q_strcpy

inline void qstrdel( char *&s )			// safe delete and set to null
{
    delete s;
    s = 0;
}

inline char *qstrcpy( char *&dest, const char *src )
{						// delete and copy
    delete dest;
    return dest = strdup( src );
}


// qstrncpy: Same as strncpy, but guarantees that the result is 0-terminated,
//	     possibly by overwriting the last character copied by strncpy.

inline char *qstrncpy( char *dest, const char *src, int len )
{
    strncpy( dest, src, len );
    if (len > 0)
	dest[len-1] = '\0';
    return dest;
}


// qstricmp:  Case-insensitive string comparision
// qstrnicmp: Case-insensitive string comparision, compares max len chars

int qstricmp( const char *s1, const char *s2 );
int qstrnicmp( const char *s1, const char *s2, uint len );


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

class QString : public QByteArray		// string class
{
public:
    QString() {}				// make null string
    QString( int size );			// allocate size incl. \0
    QString( const QString &s ) : QByteArray( s ) {}
    QString( const char *str );			// deep copy

    QString    &operator=( const QString &s )	// shallow copy
	{ return (QString&)assign( s ); }
    QString    &operator=( const char *str );	// deep copy

    bool	isEmpty() const { return QGArray::size() <= 1; }
    uint	length()  const;		// length of QString excl. \0
    bool	resize( uint newlen );		// resize incl. \0 terminator
    bool	fill( char c, int len = -1 );	// resize and fill string

    QString	copy() const			// get deep copy
		  { QString tmp( (const char *)this->data() ); return tmp; }

    QString    &sprintf( const char *format, ... );
    bool	stripWhiteSpace();		// removes white space

    int		find( char c, int index=0, bool cs=TRUE ) const;
    int		find( const char *str, int index=0, bool cs=TRUE ) const;
    int		findRev( char c, int index=-1, bool cs=TRUE) const;
    int		findRev( const char *str, int index=-1, bool cs=TRUE) const;
    int		contains( char c, bool cs=TRUE ) const;
    int		contains( const char *str, bool cs=TRUE ) const;

    QString	left( uint len )	const;	// get left substring
    QString	right( uint len )	const;	// get right substring
    QString	mid( uint index, uint len) const; // get mid substring

    QString	leftJustify( uint width, char fill=' ' )  const;
    QString	rightJustify( uint width, char fill=' ' ) const;

    QString    &lower();
    QString    &upper();

    QString    &insert( uint index, const char * );
    QString    &insert( uint index, char );
    QString    &remove( uint index, uint len );
    QString    &replace( uint index, uint len, const char * );

    short	toShort( bool *ok=0 )	const;	// convert string to short
    ushort	toUShort( bool *ok=0 )	const;	// convert string to ushort
    int		toInt( bool *ok=0 )	const;	// convert string to int
    uint	toUInt( bool *ok=0 )	const;	// convert string to uint
    long	toLong( bool *ok=0 )	const;	// convert string to long
    ulong	toULong( bool *ok=0 )	const;	// convert string to ulong    
    float	toFloat( bool *ok=0 )	const;	// convert string to float
    double	toDouble( bool *ok=0 )	const;	// convert string to double

    QString    &setStr( const char *s );	// copy s, but not deref
    QString    &setNum( short );		// set string from short
    QString    &setNum( ushort );		// set string from ushort
    QString    &setNum( int );			// set string from int
    QString    &setNum( uint );			// set string from uint
    QString    &setNum( long );			// set string from long
    QString    &setNum( ulong );		// set string from ulong
    QString    &setNum( float, char f='g', int prec=-1 );
    QString    &setNum( double, char f='g', int prec=-1 );

    bool	setExpand( uint index, char c );// set and expand if necessary

		operator char *() const	      { return data(); }
		operator const char *() const { return (pcchar)data(); }
    bool	operator !() const	      { return isNull(); }
    QString    &operator+=( const QString &s ); // append s to this string
    QString    &operator+=( const char *str );	// append str to this string
    QString    &operator+=( char c );		// append c to this string
};


// --------------------------------------------------------------------------
// QString inline functions
//

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


// --------------------------------------------------------------------------
// QString non-member operators
//

inline bool operator==( const QString &s1, const QString &s2 )
{ return strcmp(s1,s2) == 0; }

inline bool operator==( const QString &s1, const char* s2 )
{ return strcmp(s1,s2) == 0; }

inline bool operator==( const char* s1, const QString &s2 )
{ return strcmp(s1,s2) == 0; }

inline bool operator!=( const QString &s1, const QString &s2 )
{ return strcmp(s1,s2) != 0; }

inline bool operator!=( const QString &s1, const char* s2 )
{ return strcmp(s1,s2) != 0; }

inline bool operator!=( const char* s1, const QString &s2 )
{ return strcmp(s1,s2) != 0; }

inline bool operator<( const QString &s1, const QString &s2 )
{ return strcmp(s1,s2) < 0; }

inline bool operator<( const QString &s1, const char* s2 )
{ return strcmp(s1,s2) < 0; }

inline bool operator<( const char* s1, const QString &s2 )
{ return strcmp(s1,s2) < 0; }

inline bool operator>( const QString &s1, const QString &s2 )
{ return strcmp(s1,s2) > 0; }

inline bool operator>( const QString &s1, const char* s2 )
{ return strcmp(s1,s2) > 0; }

inline bool operator>( const char* s1, const QString &s2 )
{ return strcmp(s1,s2) > 0; }

inline bool operator<=( const QString &s1, const QString &s2 )
{ return strcmp(s1,s2) <= 0; }

inline bool operator<=( const QString &s1, const char* s2 )
{ return strcmp(s1,s2) <= 0; }

inline bool operator<=( const char* s1, const QString &s2 )
{ return strcmp(s1,s2) <= 0; }

inline bool operator>=( const QString &s1, const QString &s2 )
{ return strcmp(s1,s2) >= 0; }

inline bool operator>=( const QString &s1, const char* s2 )
{ return strcmp(s1,s2) >= 0; }

inline bool operator>=( const char* s1, const QString &s2 )
{ return strcmp(s1,s2) >= 0; }


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

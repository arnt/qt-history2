/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcstring.h#8 $
**
** Definition of the extended char array operations,
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCSTRING_H
#define QCSTRING_H

#ifndef QT_H
#include "qarray.h"
#endif // QT_H

#include <string.h>

#if defined(_OS_SUN_) && defined(_CC_GNU_)
#include <strings.h>
#endif


/*****************************************************************************
  Fixes and workarounds for some platforms
 *****************************************************************************/

#if defined(_OS_HPUX_)
// HP-UX has badly defined strstr() etc.
inline char *hack_strstr( const char *s1, const char *s2 )
{ return (char *)strstr(s1, s2); }
inline char *hack_strchr( const char *s, int c )
{ return (char *)strchr(s, c); }
inline char *hack_strrchr( const char *s, int c )
{ return (char *)strrchr(s, c); }
#define strstr	hack_strstr
#define strchr	hack_strchr
#define strrchr hack_strrchr
#endif


/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

Q_EXPORT void *qmemmove( void *dst, const void *src, uint len );

#if defined(_OS_SUN_) || defined(_CC_OC_)
#define memmove qmemmove
#endif

Q_EXPORT char *qstrdup( const char * );

Q_EXPORT inline uint cstrlen( const char *str )
{ return strlen(str); }

Q_EXPORT inline uint qstrlen( const char *str )
{ return str ? strlen(str) : 0; }

#undef	strlen
#define strlen qstrlen

Q_EXPORT inline char *cstrcpy( char *dst, const char *src )
{ return strcpy(dst,src); }

Q_EXPORT inline char *qstrcpy( char *dst, const char *src )
{ return src ? strcpy(dst, src) : 0; }

#undef	strcpy
#define strcpy qstrcpy

Q_EXPORT char *qstrncpy( char *dst, const char *src, uint len );

Q_EXPORT inline int cstrcmp( const char *str1, const char *str2 )
{ return strcmp(str1,str2); }

Q_EXPORT inline int qstrcmp( const char *str1, const char *str2 )
{ return (str1 && str2) ? strcmp(str1,str2) : (int)((long)str2 - (long)str1); }

#undef	strcmp
#define strcmp qstrcmp

Q_EXPORT inline int cstrncmp( const char *str1, const char *str2, uint len )
{ return strncmp(str1,str2,len); }

Q_EXPORT inline int qstrncmp( const char *str1, const char *str2, uint len )
{ return (str1 && str2) ? strncmp(str1,str2,len) :
			  (int)((long)str2 - (long)str1); }

#undef	strncmp
#define strncmp qstrncmp

Q_EXPORT int qstricmp( const char *, const char * );
Q_EXPORT int qstrnicmp( const char *, const char *, uint len );

#undef	stricmp
#define stricmp	 qstricmp
#undef	strnicmp
#define strnicmp qstrnicmp


// qChecksum: Internet checksum

Q_EXPORT Q_UINT16 qChecksum( const char *s, uint len );

/*****************************************************************************
  QByteArray class
 *****************************************************************************/

#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QArray<char>;
#endif
typedef QArray<char> QByteArray;


/*****************************************************************************
  QByteArray stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QByteArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QByteArray & );



/*****************************************************************************
  QCString class
 *****************************************************************************/

class QRegExp;

class Q_EXPORT QCString : public QByteArray	// C string class
{
public:
    QCString() {}				// make null string
    QCString( int size );			// allocate size incl. \0
    QCString( const QCString &s ) : QByteArray( s ) {}
    QCString( const char *str );		// deep copy
    QCString( const char *str, uint maxlen );	// deep copy, max length

    QCString    &operator=( const QCString &s );// shallow copy
    QCString    &operator=( const char *str );	// deep copy

    bool	isNull()	const;
    bool	isEmpty()	const;
    uint	length()	const;
    bool	resize( uint newlen );
    bool	truncate( uint pos );
    bool	fill( char c, int len = -1 );

    QCString	copy()	const;

    QCString    &sprintf( const char *format, ... );

    int		find( char c, int index=0, bool cs=TRUE ) const;
    int		find( const char *str, int index=0, bool cs=TRUE ) const;
    int		find( const QRegExp &, int index=0 ) const;
    int		findRev( char c, int index=-1, bool cs=TRUE) const;
    int		findRev( const char *str, int index=-1, bool cs=TRUE) const;
    int		findRev( const QRegExp &, int index=-1 ) const;
    int		contains( char c, bool cs=TRUE ) const;
    int		contains( const char *str, bool cs=TRUE ) const;
    int		contains( const QRegExp & ) const;

    QCString	left( uint len )  const;
    QCString	right( uint len ) const;
    QCString	mid( uint index, uint len=0xffffffff) const;

    QCString	leftJustify( uint width, char fill=' ', bool trunc=FALSE)const;
    QCString	rightJustify( uint width, char fill=' ',bool trunc=FALSE)const;

    QCString	lower() const;
    QCString	upper() const;

    QCString	stripWhiteSpace()	const;
    QCString	simplifyWhiteSpace()	const;

    QCString    &insert( uint index, const char * );
    QCString    &insert( uint index, char );
    QCString    &append( const char * );
    QCString    &prepend( const char * );
    QCString    &remove( uint index, uint len );
    QCString    &replace( uint index, uint len, const char * );
    QCString    &replace( const QRegExp &, const char * );

    short	toShort( bool *ok=0 )	const;
    ushort	toUShort( bool *ok=0 )	const;
    int		toInt( bool *ok=0 )	const;
    uint	toUInt( bool *ok=0 )	const;
    long	toLong( bool *ok=0 )	const;
    ulong	toULong( bool *ok=0 )	const;
    float	toFloat( bool *ok=0 )	const;
    double	toDouble( bool *ok=0 )	const;

    QCString    &setStr( const char *s );
    QCString    &setNum( short );
    QCString    &setNum( ushort );
    QCString    &setNum( int );
    QCString    &setNum( uint );
    QCString    &setNum( long );
    QCString    &setNum( ulong );
    QCString    &setNum( float, char f='g', int prec=6 );
    QCString    &setNum( double, char f='g', int prec=6 );

    bool	setExpand( uint index, char c );

		operator const char *() const;
    QCString    &operator+=( const char *str );
    QCString    &operator+=( char c );
};


/*****************************************************************************
  QCString stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QCString & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QCString & );


/*****************************************************************************
  QCString inline functions
 *****************************************************************************/

inline QCString &QCString::operator=( const QCString &s )
{ return (QCString&)assign( s ); }

inline QCString &QCString::operator=( const char *str )
{ return (QCString&)duplicate( str, strlen(str)+1 ); }

inline bool QCString::isNull() const
{ return data() == 0; }

inline bool QCString::isEmpty() const
{ return data() == 0 || *data() == '\0'; }

inline uint QCString::length() const
{ return strlen( data() ); }

inline bool QCString::truncate( uint pos )
{ return resize(pos+1); }

inline QCString QCString::copy() const
{ return QCString( data() ); }

inline QCString &QCString::prepend( const char *s )
{ return insert(0,s); }

inline QCString &QCString::append( const char *s )
{ return operator+=(s); }

inline QCString &QCString::setNum( short n )
{ return setNum((long)n); }

inline QCString &QCString::setNum( ushort n )
{ return setNum((ulong)n); }

inline QCString &QCString::setNum( int n )
{ return setNum((long)n); }

inline QCString &QCString::setNum( uint n )
{ return setNum((ulong)n); }

inline QCString &QCString::setNum( float n, char f, int prec )
{ return setNum((double)n,f,prec); }

inline QCString::operator const char *() const
{ return (const char *)data(); }


/*****************************************************************************
  QCString non-member operators
 *****************************************************************************/

Q_EXPORT inline bool operator==( const QCString &s1, const QCString &s2 )
{ return strcmp(s1.data(),s2.data()) == 0; }

Q_EXPORT inline bool operator==( const QCString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) == 0; }

Q_EXPORT inline bool operator==( const char *s1, const QCString &s2 )
{ return strcmp(s1,s2.data()) == 0; }

Q_EXPORT inline bool operator!=( const QCString &s1, const QCString &s2 )
{ return strcmp(s1.data(),s2.data()) != 0; }

Q_EXPORT inline bool operator!=( const QCString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) != 0; }

Q_EXPORT inline bool operator!=( const char *s1, const QCString &s2 )
{ return strcmp(s1,s2.data()) != 0; }

Q_EXPORT inline bool operator<( const QCString &s1, const QCString& s2 )
{ return strcmp(s1.data(),s2.data()) < 0; }

Q_EXPORT inline bool operator<( const QCString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) < 0; }

Q_EXPORT inline bool operator<( const char *s1, const QCString &s2 )
{ return strcmp(s1,s2.data()) < 0; }

Q_EXPORT inline bool operator<=( const QCString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) <= 0; }

Q_EXPORT inline bool operator<=( const char *s1, const QCString &s2 )
{ return strcmp(s1,s2.data()) <= 0; }

Q_EXPORT inline bool operator>( const QCString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) > 0; }

Q_EXPORT inline bool operator>( const char *s1, const QCString &s2 )
{ return strcmp(s1,s2.data()) > 0; }

Q_EXPORT inline bool operator>=( const QCString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) >= 0; }

Q_EXPORT inline bool operator>=( const char *s1, const QCString &s2 )
{ return strcmp(s1,s2.data()) >= 0; }

Q_EXPORT inline QCString operator+( const QCString &s1, const QCString &s2 )
{
    QCString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QCString operator+( const QCString &s1, const char *s2 )
{
    QCString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QCString operator+( const char *s1, const QCString &s2 )
{
    QCString tmp( s1 );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QCString operator+( const QCString &s1, char c2 )
{
    QCString tmp( s1.data() );
    tmp += c2;
    return tmp;
}

Q_EXPORT inline QCString operator+( char c1, const QCString &s2 )
{
    QCString tmp;
    tmp += c1;
    tmp += s2;
    return tmp;
}

#endif // QCSTRING_H

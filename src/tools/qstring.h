/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.h#4 $
**
** Definition of extended char array operations, and QByteArray and
** QString classes
**
** Author  : Haavard Nord
** Created : 920609
**
** Copyright (C) 1992-1994 by Troll Tech as.  All rights reserved.
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
    int		fill( char c, int len = -1 );	// resize and fill string

    QString copy() const			// get deep copy
		{ QString tmp( (const char *)this->data() ); return tmp; }

    void	sprintf( const char *format, ... );
    bool	stripWhiteSpace();		// removes white space

    int		find( char c, uint index = 0, bool cs=TRUE ) const;
    int		find( const char *str, uint index = 0, bool cs=TRUE ) const;
    int		contains( char c, bool cs=TRUE ) const;
    int		contains( const char *str, bool cs=TRUE ) const;

    bool	setGrow( uint index, char c );	// set and grow if necessary

//		operator char *() const	      { return data(); }
		operator const char *() const { return (pcchar)data(); }
    bool	operator !() const	      { return isNull(); }
    QString    &operator+=( const QString &s ); // append s to this string
    QString    &operator+=( const char *str );	// append str to this string
    QString    &operator+=( char c );		// append c to this string
};


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

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.cpp#3 $
**
** Implementation of extended char array operations, and QByteArray and
** QString classes
**
** Author  : Haavard Nord
** Created : 920722
**
** Copyright (C) 1992-1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#define	 NO_WARNINGS
#define	 QSTRING_C
#include "qstring.h"
#include "qdstream.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qstring.cpp#3 $";
#endif


// --------------------------------------------------------------------------
// Safe and portable C string functions; extensions to standard string.h
//

char *q_strdup( const char *src )		// safe duplicate string
{
    if ( !src )
	return 0;
    char *dst = new char[strlen(src)+1];
    CHECK_PTR( dst );
    return strcpy( dst, src );
}

int qstricmp( const char *str1, const char *str2 )
{						// compare case-insensitive
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( s1 == s2 )				// identical
	return 0;
    for ( ; !(res = (c=tolower(*s1)) - tolower(*s2)); s1++, s2++ )
	if ( !c )				// strings are equal
	    break;
    return res;
}

int qstrnicmp( const char *str1, const char *str2, uint len )
{						// compare case-insensitive/len
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( s1 == s2 )				// identical
	return 0;
    for ( ; len--; s1++, s2++ ) {
	if ( res = (c=tolower(*s1)) - tolower(*s2) )
	    return res;
	if ( !c )				// strings are equal
	    break;
    }
    return 0;
}

UINT16 qchecksum( const char *str, uint len )	// return IP checksum
{
    int a = len/2;
    register short *p = (short *)str;
    register ulong b = 0;
    switch ( a & 15 ) {				// sum words
	while( a>15 ) {				// this is legal ANSI C !!!
	    b += *p++;
	    case 15: b += *p++;
	    case 14: b += *p++;
	    case 13: b += *p++;
	    case 12: b += *p++;
	    case 11: b += *p++;
	    case 10: b += *p++;
	    case  9: b += *p++;
	    case  8: b += *p++;
	    case  7: b += *p++;
	    case  6: b += *p++;
	    case  5: b += *p++;
	    case  4: b += *p++;
	    case  3: b += *p++;
	    case  2: b += *p++;
	    case  1: b += *p++;
	    case  0:
	    a -= 16;
	    b = (b & 0xffff) + (b>>16);
	}
    }
    if ( len & 1 ) {
	str = (const char *)p;
	b += (255 & (*str));
	b = (b & 0xffff) + (b>>16);
    }
    return UINT16(~b);
}


// --------------------------------------------------------------------------
// QByteArray stream functions
//

QDataStream &operator<<( QDataStream &s, const QByteArray &a )
{
    return s.writeBytes( a.data(), a.size() );
}

QDataStream &operator>>( QDataStream &s, QByteArray &a )
{
    UINT32 len;
    s >> len;					// read size of array
    if ( !a.resize( (uint)len ) ) {		// resize array
#if defined(CHECK_NULL)
	warning( "QDataStream: Not enough memory to read QByteArray" );
#endif
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( a.data(), (uint)len );
    return s;
}


// --------------------------------------------------------------------------
// QString member functions
//

QString::QString( int size ) : QByteArray( size )
{						// allocate size incl. \0
    if ( size ) {
	*data() = '\0';				// set terminator
	*(data()+size-1) = '\0';
    }
}

QString::QString( const char *str )		// deep copy
{
    duplicate( str, strlen(str)+1 );
}


QString& QString::operator=( const char *str )	// deep copy
{
    return (QString &)duplicate( str, strlen(str)+1 );
}


uint QString::length() const			// length of string excl. \0
{
    uint len = QByteArray::size();
    return len ? len - 1 : 0;			// subtract for terminating \0
}						// (if not empty)

bool QString::resize( uint len )		// resize incl. \0 terminator
{
    if ( !QByteArray::resize(len+1) )
	return FALSE;
    at(len) = '\0';
    return TRUE;
}


void QString::sprintf( const char *format, ... )// make formatted string
{
    va_list ap;
    va_start( ap, format );
    if ( size() < 255 )
	resize( 255 );				// make string big enough
    vsprintf( data(), format, ap );
    resize( strlen(data()) );			// truncate
    va_end( ap );
}

bool QString::stripWhiteSpace()			// strip white space
{
    if ( isEmpty() )				// nothing to do
	return FALSE;
    register char *s = data();
    if ( !isspace(s[0]) && !isspace(s[length()-1]) )
	return FALSE;
    int start = 0;
    int end = length()-1;
    while ( isspace(s[start]) )			// skip white space from start
	start++;
    if ( s[start] == '\0' ) {			// only white space
	resize( 0 );
	return TRUE;
    }
    while ( end && isspace(s[end]) )		// skip white space from end
	end--;
    end -= start - 1;
#if defined(_OS_SUN_) || defined(_CC_OC_)
    int n = end;				// no ANSI memmove
    register char *p = data();
    s = data() + start;
    while ( n-- )
	*p++ = *s++;
#else
    memmove( data(), &s[start], end );
#endif
    resize( end );
    return TRUE;
}

int QString::fill( char c, int len )		// fill string with c
{
    if ( len < 0 )
	len = length();
    if ( !QByteArray::fill(c,len+1) )
	return FALSE;
    at(len) = '\0';
    return TRUE;
}


int QString::find( char c, uint index, bool cs ) const
{						// find char
    if ( index >= size() ) {
#if defined(CHECK_RANGE)
	warning( "QString::find: Index %d out of range", index );
#endif
	return -1;
    }
    char *d;
    if ( cs )					// case sensitive
	d = strchr( data()+index, c );
    else {
	d = data()+index;
	c = tolower( c );
	while ( *d && tolower(*d) != c )
	    d++;
	if ( !*d )				// not found
	    d = 0;
    }
    return d ? (int)(d - data()) : -1;
}

int QString::find( const char *str, uint index, bool cs ) const
{						// find substring
    if ( index >= size() ) {
#if defined(CHECK_RANGE)
	warning( "QString::find: Index %d out of range", index );
#endif
	return -1;
    }
    char *d;
    if ( cs )					// case sensitive
	d = strstr( data()+index, str );
    else {					// case insensitive
	d = data()+index;
	int len = strlen( str );
	while ( *d ) {
	    if ( tolower(*d) == tolower(*str) && strnicmp( d, str, len ) == 0 )
		 break;
	    d++;
	}
	if ( !*d )				// not found
	    d = 0;
    }
    return d ? (int)(d - data()) : -1;
}

int QString::contains( char c, bool cs ) const	// get # c's
{
    int count = 0;
    char *d = data();
    if ( !d )
	return 0;
    if ( cs ) {					// case sensitive
	while ( *d )
	    if ( *d++ == c )
		count++;
    }
    else {					// case insensitive
	c = tolower( c );
	while ( *d ) {
	    if ( tolower(*d) == c )
		count++;
	    d++;
	}
    }
    return count;
}

int QString::contains( const char *str, bool cs ) const
{						// get # str substrings
    int count = 0;
    char *d = data();
    if ( !d )
	return 0;
    int len = strlen( str );
    while ( *d ) {				// counts overlapping strings
	if ( cs ) {
	    if ( strncmp( d, str, len ) == 0 )
		count++;
	}
	else {
	    if ( tolower(*d) == tolower(*str) && strnicmp( d, str, len ) == 0 )
		count++;
	}
	d++;
    }
    return count;
}


bool QString::setGrow( uint index, char c )	// set and grow if necessary
{
    if ( index >= length() ) {
	uint oldlen = length();
	if ( !QByteArray::resize( index+2 ) )	// no memory
	    return FALSE;
	memset( data() + oldlen, ' ', length() - oldlen );
	*(data() + length()) = '\0';		// terminate padded string
    }
    *(data() + index) = c;
    return TRUE;
}


QString& QString::operator+=( const QString &s )// append QString s to this
{
    uint len2 = s.length();			// length of other string
    if ( !len2 )
	return *this;				// nothing to append

    int len1 = length();			// length of this string
    if ( !QByteArray::resize( len1 + len2 + 1 ) )
	return *this;				// no memory

    memcpy( data() + len1, s.data(), len2 + 1 );

    return *this;
}

QString& QString::operator+=( const char *str ) // append char *str to this
{
    if ( !str )
	return *this;				// nothing to append

    uint len1 = length();
    uint len2 = strlen(str);
    if ( !QByteArray::resize( len1 + len2 + 1 ) )
	return *this;				// no memory

    memcpy( data() + len1, str, len2 + 1 );

    return *this;
}

QString& QString::operator+=( char c )		// append c to this string
{
    uint len = length();
    if ( !QByteArray::resize( len + 2 ) )
	return *this;				// no memory

    *(data() + len) = c;
    *(data() + len+1) = '\0';

    return *this;
}

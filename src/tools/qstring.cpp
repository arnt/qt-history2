/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.cpp#5 $
**
** Implementation of extended char array operations, and QByteArray and
** QString classes
**
** Author  : Haavard Nord
** Created : 920722
**
** Copyright (C) 1992-1994 by Troll Tech AS.  All rights reserved.
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
static char ident[] = "$Id: //depot/qt/main/src/tools/qstring.cpp#5 $";
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


QString &QString::sprintf( const char *format, ... )
{						// make formatted string
    va_list ap;
    va_start( ap, format );
    if ( size() < 255 )
	resize( 255 );				// make string big enough
    vsprintf( data(), format, ap );
    resize( strlen(data()) );			// truncate
    va_end( ap );
    return *this;
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

bool QString::fill( char c, int len )		// fill string with c
{
    if ( len < 0 )
	len = length();
    if ( !QByteArray::fill(c,len+1) )
	return FALSE;
    at(len) = '\0';
    return TRUE;
}


int QString::find( char c, int index, bool cs ) const
{						// find char
    if ( (uint)index >= size() )		// index outside string
	return -1;
    register char *d;
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

int QString::find( const char *str, int index, bool cs ) const
{						// find substring
    if ( (uint)index >= size() )		// index outside string
	return -1;
    register char *d;
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

int QString::findRev( char c, int index, bool cs ) const
{						// reverse find char
    if ( index < 0 ) {				// neg index ==> start from end
	index = length();
	if ( index == 0 )
	    return -1;
    }
    else if ( (uint)index >= size() )		// bad index
	return -1;
    char *b = data();
    register char *d = b+index;
    if ( cs ) {					// case sensitive
	while ( *d != c && d >= b )
	    d--;
    }
    else {
	c = tolower( c );
	while ( tolower(*d) != c && d >= b )
	    d--;
    }
    return d >= b ? (int)(d - b) : -1;
}

int QString::findRev( const char *str, int index, bool cs ) const
{						// reverse find substring
    int slen = strlen(str);
    if ( index < 0 )				// neg index ==> start from end
	index = length()-slen;
    else if ( (uint)index >= size() )		// bad index
	return -1;
    else if ( (uint)(index + slen) > length() )	// str would be too long
	index = length() - slen;
    if ( index < 0 )
	return -1;

    register char *d = data() + index;
    if ( cs ) {					// case sensitive
	for ( int i=index; i>=0; i-- )
	    if ( strncmp(d--,str,slen)==0 )
		return i;
    }
    else {					// case insensitive
	for ( int i=index; i>=0; i-- )
	    if ( strnicmp(d--,str,slen)==0 )
		return i;
    }
    return -1;
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


QString QString::left( uint len ) const		// get left substring
{
    if ( isEmpty() ) {
	QString empty;
	return empty;
    }
    else if ( len >= size() ) {
	QString same( data() );
	return same;
    }
    else {
	QString s( len+1 );
	strncpy( s.data(), data(), len );
	*(s.data()+len) = '\0';
	s.resize( (int)strchr(s.data(),0) - (int)s.data() );
	return s;
    }
}

QString QString::right( uint len ) const	// get right substring
{
    if ( isEmpty() ) {
	QString empty;
	return empty;
    }
    else if ( len >= size() ) {
	QString same( data() );
	return same;
    }
    else {
	register char *p = strchr(data(),0) - len;
	if ( p < data() )
	    p = data();
	QString s( p );
	return s;
    }
}

QString QString::mid( uint index, uint len ) const // get mid substring
{
    uint slen = strlen( data() );
    if ( isEmpty() || index >= slen ) {
	QString empty;
	return empty;
    }
    else {
	register char *p = data()+index;
	QString s( len+1 );
	strncpy( s.data(), p, len );
	*(s.data()+len) = '\0';
	s.resize( (int)strchr(s.data(),0) - (int)s.data() );
	return s;
    }
}

QString QString::leftJustify( uint width, char fill ) const
{
    QString tmp;
    int len = strlen(data());
    int padlen = width - len;
    if ( padlen > 0 ) {
	tmp.resize( len+padlen+1 );
	memcpy( tmp.data(), data(), len );
	memset( tmp.data()+len, fill, padlen );
	tmp[len+padlen] = '\0';
    }
    else
	tmp = copy();
    return tmp;
}

QString QString::rightJustify( uint width, char fill ) const
{
    QString tmp;
    int len = strlen(data());
    int padlen = width - len;
    if ( padlen > 0 ) {
	tmp.resize( len+padlen+1 );
	memset( tmp.data(), fill, padlen );
	memcpy( tmp.data()+padlen, data(), len );
	tmp[len+padlen] = '\0';
    }
    else
	tmp = copy();
    return tmp;
}


QString &QString::lower()			// convert to lower case
{
    if ( !isEmpty() ) {
	register char *p = data();
	while ( *p ) {
	    *p = tolower(*p);
	    p++;
	}
    }
    return *this;
}

QString &QString::upper()			// convert to upper case
{
    if ( !isEmpty() ) {
	register char *p = data();
	while ( *p ) {
	    *p = toupper(*p);
	    p++;
	}
    }
    return *this;
}


QString &QString::insert( uint index, const char *s )
{						// insert s into string
    int len = strlen(s);
    if ( len == 0 )
	return *this;
    uint olen = length();
    int nlen = olen + len;
    if ( index >= olen ) {			// insert after end of string
	if ( QByteArray::resize(nlen+index-olen+1) ) {
	    memset( data()+olen, ' ', index-olen );
	    memcpy( data()+index, s, len+1 );
	}
    }
    else if ( QByteArray::resize(nlen+1) ) {	// normal insert
#if defined(_OS_SUN_) || defined(_CC_OC_)
	register char *sd = data()+length();	// no ANSI memmove
    	register char *ss = sd - len;
	int cnt = olen - index + 1;
	while ( cnt-- )
	    *sd-- = *ss--;
#else
	memmove( data()+index+len, data()+index, olen-index+1 );
#endif
	memcpy( data()+index, s, len );
    }
    return *this;
}

QString &QString::insert( uint index, char c )	// insert char
{
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return insert( index, buf );
}

QString &QString::remove( uint index, uint len )// remove part of string
{
    uint olen = length();
    if ( index + len > olen ) {			// range problems
	if ( index >= olen )			// index outside string
	    return *this;
	len = olen - index;			// adjust len
    }
#if defined(_OS_SUN_) || defined(_CC_OC_)
    register char *sd = data()+index;		// no ANSI memmove
    register char *ss = sd + len;
    int cnt = olen - index + 1;
    while ( cnt-- )
	*sd++ = *ss++;
#else
    memmove( data()+index, data()+index+len, olen-index+1 );
#endif
    QByteArray::resize(size()-len);
    return *this;
}

QString &QString::replace( uint index, uint len, const char *s )
{						// replace part of string
    remove( index, len );
    insert( index, s );
    return *this;
}


long QString::toLong( bool *ok ) const		// convert string to long
{
    char *end;
    long val = strtol( data(), &end, 0 );
    if ( ok ) {
	if ( end == 0 || *end == '\0' )
	    *ok = TRUE;
	else
	    *ok = FALSE;
    }
    return val;
}

ulong QString::toULong( bool *ok ) const	// convert string to ulong
{
    char *end;
    ulong val = strtoul( data(), &end, 0 );
    if ( ok ) {
	if ( end == 0 || *end == '\0' )
	    *ok = TRUE;
	else
	    *ok = FALSE;
    }
    return val;
}

double QString::toDouble( bool *ok ) const	// convert string to double
{
    char *end;
    double val = strtod( data(), &end );
    if ( ok ) {
	if ( end == 0 || *end == '\0' )
	    *ok = TRUE;
	else
	    *ok = FALSE;
    }
    return val;
}


QString &QString::setStr( const char *s )	// copy string, but not deref
{
    if ( s )					// valid string
	store( s, strlen(s)+1 );
    else					// empty
	resize( 0 );
    return *this;
}

QString &QString::setNum( long n )		// set string from long
{
    char buf[20];
    register char *p = &buf[19];
    bool neg;
    if ( n < 0 ) {
	neg = TRUE;
	n = -n;
    }
    else
	neg = FALSE;
    *p = '\0';
    do {
	*--p = ((int)(n%10)) + '0';
	n /= 10;
    } while ( n );
    if ( neg )
	*--p = '-';
    store( p, strlen(p)+1 );
    return *this;
}

QString &QString::setNum( ulong n )		// set string from ulong
{
    char buf[20];
    register char *p = &buf[19];
    *p = '\0';
    do {
	*--p = ((int)(n%10)) + '0';
	n /= 10;
    } while ( n );
    store( p, strlen(p)+1 );
    return *this;
}

QString &QString::setNum( double n, char f, int prec )
{
#if defined(CHECK_RANGE)
    if ( !(f=='f' || f=='F' || f=='e' || f=='E' || f=='g' || f=='G') )
	warning( "QString::setNum: Invalid format char '%c'", f );
#endif
    char format[20];
    register char *fs = format;			// generate format string
    *fs++ = '%';				//   "%.<prec>l<f>"
    if ( prec < 0 )
	prec = 6;
    else
    if ( prec > 99 )
	prec = 99;
    *fs++ = '.';
    if ( prec >= 10 ) {
	*fs++ = prec / 10 + '0';
	*fs++ = prec % 10 + '0';
    }
    else
	*fs++ = prec + '0';
    *fs++ = 'l';
    *fs++ = f;
    *fs = '\0';
    return sprintf( format, n );
}


bool QString::setExpand( uint index, char c )	// set and expand if necessary
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

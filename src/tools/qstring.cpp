/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.cpp#45 $
**
** Implementation of extended char array operations, and QByteArray and
** QString classes
**
** Author  : Haavard Nord
** Created : 920722
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 QSTRING_C
#include "qstring.h"
#include "qdstream.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qstring.cpp#45 $";
#endif


/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \fn void *memmove( void *dst, const void *src, uint len )
  \relates QString

  This function is normally part of the C library. Qt implements
  memmove() for platforms that do not have it.

  memmove() copies \e len bytes from \e src into \e dst.  The data is
  copied correctly even if \e src and \e dst overlap.
 ----------------------------------------------------------------------------*/

void *qmemmove( void *dst, const void *src, uint len )
{
    register char *d;
    register char *s;
    if ( dst > src ) {
	d = (char *)dst + len - 1;
	s = (char *)src + len - 1;
	while ( len-- )
	    *d-- = *s--;
    }
    else if ( dst < src ) {
	d = (char *)dst;
	s = (char *)src;
	while ( len-- )
	    *d++ = *s++;
    }
    return dst;
}

/*----------------------------------------------------------------------------
  \relates QString

  Returns a duplicate string.

  Allocates space for a copy of \e str (using \c new), copies it, and returns
  a pointer to the copy.
  If \e src is null, it immediately returns 0.
 ----------------------------------------------------------------------------*/

char *qstrdup( const char *str )
{
    if ( !str )
	return 0;
    char *dst = new char[strlen(str)+1];
    CHECK_PTR( dst );
    return cstrcpy( dst, str );
}

/*----------------------------------------------------------------------------
  \fn uint strlen( const char *str )
  \relates QString

  A safe strlen() function that overrides the one defined by the C library.
  The original strlen() function has been renamed cstrlen().

  Returns the number of characters in \e str, or 0 if \e str is null.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn char *strcpy( char *dst, const char *str )
  \relates QString

  A safe strcpy() function that overrides the one defined by the C library.
  The original strcpy() function has been renamed cstrcpy().

  Copies all characters (including \0) from \e str into \e dst and returns
  a pointer to \e dst.
  If \e src is null, it immediately returns 0.

  \sa qstrncpy()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \relates QString

  A safe strncpy() function.

  Copies all characters up to \e len bytes from \e str into \e dst and returns
  a pointer to \e dst.  Guarantees that \e dst is \0-terminated.
  If \e src is null, it immediately returns 0.

  \sa strcpy()
 ----------------------------------------------------------------------------*/

char *qstrncpy( char *dst, const char *src, uint len )
{
    if ( !src )
	return 0;
    strncpy( dst, src, len );
    if ( len > 0 )
	dst[len-1] = '\0';
    return dst;
}

/*----------------------------------------------------------------------------
  \fn int strcmp( const char *str1, const char *str2 )
  \relates QString

  A safe strcmp() function that overrides the one defined by the C library.
  The original strcmp() function has been renamed cstrcmp().

  Compares \e str1 and \e str2.  Returns a negative value if \e str1
  is less than \e str2, 0 if \e str1 is equal to \e str2 or a positive
  value if \e str1 is greater than \e str2.

  Special case I: <br>
  Returns 0 if \e str1 and \e str2 are both null.

  Special case II: <br>
  Returns a nonzero value if \e str1 is null or \e str2 is null (but not both).

  \sa strncmp(), stricmp(), strnicmp()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int strncmp( const char *str1, const char *str2, uint len )
  \relates QString

  A safe strncmp() function that overrides the one defined by the C library.
  The original strncmp() function has been renamed cstrncmp().

  Compares \e str1 and \e str2 up to \e len bytes.

  Returns a negative value if \e str1 is less than \e str2, 0 if \e str1
  is equal to \e str2 or a positive value if \e str1 is greater than \e
  str2.

  Special case I: <br>
  Returns 0 if \e str1 and \e str2 are both null.

  Special case II: <br>
  Returns a nonzero value if \e str1 is null or \e str2 is null (but not both).

  \sa strcmp(), stricmp(), strnicmp()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int stricmp( const char *str1, const char *str2 )
  \relates QString

  A safe stricmp() function that overrides the one defined by the C library,
  if the C library has one.

  Compares \e str1 and \e str2 ignoring the case.

  Returns a negative value if \e str1 is less than \e str2, 0 if \e str1
  is equal to \e str2 or a positive value if \e str1 is greater than \e
  str2.

  Special case I: <br>
  Returns 0 if \e str1 and \e str2 are both null.

  Special case II: <br>
  Returns a nonzero value if \e str1 is null or \e str2 is null (but not both).

  \sa strcmp(), strncmp(), strnicmp()
 ----------------------------------------------------------------------------*/

int qstricmp( const char *str1, const char *str2 )
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return s1 == s1 ? 0 : (int)((long)s2 - (long)s1);
    for ( ; !(res = (c=tolower(*s1)) - tolower(*s2)); s1++, s2++ )
	if ( !c )				// strings are equal
	    break;
    return res;
}

/*----------------------------------------------------------------------------
  \fn int strnicmp( const char *str1, const char *str2, uint len )
  \relates QString

  A safe strnicmp() function that overrides the one defined by the C library,
  if the C library has one.

  Compares \e str1 and \e str2 up to \e len bytes ignoring the case.

  Returns a negative value if \e str1 is less than \e str2, 0 if \e str1
  is equal to \e str2 or a positive value if \e str1 is greater than \e
  str2.

  Special case I: <br>
  Returns 0 if \e str1 and \e str2 are both null.

  Special case II: <br>
  Returns a nonzero value if \e str1 is null or \e str2 is null (but not both).

  \sa strcmp(), strncmp() stricmp()
 ----------------------------------------------------------------------------*/

int qstrnicmp( const char *str1, const char *str2, uint len )
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return s1 == s1 ? 0 : (int)((long)s2 - (long)s1);
    for ( ; len--; s1++, s2++ ) {
	if ( (res = (c=tolower(*s1)) - tolower(*s2)) )
	    return res;
	if ( !c )				// strings are equal
	    break;
    }
    return 0;
}


static UINT16 crc_tbl[16];
static bool   crc_tbl_init = FALSE;

static void createCRC16Table()			// build CRC16 lookup table
{
    register int i;
    register int j;
    int v0, v1, v2, v3;
    for ( i=0; i<16; i++ ) {
	v0 = i & 1;
	v1 = (i >> 1) & 1;
	v2 = (i >> 2) & 1;
	v3 = (i >> 3) & 1;
	j = 0;
#undef	SET_BIT
#define SET_BIT(x,b,v)	x |= v << b
	SET_BIT(j, 0,v0);
	SET_BIT(j, 7,v0);
	SET_BIT(j,12,v0);
	SET_BIT(j, 1,v1);
	SET_BIT(j, 8,v1);
	SET_BIT(j,13,v1);
	SET_BIT(j, 2,v2);
	SET_BIT(j, 9,v2);
	SET_BIT(j,14,v2);
	SET_BIT(j, 3,v3);
	SET_BIT(j,10,v3);
	SET_BIT(j,15,v3);
	crc_tbl[i] = j;
    }
}

/*----------------------------------------------------------------------------
  \relates QByteArray
  Returns the CRC-16 checksum of \e len bytes starting at \e data.

  The checksum is independent of the byte order (endianness).
 ----------------------------------------------------------------------------*/

UINT16 qchecksum( const char *data, uint len )	// generate CRC-16 checksum
{
    if ( !crc_tbl_init ) {			// create lookup table
	createCRC16Table();
	crc_tbl_init = TRUE;
    }
    register UINT16 crc = 0xffff;
    register int index;
    uchar c;
    uchar *p = (uchar *)data;
    while ( len-- ) {
	c = *p++;
	index = ((crc ^ c) & 15);
	crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[index];
	c >>= 4;
	index = ((crc ^ c) & 15);
	crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[index];
    }
    return ~crc;
}


/*****************************************************************************
  QByteArray member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \class QByteArray qstring.h
  \ingroup tools
  \brief The QByteArray class provides an array of bytes.

  This class will be documented later.
 ----------------------------------------------------------------------------*/

/*****************************************************************************
  QByteArray stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QByteArray
  Writes a byte array to a stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QByteArray &a )
{
    return s.writeBytes( a.data(), a.size() );
}

/*----------------------------------------------------------------------------
  \relates QByteArray
  Reads a byte array from a stream.
 ----------------------------------------------------------------------------*/

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


/*****************************************************************************
  QString member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \class QString qstring.h

  \brief The QString class is provides an abstraction of the classic C
  zero-terminated char array.

  \ingroup tools

  QString inherits QByteArray, which is defined as QArray\<char\>.

  Note that for the QString methods that take a <var>const char *</var>
  parameter the results are undefined if the QString is not
  zero-terminated.  It is legal for the <var>const char *</var> parameter
  to be 0.

  A QString that has not been assigned to anything is \e null, i.e. both
  the length and data pointer is 0. A QString that references the empty
  string ("", a single '\0' char) is \e empty.  Both void and empty
  QStrings are legal parameters to the methods. Assigning <var>const char
  * 0</var> to QString gives a null QString.

  Since QString has both an internal length specifier (from QArray) and a
  zero-terminator, there will be a conflict if somebody manually inserts a
  '\0' into the string.

  \code
    QString s = "Networking is fun";
    s[10] = 0;
    strcmp( s, "Networking" ) == 0;	// TRUE
    strlen( s ) == 10;			// TRUE
    s.length() == 17;			// Also TRUE
  \endcode

  See <a href=handleclasses.html>handle classes</a> for information
  about handle classes.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn QString::QString()
  Constructs a null string.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QString::QString( const QString &s )
  Constructs a shallow copy \e s.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Constructs a string with room for \e size characters, including
  the '\0'-terminator.
 ----------------------------------------------------------------------------*/

QString::QString( int size ) : QByteArray( size )
{						// allocate size incl. \0
    if ( size ) {
	*data() = '\0';				// set terminator
	*(data()+size-1) = '\0';
    }
}

/*----------------------------------------------------------------------------
  Constructs a string that is a deep copy of \e str.
 ----------------------------------------------------------------------------*/

QString::QString( const char *str )		// deep copy
{
    duplicate( str, strlen(str)+1 );
}


/*----------------------------------------------------------------------------
  \fn QString &QString::operator=( const QString &s )
  Assigns a shallow copy of \e s and return a reference to this
  string.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QString &QString::operator=( const char *str )
  Assigns a deep copy of \e str and return a reference to the string.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::isEmpty() const
  Returns TRUE is the string is empty (i.e. if length() == 0).
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn uint QString::length() const
  Returns the length of the string, excluding the '\0'-terminator.
  Null strings (null pointers) and \e empty strings ("") have zero length.
  \sa size()
 ----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  \fn bool QString::truncate( uint pos )
  Truncates the string at position \e pos.

  Equivalent to calling resize(pos+1).

  Example:
  \code
    QString s = "truncate this string";
    s.truncate( 5 );			// s == "trunc"
  \endcode

  \sa resize()
 ----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  Extends or shrinks the string to \e len bytes, including the
  '\0'-terminator.  A \0-terminator is set at position <var>len - 1</var>
  unless <var>len == 0</var>.

  Example:
  \code
    QString s = "resize this string";
    s.resize( 7 );			// s == "resize"
  \endcode

  \sa truncate()
 ----------------------------------------------------------------------------*/

bool QString::resize( uint len )
{
    if ( !QByteArray::resize(len) )
	return FALSE;
    if ( len )
	at(len-1) = '\0';
    return TRUE;
}


/*----------------------------------------------------------------------------
  Implemented as a call to the native vsprintf() (see your C-library
  manual).

  Many vsprintf() implementations have some sort of arbitrary and
  undocumented limit, some crash your program when you exceed it.  If
  your string is shorter than 256 characters, Qt sprintf() calls
  resize(256) to decrease the chance of crashing.

  Example of use:
  \code
  QString s;
  s.sprintf( "%d - %s", 1, "first" );		// result < 256 chars

  QString big( 25000 );				// very long string
  big.sprintf( "%d - %s", 2, veryLongString );	// result < 25000 chars
  \endcode
 ----------------------------------------------------------------------------*/

QString &QString::sprintf( const char *format, ... )
{
    va_list ap;
    va_start( ap, format );
    if ( size() < 256 )
	QByteArray::resize( 256 );		// make string big enough
    vsprintf( data(), format, ap );
    resize( strlen(data()) + 1 );		// truncate
    va_end( ap );
    return *this;
}


/*----------------------------------------------------------------------------
  Fills the string with \e len bytes of value \e c, followed by a
  '\0'-terminator.

  If \e len is negative, then the current string length will be used.

  Returns FALSE is \e len is nonnegative and there is no memory to
  resize the string, otherwise TRUE is returned.
 ----------------------------------------------------------------------------*/

bool QString::fill( char c, int len )		// fill string with c
{
    if ( len < 0 )
	len = length();
    if ( !QByteArray::fill(c,len+1) )
	return FALSE;
    at(len) = '\0';
    return TRUE;
}


/*----------------------------------------------------------------------------
  Finds the first occurrence of the character \e c, starting at
  position \e index.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e c, or -1 if \e c could not be found.
 ----------------------------------------------------------------------------*/

int QString::find( char c, int index, bool cs ) const
{
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

/*----------------------------------------------------------------------------
  Finds the first occurrence of the string \e str, starting at position
  \e index.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e str, or -1 if \e str could not be found.
 ----------------------------------------------------------------------------*/

int QString::find( const char *str, int index, bool cs ) const
{
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

/*----------------------------------------------------------------------------
  Finds the first occurrence of the character \e c, starting at
  position \e index and searching backwards.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e c, or -1 if \e c could not be found.
 ----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------
  Finds the first occurrence of the string \e str, starting at
  position \e index and searching backwards.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e str, or -1 if \e str could not be found.
 ----------------------------------------------------------------------------*/

int QString::findRev( const char *str, int index, bool cs ) const
{
    int slen = strlen(str);
    if ( index < 0 )				// neg index ==> start from end
	index = length()-slen;
    else if ( (uint)index >= size() )		// bad index
	return -1;
    else if ( (uint)(index + slen) > length() ) // str would be too long
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


/*----------------------------------------------------------------------------
  Returns the number of times the character \e c occurs in the string.

  The match is case sensitive if \e cs is TRUE, or case insensitive if \e cs
  if FALSE.
 ----------------------------------------------------------------------------*/

int QString::contains( char c, bool cs ) const
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

/*----------------------------------------------------------------------------
  Returns the number of times \e str occurs in the string.

  The match is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs if FALSE.

  This function counts overlapping substrings, for example, "banana"
  contains two occurrences of "ana".

  \sa findRev()
 ----------------------------------------------------------------------------*/

int QString::contains( const char *str, bool cs ) const
{
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

/*----------------------------------------------------------------------------
  Returns a substring that contains the \e len leftmost characters
  of the string.

  The whole string will be returned if \e len exceeds the length of the string.

  Example:
  \code
    QString s = "Pineapple";
    QString t = s.left( 4 );		// t == "Pine"
  \endcode

  \sa right(), mid()
 ----------------------------------------------------------------------------*/

QString QString::left( uint len ) const
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
	return s;
    }
}

/*----------------------------------------------------------------------------
  Returns a substring that contains the \e len rightmost characters
  of the string.

  The whole string will be returned if \e len exceeds the length of the string.

  Example:
  \code
    QString s = "Pineapple";
    QString t = s.right( 5 );		// t == "apple"
  \endcode

  \sa left(), mid()
 ----------------------------------------------------------------------------*/

QString QString::right( uint len ) const
{
    if ( isEmpty() ) {
	QString empty;
	return empty;
    }
    else {
	char *p = data() + (length() - len);
	if ( p < data() )
	    p = data();
	return QString( p );
    }
}

/*----------------------------------------------------------------------------
  Returns a substring that contains the \e len characters of this
  string, starting at position \e index.

  Returns a null string if the string is empty or \e index is out
  of range.  Returns the whole string from \e index if \e index+len exceeds
  the length of the string.

  Example:
  \code
    QString s = "Two pineapples";
    QString t = s.mid( 4, 4 );		// t == "pine"
  \endcode

  \sa left(), right()
 ----------------------------------------------------------------------------*/

QString QString::mid( uint index, uint len ) const
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
	return s;
    }
}

/*----------------------------------------------------------------------------
  Returns a string of length \e width (plus '\0') that contains this
  string and padded by the \e fill character.

  If the length of the string exceeds \e width and \e truncate is FALSE,
  then the returned string will be a copy of the string.
  If the length of the string exceeds \e width and \e truncate is TRUE,
  then the returned string will be a left(\e width).

  Example:
  \code
    QString s("apple");
    QString t = s.leftJustify(8, '.');	// t == "apple..."
  \endcode

  \sa rightJustify()
 ----------------------------------------------------------------------------*/

QString QString::leftJustify( uint width, char fill, bool truncate ) const
{
    QString result;
    int len = strlen(data());
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.QByteArray::resize( len+padlen+1 );
	memcpy( result.data(), data(), len );
	memset( result.data()+len, fill, padlen );
	result[len+padlen] = '\0';
    }
    else {
	if ( truncate )
	    result = left( width );
	else
	    result = copy();
    }
    return result;
}

/*----------------------------------------------------------------------------
  Returns a string of length \e width (plus '\0') that contains pad
  characters followed by the string.

  If the length of the string exceeds \e width and \e truncate is FALSE,
  then the returned string will be a copy of the string.
  If the length of the string exceeds \e width and \e truncate is TRUE,
  then the returned string will be a right(\e width).

  Example:
  \code
    QString s("pie");
    QString t = s.rightJustify(8, '.');		// t == ".....pie"
  \endcode

  \sa leftJustify()
 ----------------------------------------------------------------------------*/

QString QString::rightJustify( uint width, char fill, bool truncate ) const
{
    QString result;
    int len = strlen(data());
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.QByteArray::resize( len+padlen+1 );
	memset( result.data(), fill, padlen );
	memcpy( result.data()+padlen, data(), len );
	result[len+padlen] = '\0';
    }
    else {
	if ( truncate )
	    result = left( width );
	else
	    result = copy();
    }
    return result;
}

/*----------------------------------------------------------------------------
  Returns a new string that is the string converted to lower case.

  Presently it only handles 7-bit ASCII, or whatever tolower()
  handles (if $LC_CTYPE is set, most UNIX systems do the Right Thing).

  Example:
  \code
    QString s("TeX");
    QString t = s.lower();		// t == "tex"
  \endcode

  \sa upper()
 ----------------------------------------------------------------------------*/

QString QString::lower() const
{
    QString s( data() );
    register char *p = s.data();
    if ( p ) {
	while ( *p ) {
	    *p = tolower(*p);
	    p++;
	}
    }
    return s;
}

/*----------------------------------------------------------------------------
  Returns a new string that is the string converted to upper case.

  Presently it only handles 7-bit ASCII, or whatever toupper()
  handles (if $LC_CTYPE is set, most UNIX systems do the Right Thing).

  Example:
  \code
    QString s("TeX");
    QString t = s.upper();		// t == "TEX"
  \endcode

  \sa lower()
 ----------------------------------------------------------------------------*/

QString QString::upper() const
{
    QString s( data() );
    register char *p = s.data();
    if ( p ) {
	while ( *p ) {
	    *p = toupper(*p);
	    p++;
	}
    }
    return s;
}


/*----------------------------------------------------------------------------
  Returns a new string that has white space removed from the start and the end.

  White space means any ASCII code 9, 10, 11, 12, 13 or 32.

  Example:
  \code
    QString s = " space ";
    QString t = s.stripWhiteSpace();	// t == "space"
  \endcode

  \sa simplifyWhiteSpace()
 ----------------------------------------------------------------------------*/

QString QString::stripWhiteSpace() const
{
    if ( isEmpty() )				// nothing to do
	return copy();

    register char *s = data();
    QString result = s;
    if ( !isspace(s[0]) && !isspace(s[length()-1]) )
	return result;				// returns a copy

    s = result.data();
    int start = 0;
    int end = result.length() - 1;
    while ( isspace(s[start]) )			// skip white space from start
	start++;
    if ( s[start] == '\0' ) {			// only white space
	result.resize( 1 );
	return result;
    }
    while ( end && isspace(s[end]) )		// skip white space from end
	end--;
    end -= start - 1;
    memmove( result.data(), &s[start], end );
    result.resize( end + 1 );
    return result;
}


/*----------------------------------------------------------------------------
  Returns a new string that has white space removed from the start and the end,
  plus any sequence of internal white space replaced with a single space
  (ASCII 32).

  White space means any ASCII code 9, 10, 11, 12, 13 or 32.

  \code
    QString s = "  lots\t of\nwhite    space ";
    QString t = s.simplifyWhiteSpace();	// t == "lots of white space"
  \endcode

  \sa stripWhiteSpace()
 ----------------------------------------------------------------------------*/

QString QString::simplifyWhiteSpace() const
{
    if ( isEmpty() )				// nothing to do
	return copy();

    char *from;
    char *to;
    bool finalspace;

    QString result( data() );
    from = to = result.data();
    finalspace = FALSE;

    while ( *from ) {
	while (*from && isspace(*from))
	    from++;
	if ( !*from )
	    break;
	while (*from && !isspace(*from))
	    *to++ = *from++;
	*to++ = ' ';
	finalspace = TRUE;
    }
    if (finalspace)
	to--;

    *to = '\0';
    result.resize( (int)((long)to + 1 - (long)result.data()) );
    return result;
}


/*----------------------------------------------------------------------------
  Insert \e s into the string at position \e index.  If \e index is
  too large, \e s is inserted at the end of the string.

  \code
    QString s = "I like fish";
    s.insert( 2, "don't ");		// s == "I don't like fish"
  \endcode
 ----------------------------------------------------------------------------*/

QString &QString::insert( uint index, const char *s )
{
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
	memmove( data()+index+len, data()+index, olen-index+1 );
	memcpy( data()+index, s, len );
    }
    return *this;
}

/*----------------------------------------------------------------------------
  Insert \e c into the string at (before) position \e index and returns
  a reference to the string.

  If \e index is too large, \e c is inserted at the end of the string.

  Example:
  \code
    QString s = "Yes";
    s.insert( 12528, '!');	// s == "Yes!"
  \endcode

  \sa remove(), replace()
 ----------------------------------------------------------------------------*/

QString &QString::insert( uint index, char c )	// insert char
{
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return insert( index, buf );
}

/*----------------------------------------------------------------------------
  Removes \e len characters starting at position \e index from the
  string and returns a reference to the string.

  If \e index is too big, nothing happens.  If \e index is valid, but
  \e len is too large, the rest of the string is removed.

  \code
    QString s = "It's a black rug";
    s.remove( 8, 6 );			// s == "It's a bug"
  \endcode

  \sa insert(), replace()
 ----------------------------------------------------------------------------*/

QString &QString::remove( uint index, uint len )
{
    uint olen = length();
    if ( index + len > olen ) {			// range problems
	if ( index >= olen )			// index outside string
	    return *this;
	len = olen - index;			// adjust len
    }
    memmove( data()+index, data()+index+len, olen-index+1 );
    QByteArray::resize(size()-len);
    return *this;
}

/*----------------------------------------------------------------------------
  Replaces \e len characters starting at position \e index from the
  string with \e s, and returns a reference to the string.

  If \e index is too big, nothing is deleted and \e s is inserted at the
  end of the string.  If \e index is valid, but \e len is too large, \e
  str replaces the rest of the string.

  \code
    QString s = "Say yes!";
    s.replace( 4, 3, "NO" );		// s == "Say NO!"
  \endcode

  \sa insert(), remove()
 ----------------------------------------------------------------------------*/

QString &QString::replace( uint index, uint len, const char *s )
{
    remove( index, len );
    insert( index, s );
    return *this;
}


/*----------------------------------------------------------------------------
  Returns the string converted to a <code>long</code> value.

  If \e ok is non-null, \e *ok will be set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
 ----------------------------------------------------------------------------*/

long QString::toLong( bool *ok ) const
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

/*----------------------------------------------------------------------------
  Returns the string converted to an <code>unsigned long</code>
  value.

  If \e ok is non-null, \e *ok will be set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
 ----------------------------------------------------------------------------*/

ulong QString::toULong( bool *ok ) const
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

/*----------------------------------------------------------------------------
  \fn short QString::toShort( bool *ok ) const
  Returns the string converted to a <code>short</code> value.

  If \e ok is non-null, \e *ok will be set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn ushort QString::toUShort( bool *ok ) const
  Returns the string converted to an <code>unsigned short</code>
  value.

  If \e ok is non-null, \e *ok will be set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QString::toInt( bool *ok ) const
  Returns the string converted to a <code>int</code> value.

  If \e ok is non-null, \e *ok will be set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn uint QString::toUInt( bool *ok ) const
  Returns the string converted to an <code>unsigned int</code> value.

  If \e ok is non-null, \e *ok will be set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns the string converted to a <code>double</code> value.

If \e ok is non-null, \e *ok will be set to TRUE if there are no
conceivable errors, and FALSE if the string is not a number at all, or
if it has trailing garbage.
 ----------------------------------------------------------------------------*/

double QString::toDouble( bool *ok ) const
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

/*----------------------------------------------------------------------------
  \fn float QString::toFloat( bool *ok ) const
  Returns the string converted to a <code>float</code> value.

  If \e ok is non-null, \e *ok will be set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Makes a deep copy of \e str without dereferencing the current
  string, i.e. all strings that share data will be modified.
  Returns a reference to the string.
 ----------------------------------------------------------------------------*/

QString &QString::setStr( const char *str )
{
    if ( str )					// valid string
	store( str, strlen(str)+1 );
    else					// empty
	resize( 0 );
    return *this;
}

/*----------------------------------------------------------------------------
  Sets the string to the numerical value of \e n and returns a
  reference to the string.
 ----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------
  Sets the string to the numerical unsigned value of \e n and
  returns a reference to the string.
 ----------------------------------------------------------------------------*/

QString &QString::setNum( ulong n )
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

/*----------------------------------------------------------------------------
  \fn QString &QString::setNum( int n )
  Sets the string to the numerical value of \e n and returns a reference
  to the string.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QString &QString::setNum( uint n )
  Sets the string to the numerical unsigned value of \e n and returns a
  reference to the string.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QString &QString::setNum( short n )
  Sets the string to the numerical value of \e n and returns a reference
  to the string.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QString &QString::setNum( ushort n )
  Sets the string to the numerical unsigned value of \e n and returns a
  reference to the string.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the string to the numerical value of \e n.
  \arg \e f is format specifier: 'f', 'F', 'e', 'E', 'g', 'G' (same
  meaning as for sprintf()).
  \arg \e prec is the precision.

  Returns a reference to the string.
 ----------------------------------------------------------------------------*/

QString &QString::setNum( double n, char f, int prec )
{
#if defined(CHECK_RANGE)
    if ( !(f=='f' || f=='F' || f=='e' || f=='E' || f=='g' || f=='G') )
	warning( "QString::setNum: Invalid format char '%c'", f );
#endif
    char format[20];
    register char *fs = format;			// generate format string
    *fs++ = '%';				//   "%.<prec>l<f>"
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

/*----------------------------------------------------------------------------
  \fn QString &QString::setNum( float n, char f, int prec )
  Sets the string to the numerical value of \e n.
  \arg \e f is format specifier: 'f', 'F', 'e', 'E', 'g', 'G' (same
  meaning as for sprintf()).
  \arg \e prec is the precision.

  Returns a reference to the string.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Sets the characted at position \e index to \e c and expands the
  string if necessary.

  Returns FALSE if this \e index was out of range and the string could
  not be expanded, otherwise TRUE.
 ----------------------------------------------------------------------------*/

bool QString::setExpand( uint index, char c )
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


/*----------------------------------------------------------------------------
  \fn QString::operator char *() const
  Returns the string data.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QString::operator const char *() const
  Returns the string data.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator!() const
  Returns TRUE if it is a null string, otherwise FALSE.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Appends \e str to the string and returns a reference to the string.
 ----------------------------------------------------------------------------*/

QString& QString::operator+=( const char *str )
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

/*----------------------------------------------------------------------------
  Appends \e c to the string and returns a reference to the string.
 ----------------------------------------------------------------------------*/

QString &QString::operator+=( char c )
{
    uint len = length();
    if ( !QByteArray::resize( len + 2 ) )
	return *this;				// no memory

    *(data() + len) = c;
    *(data() + len+1) = '\0';

    return *this;
}


/*----------------------------------------------------------------------------
  \fn bool QString::operator==( const QString &s ) const
  Returns TRUE if the string is equal to \e s, or FALSE if they are
  different.
  \sa operator!=()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator!=( const QString &s ) const
  Returns TRUE if the string is different from \e s, or FALSE if they are
  equal.
  \sa operator==()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator==( const char *s ) const
  Returns TRUE if the string is equal to \e s, or FALSE if they are
  different.
  \sa operator!=()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator!=( const char *s ) const
  Returns TRUE if the string is different from \e s, or FALSE if they are
  equal.
  \sa operator==()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator==( char *s ) const
  Returns TRUE if the string is equal to \e s, or FALSE if they are
  different.
  \sa operator!=()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator!=( char *s ) const
  Returns TRUE if the string is different from \e s, or FALSE if they are
  equal.
  \sa operator==()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator<( const char *s ) const
  Returns TRUE if the string is less than \e s, otherwise FALSE.
  \sa operator>(), operator<=(), operator>=()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator>( const char *s ) const
  Returns TRUE if the string is greater than \e s, otherwise FALSE.
  \sa operator<(), operator<=(), operator>=()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator<=( const char *s ) const
  Returns TRUE if the string is less than or equal to \e s, otherwise FALSE.
  \sa operator<(), operator>(), operator>=()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QString::operator>=( const char *s ) const
  Returns TRUE if the string is greater than or equal to \e s, otherwise FALSE.
  \sa operator<(), operator>(), operator<=()
 ----------------------------------------------------------------------------*/


/*****************************************************************************
  QString stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QString
  Writes a string to the stream.

  Output format: [length (UINT32) data...]
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QString &str )
{
    return s.writeBytes( str.data(), str.size() );
}

/*----------------------------------------------------------------------------
  \relates QString
  Reads a string from the stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator>>( QDataStream &s, QString &str )
{
    UINT32 len;
    s >> len;					// read size of string
    if ( !str.QByteArray::resize( (uint)len )) {// resize string
#if defined(CHECK_NULL)
	warning( "QDataStream: Not enough memory to read QString" );
#endif
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( str.data(), (uint)len );
    return s;
}

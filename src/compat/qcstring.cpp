/****************************************************************************
**
** Implementation of extended char array operations and.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcstring.h"
#include "qregexp.h"
#include "qdatastream.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>


/*****************************************************************************
  QCString member functions
 *****************************************************************************/

/*!
    \class QCString qcstring.h
    \reentrant
    \brief The QCString class provides an abstraction of the classic C
    zero-terminated char array (char *).

    \internal

    QCString tries to behave like a more convenient \c{const char *}.
    The price of doing this is that some algorithms will perform
    badly. For example, append() is O(length()) since it scans for a
    null terminator. Although you might use QCString for text that is
    never exposed to the user, for most purposes, and especially for
    user-visible text, you should use QString. QString provides
    implicit sharing, Unicode and other internationalization support,
    and is well optimized.

    Note that for the QCString methods that take a \c{const char *}
    parameter the \c{const char *} must either be 0 (null) or not-null
    and '\0' (NUL byte) terminated; otherwise the results are
    undefined.

    A QCString that has not been assigned to anything is \e null, i.e.
    both the length and the data pointer is 0. A QCString that
    references the empty string ("", a single '\0' char) is \e empty.
    Both null and empty QCStrings are legal parameters to the methods.
    Assigning \c{const char *} 0 to QCString produces a null QCString.

    The length() function returns the length of the string; resize()
    resizes the string and truncate() truncates the string. A string
    can be filled with a character using fill(). Strings can be left
    or right padded with characters using leftJustify() and
    rightJustify(). Characters, strings and regular expressions can be
    searched for using find() and findRev(), and counted using
    contains().

    Strings and characters can be inserted with insert() and appended
    with append(). A string can be prepended with prepend().
    Characters can be removed from the string with remove() and
    replaced with replace().

    Portions of a string can be extracted using left(), right() and
    mid(). Whitespace can be removed using stripWhiteSpace() and
    simplifyWhiteSpace(). Strings can be converted to uppercase or
    lowercase with upper() and lower() respectively.

    Strings that contain numbers can be converted to numbers with
    toShort(), toInt(), toLong(), toULong(), toFloat() and toDouble().
    Numbers can be converted to strings with setNum().

    Many operators are overloaded to work with QCStrings. QCString
    also supports some more obscure functions, e.g. sprintf(),
    setStr() and setExpand().

    \target asciinotion
    \sidebar Note on Character Comparisons

    In QCString the notion of uppercase and lowercase and of which
    character is greater than or less than another character is locale
    dependent. This affects functions which support a case insensitive
    option or which compare or lowercase or uppercase their arguments.
    Case insensitive operations and comparisons will be accurate if
    both strings contain only ASCII characters. (If \c $LC_CTYPE is
    set, most Unix systems do "the right thing".) Functions that this
    affects include contains(), find(), findRev(), \l operator<(), \l
    operator<=(), \l operator>(), \l operator>=(), lower() and
    upper().

    This issue does not apply to \l{QString}s since they represent
    characters using Unicode.
    \endsidebar

    Performance note: The QCString methods for QRegExp searching are
    implemented by converting the QCString to a QString and performing
    the search on that. This implies a deep copy of the QCString data.
    If you are going to perform many QRegExp searches on a large
    QCString, you will get better performance by converting the
    QCString to a QString yourself, and then searching in the QString.
*/

/*!
    \fn QCString::QCString()

    Constructs a null string.

    \sa isNull()
*/

/*!
    \fn QCString::QCString( const QCString &s )

    Constructs a shallow copy \a s.

    \sa assign()
*/

/*! \fn QCString::QCString( int size )
    Constructs a string with room for \a size characters, including
    the '\0'-terminator. Makes a null string if \a size == 0.

    If \a size \> 0, then the first and last characters in the string
    are initialized to '\0'. All other characters are uninitialized.

    \sa resize(), isNull()
*/

/*! \fn QCString::QCString( const char *str )
    Constructs a string that is a deep copy of \a str.

    If \a str is 0 a null string is created.

    \sa isNull()
*/


/*! \fn QCString::QCString( const char *str, uint maxsize )

    Constructs a string that is a deep copy of \a str. The copy will
    be at most \a maxsize bytes long including the '\0'-terminator.

    Example:
    \code
    QCString str( "helloworld", 6 ); // assigns "hello" to str
    \endcode

    If \a str contains a 0 byte within the first \a maxsize bytes, the
    resulting QCString will be terminated by this 0. If \a str is 0 a
    null string is created.

    \sa isNull()
*/


/*!
    \fn QCString &QCString::operator=( const QCString &s )

    Assigns a shallow copy of \a s to this string and returns a
    reference to this string.
*/

/*!
    \overload QCString &QCString::operator=( const char *str )

    Assigns a deep copy of \a str to this string and returns a
    reference to this string.

    If \a str is 0 a null string is created.

    \sa isNull()
*/

/*
    \fn bool QCString::isNull() const

    Returns TRUE if the string is null, i.e. if data() == 0; otherwise
    returns FALSE. A null string is also an empty string.

    Example:
    \code
    QCString a;		// a.data() == 0,  a.size() == 0, a.length() == 0
    QCString b == "";	// b.data() == "", b.size() == 1, b.length() == 0
    a.isNull();		// TRUE  because a.data() == 0
    a.isEmpty();	// TRUE  because a.length() == 0
    b.isNull();		// FALSE because b.data() == ""
    b.isEmpty();	// TRUE  because b.length() == 0
    \endcode

    \sa isEmpty(), length(), size()
*/

/*
    \fn bool QCString::isEmpty() const

    Returns TRUE if the string is empty, i.e. if length() == 0;
    otherwise returns FALSE. An empty string is not always a null
    string.

    See example in isNull().

    \sa isNull(), length(), size()
*/

/*
    \fn uint QCString::length() const

    Returns the length of the string, excluding the '\0'-terminator.
    Equivalent to calling \c strlen(data()).

    Null strings and empty strings have zero length.

    \sa size(), isNull(), isEmpty()
*/

/*
    \fn bool QCString::truncate( uint pos )

    Truncates the string at position \a pos.

    Equivalent to calling \c resize(pos+1).

    Example:
    \code
    QCString s = "truncate this string";
    s.truncate( 5 );                      // s == "trunc"
    \endcode

    \sa resize()
*/



/*!
    Implemented as a call to the native vsprintf() (see the manual for
    your C library).

    If the string is shorter than 256 characters, this sprintf() calls
    resize(256) to decrease the chance of memory corruption. The
    string is resized back to its actual length before sprintf()
    returns.

    Example:
    \code
    QCString s;
    s.sprintf( "%d - %s", 1, "first" );		// result < 256 chars

    QCString big( 25000 );			// very long string
    big.sprintf( "%d - %s", 2, longString );	// result < 25000 chars
    \endcode

    \warning All vsprintf() implementations will write past the end of
    the target string (*this) if the \a format specification and
    arguments happen to be longer than the target string, and some
    will also fail if the target string is longer than some arbitrary
    implementation limit.

    Giving user-supplied arguments to sprintf() is risky: Sooner or
    later someone will paste a huge line into your application.
*/

QCString &QCString::sprintf( const char *format, ... )
{
    detach();
    va_list ap;
    va_start( ap, format );
    if ( size() < 256 )
	resize( 256 );		// make string big enough
    vsprintf( data(), format, ap );
    resize( qstrlen(constData()) );
    va_end( ap );
    return *this;
}



/*!
    \fn QCString QCString::copy() const

    Returns a deep copy of this string.

    \sa detach()
*/


/*!
    Returns a string of length \a width (plus one for the terminating
    '\0') that contains this string padded with the \a fill character.

    If the length of the string exceeds \a width and \a truncate is
    FALSE (the default), then the returned string is a copy of the
    string. If the length of the string exceeds \a width and \a
    truncate is TRUE, then the returned string is a left(\a width).

    Example:
    \code
    QCString s("apple");
    QCString t = s.leftJustify(8, '.');  // t == "apple..."
    \endcode

    \sa rightJustify()
*/

QCString QCString::leftJustify( uint width, char fill, bool truncate ) const
{
    QCString result;
    int len = qstrlen(constData());
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.resize( len+padlen );
	memcpy( result.data(), constData(), len );
	memset( result.data()+len, fill, padlen );
    } else {
	if ( truncate )
	    result = left( width );
	else
	    result = *this;
    }
    return result;
}

/*!
    Returns a string of length \a width (plus one for the terminating
    '\0') that contains zero or more of the \a fill character followed
    by this string.

    If the length of the string exceeds \a width and \a truncate is
    FALSE (the default), then the returned string is a copy of the
    string. If the length of the string exceeds \a width and \a
    truncate is TRUE, then the returned string is a left(\a width).

    Example:
    \code
    QCString s("pie");
    QCString t = s.rightJustify(8, '.');  // t == ".....pie"
    \endcode

    \sa leftJustify()
*/

QCString QCString::rightJustify( uint width, char fill, bool truncate ) const
{
    QCString result;
    int len = qstrlen(constData());
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.resize( len+padlen );
	memset( result.data(), fill, padlen );
	memcpy( result.data()+padlen, constData(), len );
    } else {
	if ( truncate )
	    result = left( width );
	else
	    result = *this;
    }
    return result;
}

/*!
    Returns the string converted to a \c long value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, or if it has trailing garbage; otherwise \a *ok is set to
    TRUE.
*/

long QCString::toLong( bool *ok ) const
{
    const char *p = constData();
    long val=0;
    const long max_mult = 214748364;
    bool is_ok = FALSE;
    int neg = 0;
    if ( !p )
	goto bye;
    while ( isspace((uchar) *p) )		// skip leading space
	p++;
    if ( *p == '-' ) {
	p++;
	neg = 1;
    } else if ( *p == '+' ) {
	p++;
    }
    if ( !isdigit((uchar) *p) )
	goto bye;
    while ( isdigit((uchar) *p) ) {
	if ( val > max_mult || (val == max_mult && (*p-'0') > 7+neg) )
	    goto bye;
	val = 10*val + (*p++ - '0');
    }
    if ( neg )
	val = -val;
    while ( isspace((uchar) *p) )		// skip trailing space
	p++;
    if ( *p == '\0' )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
    Returns the string converted to an \c{unsigned long} value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, or if it has trailing garbage; otherwise \a *ok is set to
    TRUE.
*/

ulong QCString::toULong( bool *ok ) const
{
    const char *p = constData();
    ulong val=0;
    const ulong max_mult = 429496729;
    bool is_ok = FALSE;
    if ( !p )
	goto bye;
    while ( isspace((uchar) *p) )		// skip leading space
	p++;
    if ( *p == '+' )
	p++;
    if ( !isdigit((uchar) *p) )
	goto bye;
    while ( isdigit((uchar) *p) ) {
	if ( val > max_mult || (val == max_mult && (*p-'0') > 5) )
	    goto bye;
	val = 10*val + (*p++ - '0');
    }
    while ( isspace((uchar) *p) )		// skip trailing space
	p++;
    if ( *p == '\0' )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
    Returns the string converted to a \c{short} value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, is out of range, or if it has trailing garbage; otherwise
    \a *ok is set to TRUE.
*/

short QCString::toShort( bool *ok ) const
{
    long v = toLong( ok );
    if ( ok && *ok && (v < -32768 || v > 32767) )
	*ok = FALSE;
    return (short)v;
}

/*!
    Returns the string converted to an \c{unsigned short} value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, is out of range, or if it has trailing garbage; otherwise
    \a *ok is set to TRUE.
*/

ushort QCString::toUShort( bool *ok ) const
{
    ulong v = toULong( ok );
    if ( ok && *ok && (v > 65535) )
	*ok = FALSE;
    return (ushort)v;
}


/*!
    Returns the string converted to a \c{int} value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, or if it has trailing garbage; otherwise \a *ok is set to
    TRUE.
*/

int QCString::toInt( bool *ok ) const
{
    return (int)toLong( ok );
}

/*!
    Returns the string converted to an \c{unsigned int} value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, or if it has trailing garbage; otherwise \a *ok is set to
    TRUE.
*/

uint QCString::toUInt( bool *ok ) const
{
    return (uint)toULong( ok );
}

/*!
    Returns the string converted to a \c{double} value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, or if it has trailing garbage; otherwise \a *ok is set to
    TRUE.
*/

double QCString::toDouble( bool *ok ) const
{
    char *end;
    double val = strtod( constData() ? constData() : "", &end );
    if ( ok )
	*ok = ( constData() && *constData() && ( end == 0 || *end == '\0' ) );
    return val;
}

/*!
    Returns the string converted to a \c{float} value.

    If \a ok is not 0: \a *ok is set to FALSE if the string is not a
    number, or if it has trailing garbage; otherwise \a *ok is set to
    TRUE.
*/

float QCString::toFloat( bool *ok ) const
{
    return (float)toDouble( ok );
}


/*! \fn QCString &QCString::setStr( const char *str )
    Makes a deep copy of \a str. Returns a reference to the string.
*/

/*!
    \overload

    Sets the string to the string representation of the number \a n
    and returns a reference to the string.
*/

QCString &QCString::setNum( long n )
{
    data();
    char buf[20];
    register char *p = &buf[19];
    bool neg;
    if ( n < 0 ) {
	neg = TRUE;
	n = -n;
    } else {
	neg = FALSE;
    }
    *p = '\0';
    do {
	*--p = ((int)(n%10)) + '0';
	n /= 10;
    } while ( n );
    if ( neg )
	*--p = '-';
    *this = p;
    return *this;
}

/*!
    \overload

    Sets the string to the string representation of the number \a n
    and returns a reference to the string.
*/

QCString &QCString::setNum( ulong n )
{
    data();
    char buf[20];
    register char *p = &buf[19];
    *p = '\0';
    do {
	*--p = ((int)(n%10)) + '0';
	n /= 10;
    } while ( n );
    *this = p;
    return *this;
}

/*!
    \overload QCString &QCString::setNum( int n )

    Sets the string to the string representation of the number \a n
    and returns a reference to the string.
*/

/*!
    \overload QCString &QCString::setNum( uint n )

    Sets the string to the string representation of the number \a n
    and returns a reference to the string.
*/

/*!
    \overload QCString &QCString::setNum( short n )

    Sets the string to the string representation of the number \a n
    and returns a reference to the string.
*/

/*!
    \overload QCString &QCString::setNum( ushort n )

    Sets the string to the string representation of the number \a n
    and returns a reference to the string.
*/

/*!
    Sets the string to the string representation of the number \a n
    and returns a reference to the string.

    The format of the string representation is specified by the format
    character \a f, and the precision (number of digits after the
    decimal point) is specified with \a prec.

    The valid formats for \a f are 'e', 'E', 'f', 'g' and 'G'. The
    formats are the same as for sprintf(); they are explained in \l
    QString::arg().
*/

QCString &QCString::setNum( double n, char f, int prec )
{
#ifndef QT_NO_DEBUG
    if ( !(f=='f' || f=='F' || f=='e' || f=='E' || f=='g' || f=='G') )
	qWarning( "QCString::setNum: Invalid format char '%c'", f );
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
    } else {
	*fs++ = prec + '0';
    }
    *fs++ = 'l';
    *fs++ = f;
    *fs = '\0';
    return sprintf( format, n );
}

/*! \overload QCString &QCString::setNum( float n, char f, int prec ) */


/*!
    Sets the character at position \a index to \a c and expands the
    string if necessary, padding with spaces.

    Returns FALSE if \a index was out of range and the string could
    not be expanded; otherwise returns TRUE.
*/

bool QCString::setExpand( uint index, char c )
{
    uint oldlen = length();
    if ( index >= oldlen ) {
	resize( index+1 );
	if ( index > oldlen )
	    memset( data() + oldlen, ' ', index - oldlen );
    }
    *(data() + index) = c;
    return TRUE;
}


/*
    \fn QCString::operator const char *() const

    Returns the string data.
*/


/*!
    \fn QCString& QCString::append( const char *str )

    Appends string \a str to the string and returns a reference to the
    string. Equivalent to operator+=().
*/



/*! \fn QDataStream &operator<<( QDataStream &s, const QCString &str )
    \relates QCString

    Writes string \a str to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

/*!
  \fn QDataStream &operator>>( QDataStream &s, QCString &str )
    \relates QCString

    Reads a string into \a str from the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

/*****************************************************************************
  Documentation for related functions
 *****************************************************************************/

/*!
    \fn bool operator==( const QCString &s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 and \a s2 are equal; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) == 0.
*/

/*!
    \overload bool operator==( const QCString &s1, const char *s2 )

    \relates QCString

    Returns TRUE if \a s1 and \a s2 are equal; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) == 0.
*/

/*!
    \overload bool operator==( const char *s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 and \a s2 are equal; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) == 0.
*/

/*!
    \fn bool operator!=( const QCString &s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 and \a s2 are different; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) != 0.
*/

/*!
    \overload bool operator!=( const QCString &s1, const char *s2 )

    \relates QCString

    Returns TRUE if \a s1 and \a s2 are different; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) != 0.
*/

/*!
    \overload bool operator!=( const char *s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 and \a s2 are different; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) != 0.
*/

/*!
    \fn bool operator<( const QCString &s1, const char *s2 )

    \relates QCString

    Returns TRUE if \a s1 is less than \a s2; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \< 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \overload bool operator<( const char *s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 is less than \a s2; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \< 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \fn bool operator<=( const QCString &s1, const char *s2 )

    \relates QCString

    Returns TRUE if \a s1 is less than or equal to \a s2; otherwise
    returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \<= 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \overload bool operator<=( const char *s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 is less than or equal to \a s2; otherwise
    returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \<= 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \fn bool operator>( const QCString &s1, const char *s2 )

    \relates QCString

    Returns TRUE if \a s1 is greater than \a s2; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \> 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \overload bool operator>( const char *s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 is greater than \a s2; otherwise returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \> 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \fn bool operator>=( const QCString &s1, const char *s2 )

    \relates QCString

    Returns TRUE if \a s1 is greater than or equal to \a s2; otherwise
    returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \>= 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \overload bool operator>=( const char *s1, const QCString &s2 )

    \relates QCString

    Returns TRUE if \a s1 is greater than or equal to \a s2; otherwise
    returns FALSE.

    Equivalent to qstrcmp(\a s1, \a s2) \>= 0.

    \sa \link #asciinotion Note on character comparisons \endlink
*/

/*!
    \fn const QCString operator+( const QCString &s1, const QCString &s2 )

    \relates QCString

    Returns a string which consists of the concatenation of \a s1 and
    \a s2.
*/

/*!
    \overload const QCString operator+( const QCString &s1, const char *s2 )

    \relates QCString

    Returns a string which consists of the concatenation of \a s1 and \a s2.
*/

/*!
    \overload const QCString operator+( const char *s1, const QCString &s2 )

    \relates QCString

    Returns a string which consists of the concatenation of \a s1 and \a s2.
*/

/*!
    \overload const QCString operator+( const QCString &s, char c )

    \relates QCString

    Returns a string which consists of the concatenation of \a s and \a c.
*/

/*!
    \overload const QCString operator+( char c, const QCString &s )

    \relates QCString

    Returns a string which consists of the concatenation of \a c and \a s.
*/

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstring.cpp#183 $
**
** Implementation of the QString class and related Unicode functions
**
** Created : 920722
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

#include "qstring.h"
#include "qregexp.h"
#include "qdatastream.h"
#include "qtextcodec.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

// ##### Unicode fns need to ifdef UNICODE (if we go that way)
QChar uctolower(QChar c)
{
    return c.row ? c : QChar((char)tolower(c.cell));
}
QChar uctoupper(QChar c)
{
    return c.row ? c : QChar((char)toupper(c.cell));
}
bool ucisspace(QChar c)
{
    return c.row ? FALSE : isspace(c.cell);
}
bool ucisdigit(QChar c)
{
    return c.row ? FALSE : isdigit(c.cell);
}
int ucstrcmp( const QString &as, const QString &bs )
{
    if ( as.d == bs.d )
	return 0;
    const QChar *a = as.unicode();
    const QChar *b = bs.unicode();
    int l=QMIN(as.length(),bs.length());
    while ( l-- && *a == *b )
	a++,b++;
    if ( l==-1 )
	return ( as.length()-bs.length() );
    return a->row == b->row ? a->cell - b->cell : a->row - b->row;
}
int ucstrncmp( const QChar *a, const QChar *b, int l )
{
    while ( l-- && *a == *b )
	a++,b++;
    if ( l==-1 )
	return 0;
    return *a - *b;
}
int ucstrnicmp( const QChar *a, const QChar *b, int l )
{
    while ( l-- && uctolower(*a) == uctolower(*b) )
	a++,b++;
    if ( l==-1 )
	return 0;
    QChar al = uctolower(*a);
    QChar bl = uctolower(*b);
    return al.row == bl.row ? al.cell - bl.cell : al.row - bl.row;
}


/*!
  \class QChar qstring.h
  \brief A Unicode character.

  A QChar is a simple 16-bit value representing a Unicode character.
  Most C++ compilers will process them much as they would a "short int".

  QChar values are normally used in combination with QString.

  They can be trivially constructed from:
   <dl>
    <dt>\c char
      <dd>the char is assumed to be a Latin-1 character,
	    for example QChar('Q') is the Unicode character U0051
	    and QChar('t') is the Unicode character U0074.
    <dt>\c short
      <dd>the MSB is the high-order byte (row) of the Unicode character,
	    for example QChar(ushort(0x6817)) is the Unicode character
	    U6817 meaning <em>kuri</em> - chestnut tree - in Japanese.
    <dt>\c char, \c char
      <dd>the characters are LSB and MSB respectively,
	    for example QChar(0x25,0x04) is the Unicode character U0425
	    which is the Cyrillic letter YA which looks like a mirrored R.
   </dl>
*/


/*!
  Returns whether the character is a whitespace character.
*/
bool QChar::isSpace() const
{
    return ucisspace(*this);
}

/*!
  \fn QChar::operator char() const

  Returns the Latin1 character equivalent to the QChar,
  or 0.  This is mainly useful for non-internationalized software.

  \sa unicode()
*/

/*!
  \fn ushort QChar::unicode() const

  Returns the numeric Unicode value equal to the QChar.  Normally, you
  should use QChar objects as they are equivalent, but for some low-level
  tasks (eg. indexing into an array of Unicode information), this function
  is useful.
*/

/*!
  This utility function converts the 8-bit string
  \a ba to Unicode, returning the result.

  The caller is responsible for deleting the return value with delete[].
*/

QChar* QString::asciiToUnicode( const QByteArray& ba, uint* len )
{
    int l = 0;
    while ( l < (int)ba.size() && ba[l] )
	l++;
    char* str = ba.data();
    QChar *uc = new QChar[l];
    QChar *result = uc;
    if ( len )
	*len = l;
    while (l--)
	*uc++ = *str++;
    return result;
}

/*!
  This utility function converts the NUL-terminated 8-bit string
  \a str to Unicode, returning the result and setting \a len to
  the length of the Unicode string.

  The caller is responsible for deleting the return value with delete[].
*/

QChar* QString::asciiToUnicode(const char *str, uint* len, uint maxlen )
{
    QChar* result = 0;
    uint l = 0;
    if ( str ) {
	if ( maxlen != (uint)-1 ) {
	    while (str[l] && l < maxlen)
		l++;
	} else {
	    // Faster?
	    l = strlen(str);
	}
	QChar *uc = new QChar[l];
	result = uc;
	uint i = l;
	while ( i-- )
	    *uc++ = *str++;
    }
    if ( len )
	*len = l;
    return result;
}

/*!
  This utility function converts \a l 16-bit characters from
  \a uc to ASCII, returning a NUL-terminated string.

  The caller is responsible for deleting the string with delete[].
*/
char* QString::unicodeToAscii(const QChar *uc, uint l)
{
    if (!uc) {
	return 0;
    }
    char *a = new char[l+1];
    char *result = a;
    while (l--)
	*a++ = *uc++;
    *a++ = '\0';
    return result;
}

/*****************************************************************************
  QString member functions
 *****************************************************************************/

/*!
  \class QString qstring.h

  \brief The QString class provides an abstraction of Unicode text and
          the classic C zero-terminated char array (<var>char*</var>).

  \ingroup tools
  \ingroup shared

  QString uses implicit
  \link shclass.html sharing\endlink, and so it is very efficient
  and easy to use.

  In all QString methods that take <var>const char*</var> parameters,
  the <var>const char*</var> is interpreted as a classic C-style
  0-terminated ASCII string. It is legal for the <var>const
  char*</var> parameter to be 0. The results are undefined if the
  <var>const char*</var> string is not 0-terminated. Functions that
  copy classic C strings into a QString will not copy the terminating
  0-character. The QChar array of the QString (as returned by
  unicode()) is not terminated by a null.

  A QString that has not been assigned to anything is \a null, i.e. both
  the length and data pointer is 0. A QString that references the empty
  string ("", a single '\0' char) is \a empty.	Both null and empty
  QStrings are legal parameters to the methods. Assigning <var>const char
  * 0</var> to QString gives a null QString.

  \sa \link shclass.html Shared classes\endlink
*/

Q_EXPORT QString::Data *QString::shared_null = 0;
QT_STATIC_CONST_IMPL QString QString::null;
QT_STATIC_CONST_IMPL QChar QChar::null;
QT_STATIC_CONST_IMPL QChar QChar::replacement((ushort)0xfffd);
QT_STATIC_CONST_IMPL QChar QChar::byteOrderMark((ushort)0xfeff);
QT_STATIC_CONST_IMPL QChar QChar::byteOrderSwapped((ushort)0xfffe);

#define Q2HELPER(x) x
#ifdef Q2HELPER
static int stat_construct_charstar=0;
static int stat_construct_charstar_size=0;
static int stat_construct_null=0;
static int stat_construct_int=0;
static int stat_construct_int_size=0;
static int stat_construct_ba=0;
static int stat_get_ascii=0;
static int stat_get_ascii_size=0;
static int stat_copy_on_write=0;
static int stat_copy_on_write_size=0;
static int stat_fast_copy=0;
void Q_EXPORT qt_qstring_stats()
{
	debug("construct_charstar = %d (%d chars)", stat_construct_charstar, stat_construct_charstar_size);
	debug("construct_null = %d", stat_construct_null);
	debug("construct_int = %d (%d chars)", stat_construct_int, stat_construct_int_size);
	debug("construct_ba = %d", stat_construct_ba);
	debug("get_ascii = %d (%d chars)", stat_get_ascii, stat_get_ascii_size);
	debug("copy_on_write = %d (%d chars)", stat_copy_on_write, stat_copy_on_write_size);
	debug("fast_copy = %d", stat_fast_copy);
}
#endif

/*!
  \fn QString::QString()

  Constructs a null string.
  \sa isNull()
*/

/*!
  Constructs a string containing the one character \a ch.
*/
QString::QString( const QChar& ch )
{
    d = new Data(new QChar[1],1,1);
    d->unicode[0] = ch;
}

/*!
  Constructs an implicitly-shared copy of \a s.
*/
QString::QString( const QString &s ) :
    d(s.d)
{
    Q2HELPER(stat_fast_copy++);
    d->ref();
}

/*!
  Private function.

  Constructs a string with preallocated space for \a size - 1 characters.

  The string is empty.

  \sa resize(), isNull()
*/

QString::QString( int size )
{
    if ( size ) {
	Q2HELPER(stat_construct_int++);
	int l = size-1;
	Q2HELPER(stat_construct_int_size+=l);
	d = new Data(new QChar[l],0,l);
    } else {
	Q2HELPER(stat_construct_null++);
	d = shared_null ? shared_null : shared_null=new Data;
	d->ref();
    }
}

/*!
  Constructs a string that is a deep copy of \a ba interpreted as
  interpreted as a classic C string.
*/

QString::QString( const QByteArray& ba )
{
    Q2HELPER(stat_construct_ba++);
    uint l;
    QChar *uc = asciiToUnicode(ba,&l);
    d = new Data(uc,l,l);
}

/*!
  Constructs a string that is a deep copy of the
  first \a length QChar in the array \a unicode.
*/

QString::QString( QChar* unicode, uint length )
{
    QChar* uc = new QChar[ length ];
    memcpy(uc, unicode, length*sizeof(QChar));
    d = new Data(uc,length,length);
}


/*!
  Constructs a string that is a deep copy of \a str, interpreted as a
  classic C string.

  If \a str is 0 a null string is created.

  \sa isNull()
*/

QString::QString( const char *str )
{
    Q2HELPER(stat_construct_charstar++);
    uint l;
    QChar *uc = asciiToUnicode(str,&l);
    Q2HELPER(stat_construct_charstar_size+=l);
    d = new Data(uc,l,l);
}


/*!
  Constructs a string that is a deep copy of \a str, interpreted as a
  classic C string. If the size (including the 0-terminator) of \a str
  is larger than \a maxSize, only the first \a maxSize - 1 characters
  will be copied.

  If \a str is 0 a null string is created.

  Example:
  \code
    QString s( "helloworld", 6 );  // s == "hello"
  \endcode

  \sa isNull()
*/

QString::QString( const char *str, uint maxSize )
{
    Q2HELPER(stat_construct_charstar++);
    uint l;
    QChar *uc = asciiToUnicode( str, &l, maxSize-1 );
    d = new Data(uc,l,l);
}

/*!
  Deallocates any space reserved solely by this QString.
*/

QString::~QString()
{
    deref();
}

void QString::real_detach()
{
    setLength( length() );
}

void QString::deref()
{
    if ( d->deref() ) {
	delete d;
	d = 0; // helps debugging
    }
}

/*!
  Assigns a shallow copy of \a s to this string and returns a
  reference to this string.
*/
QString &QString::operator=( const QString &s )
{
    Q2HELPER(stat_fast_copy++);
    s.d->ref();
    deref();
    d = s.d;
    return *this;
}

/*!
  Assigns a deep copy of \a ba, interpretted a classic C string, to
  this string and returns a reference to this string.
*/
QString &QString::operator=( const QByteArray& ba )
{
    deref();
    uint l;
    QChar *uc = asciiToUnicode(ba,&l);
    d = new Data(uc,l,l);
    return *this;
}

/*!
  Assigns a deep copy of \a str, interpretted a classic C string to
  this string and returns a reference to this string.

  If \a str is 0 a null string is created.

  \sa isNull()
*/
QString &QString::operator=( const char *str )
{
    deref();
    uint l;
    QChar *uc = asciiToUnicode(str,&l);
    d = new Data(uc,l,l);
    return *this;
}

/*!
  \fn bool QString::isNull() const

  Returns TRUE if the string is null.
  A null string is also an empty string.

  Example:
  \code
    QString a;		// a.unicode() == 0,  a.length() == 0
    QString b == "";	// b.unicode() == "", b.length() == 0
    a.isNull();		// TRUE, because a.unicode() == 0
    a.isEmpty();	// TRUE, because a.length() == 0
    b.isNull();		// FALSE, because b.unicode() != 0
    b.isEmpty();	// TRUE, because b.length() == 0
  \endcode

  \sa isEmpty(), length()
*/

/*!
  \fn bool QString::isEmpty() const

  Returns TRUE if the string is empty, i.e. if length() == 0.
  An empty string is not always a null string.

  See example in isNull().

  \sa isNull(), length()
*/

/*!
  \fn uint QString::length() const

  Returns the length of the string.

  Null strings and empty strings have zero length.

  \sa isNull(), isEmpty()
*/

/*!
  Truncates the string at position \a newLen. If newLen is less than the
  current length, this is equivalent to setLength( newLen ). Otherwise,
  nothing happens.

  Example:
  \code
    QString s = "truncate this string";
    s.truncate( 5 );				// s == "trunc"
  \endcode

  \sa setLength()
*/
//### different behaviour than 1.x - but insignificant?
void QString::truncate( uint newLen )
{
    if ( newLen < d->len )
	setLength( newLen );
	
    //### Is there any point in extending the array if newlen > d->len ?
    // (Qt 1.x did, but there, one could access the data directly...)
}

/*!
  Ensures that at least \a len characters are allocated, and sets the
  length to \a len. Will detach. New space is \e not defined.

  If \a len is 0, this string becomes empty, unless this string is null,
  in which case it remains null.

  \sa truncate(), isNull(), isEmpty()
*/

void QString::setLength( uint newLen )
{
    if ( d->count != 1 || newLen > d->maxl || 		// detach, grow, or
	 ( newLen*4 < d->maxl && d->maxl > 4 ) ) {	// shrink
	Q2HELPER(stat_copy_on_write++);
	Q2HELPER(stat_copy_on_write_size+=d->len);
	uint newMax = 4;
	while ( newMax < newLen )
	    newMax *= 2;
	QChar* nd = new QChar[newMax];
	uint len = QMIN( d->len, newLen );
	if ( d->unicode )
	    memcpy( nd, d->unicode, sizeof(QChar)*len );
	deref();
	d = new Data( nd, newLen, newMax );
    } else {
	d->len = newLen;
	d->dirtyascii = 1;
    }
}

/*!
  Returns a string equal to this one, but with the first
  occurrence of <tt>%<em>digit</em></tt> replaced by the
  text \a a.  This is particularly useful for translations,
  as it allows the order of the replacements to be controlled by the
  translator.  For example:

  \code
    label.setText( tr("I have %1 to your %2").arg(mine).arg(yours) );
  \endcode

  If there is no <tt>%<em>digit</em></tt> pattern, a
  \link warning() warning message\endlink is printed and the
  text as appended with a space at the end of the string.
  This is error-recovery and should not be occur in correct code.

  \a fieldwidth is the minimum amount of space the text will be padded
  to.  A positive value produces right-aligned text, while a negative
  value produces left aligned text.

  \sa QObject::tr()
*/
QString QString::arg(const QString& a, int fieldwidth) const
{
    int pos, len;
    QString r = copy();

    if ( !findArg(pos,len) ) {
	warning("Argument missing");
	// Make sure the text at least appears SOMEWHERE
	r += " ";
	pos = r.length();
	len = 0;
    }

    r.replace(pos,len,a);
    if ( fieldwidth < 0 ) {
	QString s = "";
	while ( (uint)-fieldwidth > a.length() ) {
	    s += ' ';
	    fieldwidth++;
	}
	r.insert(pos+a.length(),s);
    } else if ( fieldwidth ) {
	QString s;
	while ( (uint)fieldwidth > a.length() ) {
	    s += ' ';
	    fieldwidth--;
	}
	r.insert(pos,s);

    }
    return r;
}

/*!
  Returns a string equal to this one, but with the first
  occurrence of <tt>%<em>digit</em></tt> replaced by the
  integer value \a in base \a base (defaults to decimal).

  The value is converted to \a base notation (default is decimal).
  The base must be a value from 2 to 36.

  See arg(const QString&,int) for more details.
*/
QString QString::arg(long a, int fieldwidth, int base) const
{
    QString n;
    n.setNum(a,base);
    return arg(n,fieldwidth);
}

/*!
  Returns a string equal to this one, but with the first
  occurrence of <tt>%<em>digit</em></tt> replaced by the
  unsigned integer value \a in base \a base (defaults to decimal).

  The value is converted to \a base notation (default is decimal).
  The base must be a value from 2 to 36.

  See arg(const QString&,int) for more details.
*/
QString QString::arg(ulong a, int fieldwidth, int base) const
{
    QString n;
    n.setNum(a,base);
    return arg(n,fieldwidth);
}

/*!
  \fn QString QString::arg(int a, int fieldwidth, int base) const

  See QString::arg(long a, int fieldwidth, int base).
*/

/*!
  \fn QString QString::arg(uint a, int fieldwidth, int base) const

  See QString::arg(ulong a, int fieldwidth, int base).
*/


/*!
  Returns a string equal to this one, but with the first
  occurrence of <tt>%<em>digit</em></tt> replaced by the
  character \a a.

  See arg(const QString&,int) for more details.
*/
QString QString::arg(char a, int fieldwidth) const
{
    QString c;
    c += a;
    return arg(c,fieldwidth);
}

/*!
  Returns a string equal to this one, but with the first
  occurrence of <tt>%<em>digit</em></tt> replaced by the
  character \a a.

  See arg(const QString&,int) for more details.
*/
QString QString::arg(QChar a, int fieldwidth) const
{
    QString c;
    c += a;
    return arg(c,fieldwidth);
}

/*!
  Returns a string equal to this one, but with the first
  occurrence of <tt>%<em>digit</em></tt> replaced by the
  value \a a.

  See arg(const QString&,int) for more details.
*/
QString QString::arg(double a, int fieldwidth, char fmt, int prec)
{
    QString dec;
    dec.setNum(a,fmt,prec);
    return arg(dec,fieldwidth);
}


/*!
  Just 1-digit arguments.
*/
bool QString::findArg(int& pos, int& len) const
{
    char lowest=0;
    for (uint i=0; i<length(); i++) {
	if ( at(i) == '%' ) {
	    char d = at(i+1);
	    if ( d >= '0' && d <= '9' ) {
		if ( !lowest || d < lowest ) {
		    lowest = d;
		    pos = i;
		    len = 2;
		}
	    }
	}
    }
    return lowest != 0;
}

/*!
  Safely builds a formatted string from a format string and an
  arbitrary list of arguments.  The format string supports all
  the escape sequences of printf() in the standard C library.

  This method offers no Unicode support. For typesafe string building,
  with full Unicode support, you can use QTextOStream like this:

  \code
    QString str;
    QString s = ...;
    int x = ...;
    QTextOStream(str) << s << " : " << x;
  \endcode

  For \link QObject::tr() translations,\endlink especially if the
  strings contains more than one escape sequence, you should consider
  using the arg() function instead.  This allows the order of the
  replacements to be controlled by the translator, and has Unicode
  support.

  Example:
  \code
    QString str;
    const char* s = ...;
    int x = ...;
    str.sprintf( "%s : %d", s, x );
  \endcode

  \sa arg(const QString&,int)
*/

QString &QString::sprintf( const char* cformat, ... )
{
    va_list ap;
    va_start( ap, cformat );

    if ( !cformat ) {
	// Qt 1.x compat
	*this = "";
	return *this;
    }
    QString format = cformat;

    static QRegExp escape("%#?0?-? ?\\+?'?[0-9*]*\\.?[0-9*]*h?l?L?q?Z?");

    QString result;
    uint last=0;

    int len=0;
    int pos=-1;
    while ( 1 ) {
	pos=escape.match( format, last, &len );
	// Non-escaped text
	if ( pos > (int)last ) {
	    result += format.mid(last,pos-last);
//debug("%d UNESCAPED from %d = %s",pos-last,last,format.mid(last,pos-last).ascii());
	}
	if ( pos < 0 ) {
	    // The rest
//debug("THE REST = %s",format.mid(last).ascii());
	    if ( last < format.length() )
		result += format.mid(last);
	    break;
	}
	last = pos + len + 1;

	// Escape
	QString f = format.mid(pos,len);
//debug("fmt=%s",f.ascii());
	uint width, decimals;
	int params=0;
	int wpos = f.find('*');
	if ( wpos >= 0 ) {
	    params++;
	    width = va_arg(ap, int);
//debug("pwidth=%d",width);
	    if ( f.find('*',wpos+1) >= 0 ) {
		decimals = va_arg(ap, int);
//debug("pdec=%d",decimals);
		params++;
	    } else {
		decimals = 0;
	    }
	} else {
	    decimals = width = 0;
	}
	QString replacement;
	if ( format[pos+len] == 's' ||
	     format[pos+len] == 'S' ||
	     format[pos+len] == 'c' )
	{
	    bool rightjust = ( f.find('-') < 0 );
	    // Yes, %-5s really means left adjust in sprintf
//if ( rightjust ) debug("rightjust");

	    if ( wpos < 0 ) {
		QRegExp num("[0-9]+");
		int nlen;
		int p = num.match(f,0,&nlen);
		if ( p >= 0 ) {
		    width = f.mid(p,nlen).toInt();
		    /* not used
		    p = num.match(f,p+1,&nlen);
		    if ( p >= 0 ) {
			decimals = f.mid(p,nlen).toInt();
		    }
		    */
		}
	    }

	    if ( format[pos+len] == 's' ) {
		QString s = va_arg(ap, char*);
		replacement = s;
	    /*
	    } else if ( format[pos+len] == 'S' ) {
		QString *s = va_arg(ap, QString*);
		replacement = *s;
	    */
	    } else {
		int ch = va_arg(ap, int);
		replacement = QChar((ushort)ch);
	    }
	    if ( replacement.length() < width ) {
		replacement = rightjust
		    ? replacement.rightJustify(width)
		    : replacement.leftJustify(width);
	    }
//debug("rep=%s",replacement.ascii());
	} else if ( format[pos+len] == '%' ) {
	    replacement = "%";
	} else if ( format[pos+len] == 'n' ) {
	    int* n = va_arg(ap, int*);
	    *n = result.length();
	} else {
	    char in[64], out[128];
	    strncpy(in,f,63);
	    char fch = format[pos+len];
	    in[f.length()] = fch;
	    switch ( fch ) {
	      case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': {
		int value = va_arg(ap, int);
		switch (params) {
		  case 0: ::sprintf( out, in, value ); break;
		  case 1: ::sprintf( out, in, width, value ); break;
		  case 2: ::sprintf( out, in, width, decimals, value ); break;
		}
	      } break;
	      case 'e': case 'E': case 'f': case 'g': {
		double value = va_arg(ap, double);
		switch (params) {
		  case 0: ::sprintf( out, in, value ); break;
		  case 1: ::sprintf( out, in, width, value ); break;
		  case 2: ::sprintf( out, in, width, decimals, value ); break;
		}
	      } break;
	      case 'p': {
		void* value = va_arg(ap, void*);
		switch (params) {
		  case 0: ::sprintf( out, in, value ); break;
		  case 1: ::sprintf( out, in, width, value ); break;
		  case 2: ::sprintf( out, in, width, decimals, value ); break;
		}
	      } break;
	    }
//debug("  %s -> %s",in,out);
	    replacement = out;
	}
//debug("%s%c -> %s",f.ascii(),(char)format[pos+len],replacement.ascii());
	result += replacement;
//debug("now %s",result.ascii());
    }
    *this = result;

    va_end( ap );
    return *this;
}

/*!
  Fills the string with \a len characters of value \a c.

  If \a len is negative, the current string length is used.
*/

void QString::fill( QChar c, int len )
{
    if ( len < 0 )
	len = length();
    deref();
    QChar * nd = new QChar[len];
    d = new Data(nd,len,len);
    while (len--) *nd++ = c;
}


/*!
  \fn QString QString::copy() const
  \obsolete
  Returns a deep copy of this string.
*/

/*!
  Finds the first occurrence of the character \a c, starting at
  position \a index.

  The search is case sensitive if \a cs is TRUE, or case insensitive
  if \a cs is FALSE.

  Returns the position of \a c, or -1 if \a c could not be found.
*/

int QString::find( QChar c, int index, bool cs ) const
{
    if ( (uint)index >= length() )		// index outside string
	return -1;
    register const QChar *uc;
    uc = unicode()+index;
    int n = length()-index;
    if ( cs ) {
	while ( n-- && *uc != c )
	    uc++;
    } else {
	c = uctolower( c );
	while ( n-- && uctolower(*uc) != c )
	    uc++;
    }
    if ( uint(uc - unicode()) >= length() )
	return -1;
    return (int)(uc - unicode());
}

/*!
  Finds the first occurrence of the string \a str, starting at position
  \a index.

  The search is case sensitive if \a cs is TRUE, or case insensitive if \a
  cs is FALSE.

  Returns the position of \a str, or -1 if \a str could not be found.
*/

int QString::find( const QString& str, int index, bool cs ) const
{
    if ( (uint)index >= length() )		// index outside string
	return -1;
    register const QChar *uc;
    uc = unicode()+index;
    uint n = length()-index+1;
    uint strl = str.length();
    if ( cs ) {
	while ( n-- >= strl && ucstrncmp(uc,str.d->unicode,strl) )
	    uc++;
    } else {
	while ( n-- >= strl && ucstrnicmp(uc,str.d->unicode,strl) )
	    uc++;
    }
    return uc - unicode() <= length()-strl ? (int)(uc - unicode()) : -1;
}

/*!
  \fn int QString::findRev( const char* str, int index ) const

  Equivalent to findRev(QString(str), index).
*/

/*!
  \fn int QString::find( const char* str, int index ) const

  Equivalent to find(QString(str), index).
*/

/*!
  Finds the first occurrence of the character \a c, starting at
  position \a index and searching backwards. If \a index is negative,
  the search starts at the end.

  The search is case sensitive if \a cs is TRUE, or case insensitive if \a
  cs is FALSE.

  Returns the position of \a c, or -1 if \a c could not be found.
*/

int QString::findRev( QChar c, int index, bool cs ) const
{
    QString t( c );
    return findRev( t, index, cs );
}

/*!
  Finds the first occurrence of the string \a str, starting at
  position \a index and searching backwards. If \a index is negative,
  the search starts at the end.

  The search is case sensitive if \a cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \a str, or -1 if \a str could not be found.
*/

int QString::findRev( const QString& str, int index, bool cs ) const
{
    uint slen = str.length();
    if ( !slen )
	return index;
    if ( index < 0 )				// neg index ==> start from end
	index = length()-slen;
    else if ( (uint)index > length() )		// bad index
	return -1;
    else if ( (uint)index == length() )		// bad index, but accept it
	index--;
    else if ( (uint)(index + slen) > length() ) // str would be too long
	index = length() - slen;
    if ( index < 0 )
	return -1;

    register const QChar *uc = unicode() + index;
    if ( cs ) {					// case sensitive
	for ( int i=index; i>=0; i-- )
	    if ( ucstrncmp(uc--,str.unicode(),slen)==0 )
		return i;
    } else {					// case insensitive
	for ( int i=index; i>=0; i-- )
	    if ( ucstrnicmp(uc--,str.unicode(),slen)==0 )
		return i;
    }
    return -1;
}


/*!
  Returns the number of times the character \a c occurs in the string.

  The match is case sensitive if \a cs is TRUE, or case insensitive if \a cs
  if FALSE.
*/

int QString::contains( QChar c, bool cs ) const
{
    int count = 0;
    const QChar *uc = unicode();
    if ( !uc )
	return 0;
    int n = length();
    if ( cs ) {					// case sensitive
	while ( n-- )
	    if ( *uc++ == c )
		count++;
    } else {					// case insensitive
	c = uctolower( c );
	while ( n-- ) {
	    if ( uctolower(*uc) == c )
		count++;
	    uc++;
	}
    }
    return count;
}

/*!
  \overload
*/
int QString::contains( const char* str, bool cs ) const
{
    return contains(QString(str),cs);
}

/*!
  \fn int QString::contains (char c, bool cs=TRUE) const
  \overload
*/

/*!
  \fn int QString::find (char c, int index=0, bool cs=TRUE) const
  \overload
*/

/*!
  \fn int QString::findRev (char c, int index=-1, bool cs=TRUE) const
  \overload
*/

/*!
  Returns the number of times \a str occurs in the string.

  The match is case sensitive if \a cs is TRUE, or case insensitive if \e
  cs if FALSE.

  This function counts overlapping substrings, for example, "banana"
  contains two occurrences of "ana".

  \sa findRev()
*/

int QString::contains( const QString &str, bool cs ) const
{
    int count = 0;
    const QChar *uc = unicode();
    if ( !uc )
	return 0;
    int len = str.length();
    int n = length();
    while ( n-- ) {				// counts overlapping strings
	// ### Doesn't account for length of this - searches over "end"
	if ( cs ) {
	    if ( ucstrncmp( uc, str.unicode(), len ) == 0 )
		count++;
	} else {
	    if ( ucstrnicmp(uc, str.unicode(), len) == 0 )
		count++;
	}
	uc++;
    }
    return count;
}

/*!
  Returns a substring that contains the \a len leftmost characters
  of the string.

  The whole string is returned if \a len exceeds the length of the
  string.


  Example:
  \code
    QString s = "Pineapple";
    QString t = s.left( 4 );			// t == "Pine"
  \endcode

  \sa right(), mid(), isEmpty()
*/

QString QString::left( uint len ) const
{
    if ( isEmpty() ) {
	return QString();
    } else if ( len == 0 ) {			// ## just for 1.x compat:
	QString empty = "";			// not null, but empty###
	return empty;
    } else if ( len > length() ) {
	return *this;
    } else {
	QString s( len+1 );
	memcpy( s.d->unicode, d->unicode, len*sizeof(QChar) );
	s.d->len = len;
	return s;
    }
}

/*!
  Returns a substring that contains the \a len rightmost characters
  of the string.

  The whole string is returned if \a len exceeds the length of the
  string.

  Example:
  \code
    QString s = "Pineapple";
    QString t = s.right( 5 );			// t == "apple"
  \endcode

  \sa left(), mid(), isEmpty()
*/

QString QString::right( uint len ) const
{
    if ( isEmpty() || len == 0 ) {
	return QString();
    } else {
	uint l = length();
	if ( len > l )
	    len = l;
	QString s( len+1 );
	memcpy( s.d->unicode, d->unicode+(l-len), len*sizeof(QChar) );
	s.d->len = len;
	return s;
    }
}

/*!
  Returns a substring that contains the \a len characters of this
  string, starting at position \a index.

  Returns a null string if the string is empty or \a index is out
  of range.  Returns the whole string from \a index if \a index+len exceeds
  the length of the string.

  Example:
  \code
    QString s = "Two pineapples";
    QString t = s.mid( 4, 4 );			// t == "pine"
  \endcode

  \sa left(), right()
*/

QString QString::mid( uint index, uint len ) const
{
    uint slen = length();
    if ( isEmpty() || index >= slen || len == 0 ) {
	return QString();
    } else {
	if ( len > slen-index )
	    len = slen - index;
	register const QChar *p = unicode()+index;
	QString s( len+1 );
	memcpy( s.d->unicode, p, len*sizeof(QChar) );
	s.d->len = len;
	return s;
    }
}

/*!
  Returns a string of length \a width that contains this
  string and padded by the \a fill character.

  If the length of the string exceeds \a width and \a truncate is FALSE,
  then the returned string is a copy of the string.
  If the length of the string exceeds \a width and \a truncate is TRUE,
  then the returned string is a left(\a width).

  Example:
  \code
    QString s("apple");
    QString t = s.leftJustify(8, '.');		// t == "apple..."
  \endcode

  \sa rightJustify()
*/

QString QString::leftJustify( uint width, QChar fill, bool truncate ) const
{
    QString result;
    int len = length();
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.setLength(len+padlen);
	if ( len )
	    memcpy( result.d->unicode, unicode(), sizeof(QChar)*len );
	QChar* uc = result.d->unicode + len;
	while (padlen--)
	    *uc++ = fill;
    } else {
	if ( truncate )
	    result = left( width );
	else
	    result = *this;
    }
    return result;
}

/*!
  Returns a string of length \a width that contains pad
  characters followed by the string.

  If the length of the string exceeds \a width and \a truncate is FALSE,
  then the returned string is a copy of the string.
  If the length of the string exceeds \a width and \a truncate is TRUE,
  then the returned string is a right(\a width).

  Example:
  \code
    QString s("pie");
    QString t = s.rightJustify(8, '.');		// t == ".....pie"
  \endcode

  \sa leftJustify()
*/

QString QString::rightJustify( uint width, QChar fill, bool truncate ) const
{
    QString result;
    int len = length();
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.setLength( len+padlen );
	QChar* uc = result.d->unicode;
	while (padlen--)
	    *uc++ = fill;
	if ( len )
	    memcpy( uc, unicode(), sizeof(QChar)*len );
    } else {
	if ( truncate )
	    result = left( width );
	else
	    result = *this;
    }
    return result;
}

/*!
  Returns a new string that is the string converted to lower case.

  Example:
  \code
    QString s("TeX");
    QString t = s.lower();			// t == "tex"
  \endcode

  \sa upper()
*/

QString QString::lower() const
{
    QString s(*this);
    int l=length();
    s.real_detach(); // could do this only when we find a change
    register QChar *p=s.d->unicode;
    if ( p ) {
	while ( l-- ) {
	    *p = uctolower(*p);
	    p++;
	}
    }
    return s;
}

/*!
  Returns a new string that is the string converted to upper case.

  Example:
  \code
    QString s("TeX");
    QString t = s.upper();			// t == "TEX"
  \endcode

  \sa lower()
*/

QString QString::upper() const
{
    QString s(*this);
    int l=length();
    s.real_detach(); // could do this only when we find a change
    register QChar *p=s.d->unicode;
    if ( p ) {
	while ( l-- ) {
	    *p = uctoupper(*p);
	    p++;
	}
    }
    return s;
}


/*!
  Returns a new string that has white space removed from the start and the end.

  White space means any character for which ucisspace() returns
  TRUE. This includes ASCII characters 9 (TAB), 10 (LF), 11 (VT), 12
  (FF), 13 (CR), and 32 (Space).

  Example:
  \code
    QString s = " space ";
    QString t = s.stripWhiteSpace();		// t == "space"
  \endcode

  \sa simplifyWhiteSpace()
*/

QString QString::stripWhiteSpace() const
{
    if ( isEmpty() )				// nothing to do
	return *this;
    if ( !ucisspace(at(0)) && !ucisspace(at(length()-1)) )
	return *this;

    register const QChar *s = unicode();
    QString result;

    int start = 0;
    int end = length() - 1;
    while ( start<=end && ucisspace(s[start]) )	// skip white space from start
	start++;
    if ( start > end ) {			// only white space
	return result;
    }
    while ( end && ucisspace(s[end]) )		// skip white space from end
	end--;
    int l = end - start + 1;
    result.setLength( l );
    if ( l )
	memcpy( result.d->unicode, &s[start], sizeof(QChar)*l );
    return result;
}


/*!
  Returns a new string that has white space removed from the start and the end,
  plus any sequence of internal white space replaced with a single space
  (ASCII 32).

  White space means any character for which ucisspace() returns
  TRUE. This includes ASCII characters 9 (TAB), 10 (LF), 11 (VT), 12
  (FF), 13 (CR), and 32 (Space).

  \code
    QString s = "  lots\t of\nwhite    space ";
    QString t = s.simplifyWhiteSpace();		// t == "lots of white space"
  \endcode

  \sa stripWhiteSpace()
*/

QString QString::simplifyWhiteSpace() const
{
    if ( isEmpty() )				// nothing to do
	return *this;
    QString result;
    result.setLength( length() );
    const QChar *from = unicode();
    const QChar *fromend = from+length();
    int outc=0;
    QChar *to	= result.d->unicode;
    while ( TRUE ) {
	while ( from!=fromend && ucisspace(*from) )
	    from++;
	while ( from!=fromend && !ucisspace(*from) )
	    to[outc++] = *from++;
	if ( from!=fromend )
	    to[outc++] = ' ';
	else
	    break;
    }
    if ( outc > 0 && to[outc-1] == ' ' )
	outc--;
    result.truncate( outc );
    return result;
}


/*!
  Insert \a s into the string before position \a index.

  If \a index is beyond the end of the string, the string is extended with
  spaces (ASCII 32) to length \a index and \a s is then appended.

  \code
    QString s = "I like fish";
    s.insert( 2, "don't ");			// s == "I don't like fish"
    s = "x";
    s.insert( 3, "yz" );			// s == "x  yz"
  \endcode
*/

QString &QString::insert( uint index, const QString &s )
{
    return insert( index, s.unicode(), s.length() );
}

/*!
  Insert \a len units of QChar data from \a s into the string before
  position \a index.
*/

QString &QString::insert( uint index, const QChar* s, uint len )
{
    if ( len == 0 )
	return *this;
    uint olen = length();
    int nlen = olen + len;

    if ( index >= olen ) {			// insert after end of string
	setLength( nlen+index-olen );
	int n = index-olen;
	QChar* uc = d->unicode+olen;
	while (n--)
	    *uc++ = ' ';
	memcpy( d->unicode+index, s, sizeof(QChar)*len );
    } else {					// normal insert
	setLength( nlen );
	memmove( d->unicode+index+len, unicode()+index,
		 sizeof(QChar)*(olen-index) );
	memcpy( d->unicode+index, s, sizeof(QChar)*len );
    }
    return *this;
}

/*!
  Insert \a c into the string at (before) position \a index and returns
  a reference to the string.

  If \a index is beyond the end of the string, the string is extended with
  spaces (ASCII 32) to length \a index and \a c is then appended.

  Example:
  \code
    QString s = "Ys";
    s.insert( 1, 'e' );				// s == "Yes"
    s.insert( 3, '!');				// s == "Yes!"
  \endcode

  \sa remove(), replace()
*/

QString &QString::insert( uint index, QChar c )	// insert char
{
    QString s( c );
    return insert( index, s );
}

/*!
  \fn QString& QString::insert( uint index, char c )
  \overload
*/

/*!
  \fn QString &QString::prepend( const QString &s )

  Prepend \s to the string. Equivalent to insert(0,s).

  \sa insert()
*/

/*!
  \fn QString& QString::prepend( char ch )
  Prepends \a ch to the string and returns a reference to the result.

  \sa insert()
 */


/*!
  Removes \a len characters starting at position \a index from the
  string and returns a reference to the string.

  If \a index is too big, nothing happens.  If \a index is valid, but
  \a len is too large, the rest of the string is removed.

  \code
    QString s = "Montreal";
    s.remove( 1, 4 );
    // s == "Meal"
  \endcode

  \sa insert(), replace()
*/

QString &QString::remove( uint index, uint len )
{
    uint olen = length();
    if ( index + len >= olen ) {		// range problems
	if ( index < olen ) {			// index ok
	    setLength( index );
	}
    } else if ( len != 0 ) {
	real_detach();
	memmove( d->unicode+index, d->unicode+index+len,
		 sizeof(QChar)*(olen-index-len) );
	setLength( olen-len );
    }
    return *this;
}

/*!
  Replaces \a len characters starting at position \a index from the
  string with \a s, and returns a reference to the string.

  If \a index is too big, nothing is deleted and \a s is inserted at the
  end of the string.  If \a index is valid, but \a len is too large, \e
  str replaces the rest of the string.

  \code
    QString s = "Say yes!";
    s.replace( 4, 3, "NO" );			// s == "Say NO!"
  \endcode

  \sa insert(), remove()
*/

QString &QString::replace( uint index, uint len, const QString &s )
{
    return replace( index, len, s.unicode(), s.length() );
}


/*!
  Replaces \a len characters starting at position \a index by
  \a slen units ot QChar data from \a s, and returns a reference to the string.

  \sa insert(), remove()
*/

QString &QString::replace( uint index, uint len, const QChar* s, uint slen )
{
    if ( len == slen && index + len <= length() ) {
	// Optimized common case
	real_detach();
	memcpy( d->unicode+index, s, len*sizeof(QChar) );
    } else {
	remove( index, len );
	insert( index, s, slen );
    }
    return *this;
}



/*!
  Finds the first occurrence of the regular expression \a rx, starting at
  position \a index.

  Returns the position of the next match, or -1 if \a rx was not found.
*/

int QString::find( const QRegExp &rx, int index ) const
{
    if ( (uint)index >= length() )
	return -1;
    else
	return rx.match( *this, index );
}

/*!
  Finds the first occurrence of the regular expression \a rx, starting at
  position \a index and searching backwards. If \a index is negative,
  the search starts at the end of this string.

  Returns the position of the next match (backwards), or -1 if \a rx was not
  found.
*/

int QString::findRev( const QRegExp &rx, int index ) const
{
    if ( index < 0 )				// neg index ==> start from end
	index = length() - 1;
    else if ( (uint)index >= length() )		// bad index
	return -1;
    while( index >= 0 ) {
	if ( rx.match( *this, index ) == index )
	    return index;
	index--;
    }
    return -1;
}

/*!
  Counts the number of overlapping occurrences of \a rx in the string.

  Example:
  \code
    QString s = "banana and panama";
    QRegExp r = QRegExp("a[nm]a", TRUE, FALSE);
    s.contains( r );				// 4 matches
  \endcode

  \sa find(), findRev()
*/

int QString::contains( const QRegExp &rx ) const
{
    if ( isEmpty() )
	return 0;
    int count = 0;
    int index = -1;
    int len = length();
    while ( index < len-1 ) {			// count overlapping matches
	index = rx.match( *this, index+1 );
	if ( index < 0 )
	    break;
	count++;
    }
    return count;
}


/*!
  Replaces every occurrence of \a rx in the string with \a str.
  Returns a reference to the string.

  Examples:
  \code
    QString s = "banana";
    s.replace( QRegExp("a.*a"), "" );		// becomes "b"

    QString s = "banana";
    s.replace( QRegExp("^[bn]a"), " " );	// becomes " nana"

    QString s = "banana";
    s.replace( QRegExp("^[bn]a"), "" );		// NOTE! becomes ""
  \endcode

*/

QString &QString::replace( const QRegExp &rx, const QString &str )
{
    if ( isEmpty() )
	return *this;
    int index = 0;
    int slen  = str.length();
    int len;
    while ( index < (int)length()-1 ) {
	index = rx.match( *this, index, &len, FALSE );
	if ( index >= 0 ) {
	    replace( index, len, str );
	    index += slen;
	    if ( !len )
		break;	// Avoid infinite loop on 0-length matches, e.g. [a-z]*
	}
	else
	    break;
    }
    return *this;
}

/*!
  Returns the string converted to a <code>long</code> value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
*/

long QString::toLong( bool *ok ) const
{
    const QChar *p = unicode();
    long val=0;
    int l = length();
    const long max_mult = 214748364;
    bool is_ok = FALSE;
    int neg = 0;
    if ( !p )
	goto bye;
    while ( l && ucisspace(*p) )			// skip leading space
	l--,p++;
    if ( l && *p == '-' ) {
	l--;
	p++;
	neg = 1;
    } else if ( *p == '+' ) {
	l--;
	p++;
    }
    if ( !l || !ucisdigit(*p) )
	goto bye;
    while ( l && ucisdigit(*p) ) {
	l--;
	if ( val > max_mult || (val == max_mult && (*p-'0') > 7+neg) )
	    goto bye;
	val = 10*val + (*p++ - '0');
    }
    if ( neg )
	val = -val;
    while ( l && ucisspace(*p) )			// skip trailing space
	l--,p++;
    if ( !l )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
  Returns the string converted to an <code>unsigned long</code>
  value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

ulong QString::toULong( bool *ok ) const
{
    const QChar *p = unicode();
    ulong val=0;
    int l = length();
    const ulong max_mult = 429496729;
    bool is_ok = FALSE;
    if ( !p )
	goto bye;
    while ( l && ucisspace(*p) )			// skip leading space
	l--,p++;
    if ( *p == '+' )
	l--,p++;
    if ( !l || !ucisdigit(*p) )
	goto bye;
    while ( l && ucisdigit(*p) ) {
	l--;
	if ( val > max_mult || (val == max_mult && (*p-'0') > 5) )
	    goto bye;
	val = 10*val + (*p++ - '0');
    }
    while ( l && ucisspace(*p) )			// skip trailing space
	l--,p++;
    if ( !l )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
  Returns the string converted to a <code>short</code> value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
*/

short QString::toShort( bool *ok ) const
{
    long v = toLong( ok );
    if ( ok && *ok && (v < -32768 || v > 32767) ) {
	*ok = FALSE;
	v = 0;
    }
    return (short)v;
}

/*!
  Returns the string converted to an <code>unsigned short</code> value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
*/

ushort QString::toUShort( bool *ok ) const
{
    ulong v = toULong( ok );
    if ( ok && *ok && (v > 65535) ) {
	*ok = FALSE;
	v = 0;
    }
    return (ushort)v;
}


/*!
  Returns the string converted to a <code>int</code> value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

int QString::toInt( bool *ok ) const
{
    return (int)toLong( ok );
}

/*!
  Returns the string converted to an <code>unsigned int</code> value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

uint QString::toUInt( bool *ok ) const
{
    return (uint)toULong( ok );
}

/*!
  Returns the string converted to a <code>double</code> value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no conceivable
  errors, and FALSE if the string is not a number at all, or if it has
  trailing garbage.
*/

double QString::toDouble( bool *ok ) const
{
    char *end;
    const char *a = ascii();
    double val = strtod( a ? a : "", &end );
    if ( ok )
	*ok = ( a && *a && ( end == 0 || *end == '\0' ) );
    return val;
}

/*!
  Returns the string converted to a <code>float</code> value.

  If \a ok is non-null, \a *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

float QString::toFloat( bool *ok ) const
{
    return (float)toDouble( ok );
}


/*!
  Sets the string to the printed value of \a n and returns a
  reference to the string.

  The value is converted to \a base notation (default is decimal).
  The base must be a value from 2 to 36.
*/

QString &QString::setNum( long n, int base )
{
#if defined(CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	warning( "QString::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    char buf[65];
    register char *p = &buf[64];
    bool neg;
    if ( n < 0 ) {
	neg = TRUE;
	n = -n;
    } else {
	neg = FALSE;
    }
    *p = '\0';
    do {
	*--p = "0123456789abcdefghijklmnopqrstuvwxyz"[((int)(n%base))];
	n /= base;
    } while ( n );
    if ( neg )
	*--p = '-';
    return *this = p;
}

/*!
  Sets the string to the printed unsigned value of \a n and
  returns a reference to the string.

  The value is converted to \a base notation (default is decimal).
  The base must be a value from 2 to 36.
*/

QString &QString::setNum( ulong n, int base )
{
#if defined(CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	warning( "QString::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    char buf[65];
    register char *p = &buf[64];
    *p = '\0';
    do {
	*--p = "0123456789abcdefghijklmnopqrstuvwxyz"[((int)(n%base))];
	n /= base;
    } while ( n );
    return *this = p;
}

/*!
  \fn QString &QString::setNum( int n, int base=10 )
  Sets the string to the printed value of \a n and returns a reference
  to the string.
*/

/*!
  \fn QString &QString::setNum( uint n, int base=10 )
  Sets the string to the printed unsigned value of \a n and returns a
  reference to the string.
*/

/*!
  \fn QString &QString::setNum( short n, int base=10 )
  Sets the string to the printed value of \a n and returns a reference
  to the string.
*/

/*!
  \fn QString &QString::setNum( ushort n, int base=10 )
  Sets the string to the printed unsigned value of \a n and returns a
  reference to the string.
*/

/*!
  Sets the string to the printed value of \a n.

  \arg \a f is the format specifier: 'f', 'F', 'e', 'E', 'g', 'G' (same
  as sprintf()).
  \arg \a prec is the precision.

  Returns a reference to the string.
*/

QString &QString::setNum( double n, char f, int prec )
{
#if defined(CHECK_RANGE)
    if ( !(f=='f' || f=='F' || f=='e' || f=='E' || f=='g' || f=='G') ) {
	warning( "QString::setNum: Invalid format char '%c'", f );
	f = 'f';
    }
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

/*!
  \fn QString &QString::setNum( float n, char f, int prec )
  Sets the string to the printed value of \a n.

  \arg \a f is the format specifier: 'f', 'F', 'e', 'E', 'g', 'G' (same
  as sprintf()).
  \arg \a prec is the precision.

  Returns a reference to the string.
*/


/*!
  \obsolete

  Sets the character at position \a index to \a c and expands the
  string if necessary, filling with spaces.

  This is never necessary.
*/
void QString::setExpand( uint index, QChar c )
{
    int spaces = index - d->len;
    at(index) = c;
    while (spaces-->0)
	d->unicode[--index]=' ';
}


/*!
  \fn const char* QString::data() const

  Obsolete.  This method is provided to aide porting to Qt 2.0.

  In Qt 1.x, this returned a char* allowing direct manipulation of the
  string as a sequence of bytes.  Now that QString is a Unicode
  string, char* conversion constructs a temporary string, and hence
  direct character operations are meaningless.
*/

/*!
  \fn bool QString::operator!() const
  Returns TRUE if it is a null string, otherwise FALSE.
*/


/*!
  \fn QString& QString::append( const QString& str )
  Appends \a str to the string and returns a reference to the result.
  Equivalent to operator+=().
 */

/*!
  \fn QString& QString::append( char ch )
  Appends \a ch to the string and returns a reference to the result.
  Equivalent to operator+=().
 */

/*!
  Appends \a str to the string and returns a reference to the string.
*/
QString& QString::operator+=( const QString &str )
{
    uint len1 = length();
    uint len2 = str.length();
    if ( len2 ) {
	setLength(len1+len2);
	memcpy( d->unicode+len1, str.unicode(), sizeof(QChar)*len2 );
    }
    return *this;
}

/*!
  Appends \a c to the string and returns a reference to the string.
*/

QString &QString::operator+=( QChar c )
{
    setLength(length()+1);
    d->unicode[length()-1] = c;
    return *this;
}

/*!
  Appends \a c to the string and returns a reference to the string.
*/

QString &QString::operator+=( char c )
{
    setLength(length()+1);
    d->unicode[length()-1] = c;
    return *this;
}

/*!
  Result remains valid so long as one unmodified
  copy of the string exists, or the function is not called again.
*/
const char* QString::ascii() const
{
    if ( d->ascii ) {
	if ( d->dirtyascii )
	    delete [] d->ascii;
	else
	    return d->ascii;
    }
    Q2HELPER(stat_get_ascii++);
    Q2HELPER(stat_get_ascii_size+=d->len);
    d->ascii = unicodeToAscii( d->unicode, d->len );
    // Q2HELPER(ASSERT( strlen(d->ascii) == d->len ));
    d->dirtyascii = 0;
    return d->ascii;
}

/*!
  Returns the string encoded in UTF8 format.

  See QTextCodec for more diverse coding/decoding of Unicode strings.
*/
QCString QString::utf8() const
{
    static QTextCodec* codec = QTextCodec::codecForMib(106);
    return codec
	    ? codec->fromUnicode(*this)
	    : QCString(ascii());
}

/*!
  Returns the unicode string decoded from the
  first \a len bytes of \a utf8.  If \a len is -1 (the default), the
  length of \a utf8 is used.  If trailing partial characters are in
  \a utf8, they are ignored.

  See QTextCodec for more diverse coding/decoding of Unicode strings.
*/
QString QString::fromUtf8(const char* utf8, int len)
{
    static QTextCodec* codec = QTextCodec::codecForMib(106);
    return codec
	    ? codec->toUnicode(utf8, len < 0 ? strlen(utf8) : len)
	    : QString(utf8);
}

/*!
  \fn const QChar* QString::unicode() const

  Returns the Unicode representation of the string.  The result
  remains valid until the string is modified.
*/

/*!
  \fn QString::operator const char *() const

  Returns ascii().
*/

/*!
  \fn const QChar& QString::at( uint ) const

  Returns the character at \a i, or 0 if \a i is beyond the length
  of the string.
*/

/*!
  \fn const QChar& QString::operator[](int) const

  Returns the character at \a i, or 0 if \a i is beyond the length
  of the string.
*/

/*!
  \fn QChar& QString::operator[](int)

  Returns a reference to the character at \a i, expanding
  the string with character-0 if necessary.  The resulting reference
  can then be assigned to, or otherwise used immediately, but
  becomes invalid once further modifications are made to the string.
*/

/*!
  \fn QChar& QString::at( uint i )
  Returns a reference to the character at \a i, expanding
  the string with character-0 if necessary.  The resulting reference
  can then be assigned to, or otherwise used immediately, but
  becomes invalid once further modifications are made to the string.
*/

/*!
  Internal chunk of code to handle the
  uncommon cases of at() above.
*/
void QString::subat( uint i )
{
    uint olen = d->len;
    if ( i >= olen ) {
	setLength( i+1 );		// i is index; i+1 is needed length
	for ( uint j=olen; j<=i; j++ )
	    d->unicode[j]='\0';
    } else {
	// Just be sure to detach
	real_detach();
    }
}


/*****************************************************************************
  QString stream functions
 *****************************************************************************/

/*!
  \relates QString
  Writes a string to the stream.

  Output format: [length (Q_UINT32) data...]
*/

QDataStream &operator<<( QDataStream &s, const QString &str )
{
    return s.writeBytes( (const char*)str.unicode(),
			 sizeof(QChar)*str.length() );
}

/*!
  \relates QString
  Reads a string from the stream.
*/

QDataStream &operator>>( QDataStream &s, QString &str )
{
    Q_UINT32 bytes;
    s >> bytes;					// read size of string
    str.setLength( bytes/2 );
    if ( bytes > 0 )				// not null array
	s.readRawBytes( (char*)str.d->unicode, bytes );
    return s;
}

/*!
  \fn int QString::compare (const QString & s1, const QString & s2)

  Compare \a s1 to \a s2 returning an integer less than, equal to, or
  greater than zero if s1 is, respectively, lexically less than, equal to,
  or greater than s2.
*/

/*!
  Compares this string to \a s, returning an integer less than, equal to, or
  greater than zero if it is, respectively, lexically less than, equal to,
  or greater than \a s.

*/
int QString::compare( const QString& s ) const
{
    return ucstrcmp(*this,s);
}

bool operator==( const QString &s1, const QString &s2 )
{ return s1.length()==s2.length() && ucstrcmp(s1,s2) == 0; }

bool operator==( const QString &s1, const char *s2 )
{ return s1==QString(s2); }

bool operator==( const char *s1, const QString &s2 )
{ return QString(s1)==s2; }

bool operator!=( const QString &s1, const QString &s2 )
{ return !(s1==s2); }

bool operator!=( const QString &s1, const char *s2 )
{ return !(s1==s2); }

bool operator!=( const char *s1, const QString &s2 )
{ return !(s1==s2); }

bool operator<( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) < 0; }

bool operator<( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) < 0; }

bool operator<( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) < 0; }

bool operator<=( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) <= 0; }

bool operator<=( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) <= 0; }

bool operator<=( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) <= 0; }

bool operator>( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) > 0; }

bool operator>( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) > 0; }

bool operator>( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) > 0; }

bool operator>=( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) >= 0; }

bool operator>=( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) >= 0; }

bool operator>=( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) >= 0; }

/*****************************************************************************
  Documentation for related functions
 *****************************************************************************/

/*!
  \fn bool operator==( const QString &s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are equal, or FALSE if they are different.

  Equivalent to <code>strcmp(s1,s2) == 0</code>.
*/

/*!
  \fn bool operator==( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if the two strings are equal, or FALSE if they are different.

  Equivalent to <code>strcmp(s1,s2) == 0</code>.
*/

/*!
  \fn bool operator==( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are equal, or FALSE if they are different.

  Equivalent to <code>strcmp(s1,s2) == 0</code>.
*/

/*!
  \fn bool operator!=( const QString &s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are different, or FALSE if they are equal.

  Equivalent to <code>strcmp(s1,s2) != 0</code>.
*/

/*!
  \fn bool operator!=( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if the two strings are different, or FALSE if they are equal.

  Equivalent to <code>strcmp(s1,s2) != 0</code>.
*/

/*!
  \fn bool operator!=( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are different, or FALSE if they are equal.

  Equivalent to <code>strcmp(s1,s2) != 0</code>.
*/

/*!
  \fn bool operator<( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically less than \a s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \< 0</code>.
*/

/*!
  \fn bool operator<( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically less than \a s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \< 0</code>.
*/

/*!
  \fn bool operator<=( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically less than or equal to \a s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \<= 0</code>.
*/

/*!
  \fn bool operator<=( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically less than or equal to \a s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \<= 0</code>.
*/

/*!
  \fn bool operator>( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically greater than \a s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \> 0</code>.
*/

/*!
  \fn bool operator>( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically greater than \a s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \> 0</code>.
*/

/*!
  \fn bool operator>=( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically greater than or equal to \a s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \>= 0</code>.
*/

/*!
  \fn bool operator>=( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \a s1 is alphabetically greater than or equal to \a s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \>= 0</code>.
*/

/*!
  \fn QString operator+( const QString &s1, const QString &s2 )
  \relates QString
  Returns the concatenated string of s1 and s2.
*/

/*!
  \fn QString operator+( const QString &s1, const char *s2 )
  \relates QString
  Returns the concatenated string of s1 and s2.
*/

/*!
  \fn QString operator+( const char *s1, const QString &s2 )
  \relates QString
  Returns the concatenated string of s1 and s2.
*/

/*!
  \fn QString operator+( const QString &s, char c )
  \relates QString
  Returns the concatenated string of s and c.
*/

/*!
  \fn QString operator+( char c, const QString &s )
  \relates QString
  Returns the concatenated string of c and s.
*/



/*****************************************************************************
  QConstString member functions
 *****************************************************************************/

/*!
  \class QConstString qstring.h
  \brief A QString which uses constant Unicode data.

  In order to minimize copying, highly optimized applications
  can use QConstString to provide a QString-compatible object
  from existing Unicode data.  The Unicode data must exist
  for the entire lifetime of the QConstString object.
*/

/*!
  Creates a QConstString that uses the first \a length Unicode
  characters in the array \a unicode.  Any attempt to modify
  copies of the string will cause it to create a copy of the
  data, thus it remains forever unmodified.
*/
QConstString::QConstString( QChar* unicode, uint length ) :
    QString(new Data(unicode, length, length))
{
}

/*!
  Destroys the QConstString, creating a copy of the data if
  other strings are still using it.
*/
QConstString::~QConstString()
{
    if ( d->count > 1 ) {
        QChar* cp = new QChar[d->len];
        memcpy( cp, d->unicode, d->len*sizeof(QChar) );
        d->unicode = cp;
    } else {
        d->unicode = 0;
    }

    // The original d->unicode is now unlinked.
}

/*!
  \fn const QString& QConstString::string() const

  Returns a constant string referencing the data passed during
  construction.
*/


#if defined(_OS_WIN32_)

#include <windows.h>

/*!
  Returns a static Windows TCHAR* from a QString, possibly adding NUL.
*/
const void* qt_winTchar(const QString& str, bool addnul)
{
#ifdef UNICODE
    static uint buflen = 256;
    static TCHAR *buf = new TCHAR[buflen];

    const QChar* uc = str.unicode();

#define EXTEND if (str.length() > buflen) { delete buf; buf = new TCHAR[buflen=str.length()+1]; }

#if defined(_WS_X11_) || defined(_OS_WIN32_BYTESWAP_)
    EXTEND
    for ( int i=str.length(); i--; )
	buf[i] = uc[i].row << 8 | uc[i].cell;
    if ( addnul )
	buf[str.length()] = 0;
#else
    // Same endianness of TCHAR
    if ( addnul ) {
	EXTEND
	memcpy(buf,uc,sizeof(TCHAR)*str.length());
	buf[str.length()] = 0;
    } else {
	return uc;
    }
#endif
    return buf;
#undef EXTEND

#else
    return str.ascii();
#endif
}

/*!
  Makes a new null terminated Windows TCHAR* from a QString.
*/
void* qt_winTchar_new(const QString& str)
{
    TCHAR* result = new TCHAR[str.length()+1];
    memcpy(result, qt_winTchar(str,FALSE), sizeof(TCHAR)*str.length());
    result[str.length()] = 0;
    return result;
}

/*!
  Makes a QString from a Windows TCHAR*.
*/
QString qt_winQString(void* tc)
{
#ifdef UNICODE

    int len=0;
    while ( ((TCHAR*)tc)[len] )
	len++;
#if defined(_WS_X11_) || defined(_OS_WIN32_BYTESWAP_)
    QString r;
    for ( int i=0; i<len; i++ )
	r += QChar(((TCHAR*)tc)[i]&0xff,((TCHAR*)tc)[i]>>8);
    return r;
#else
    // Same endianness of TCHAR
    return QString((QChar*)tc,len);
#endif
#undef EXTEND
#else
    return (TCHAR*)tc;
#endif
}

#endif // _OS_WIN32_


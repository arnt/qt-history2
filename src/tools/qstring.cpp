/****************************************************************************
** $Id$
**
** Implementation of the QString class and related Unicode functions
**
** Created : 920722
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// Don't define it while compiling this module, or USERS of Qt will
// not be able to link.
#ifdef QT_NO_CAST_ASCII
#undef QT_NO_CAST_ASCII
#endif

#include "qstring.h"
#include "qregexp.h"
#include "qdatastream.h"
#ifndef QT_NO_TEXTCODEC
#include "qtextcodec.h"
#endif

#include "qunicodetables_p.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif
#if !defined( QT_NO_COMPONENT ) && !defined( QT_LITE_COMPONENT )
#include "qcleanuphandler.h"
#endif

static int ucstrcmp( const QString &as, const QString &bs )
{
    const QChar *a = as.unicode();
    const QChar *b = bs.unicode();
    if ( a == b )
	return 0;
    if ( a == 0 )
	return 1;
    if ( b == 0 )
	return -1;
    int l=QMIN(as.length(),bs.length());
    while ( l-- && *a == *b )
	a++,b++;
    if ( l==-1 )
	return ( as.length()-bs.length() );
    return a->unicode() - b->unicode();
}

static int ucstrncmp( const QChar *a, const QChar *b, int l )
{
    while ( l-- && *a == *b )
	a++,b++;
    if ( l==-1 )
	return 0;
    return a->unicode() - b->unicode();
}

static int ucstrnicmp( const QChar *a, const QChar *b, int l )
{
    while ( l-- && ::lower( *a ) == ::lower( *b ) )
	a++,b++;
    if ( l==-1 )
	return 0;
    return ::lower( *a ).unicode() - ::lower( *b ).unicode();
}

static uint computeNewMax( uint len )
{
    uint newMax = 4;
    while ( newMax < len )
	newMax *= 2;
    // try to save some memory
    if ( newMax >= 1024 * 1024 && len <= newMax - (newMax >> 2) )
	newMax -= newMax >> 2;
    return newMax;
}

/*!
    \class QCharRef qstring.h
    \reentrant
    \brief The QCharRef class is a helper class for QString.

    \ingroup text

    When you get an object of type QCharRef, if you can assign to it,
    the assignment will apply to the character in the string from
    which you got the reference. That is its whole purpose in life.
    The QCharRef becomes invalid once modifications are made to the
    string: if you want to keep the character, copy it into a QChar.

    Most of the QChar member functions also exist in QCharRef.
    However, they are not explicitly documented here.

    \sa QString::operator[]() QString::at() QChar
*/

/*!
    \class QChar qstring.h
    \reentrant
    \brief The QChar class provides a lightweight Unicode character.

    \ingroup text

    Unicode characters are (so far) 16-bit entities without any markup
    or structure. This class represents such an entity. It is
    lightweight, so it can be used everywhere. Most compilers treat it
    like a "short int".  (In a few years it may be necessary to make
    QChar 32-bit when more than 65536 Unicode code points have been
    defined and come into use.)

    QChar provides a full complement of testing/classification
    functions, converting to and from other formats, converting from
    composed to decomposed Unicode, and trying to compare and
    case-convert if you ask it to.

    The classification functions include functions like those in
    ctype.h, but operating on the full range of Unicode characters.
    They all return TRUE if the character is a certain type of
    character; otherwise they return FALSE. These classification
    functions are isNull() (returns TRUE if the character is U+0000),
    isPrint() (TRUE if the character is any sort of printable
    character, including whitespace), isPunct() (any sort of
    punctation), isMark() (Unicode Mark), isLetter (a letter),
    isNumber() (any sort of numeric character), isLetterOrNumber(),
    and isDigit() (decimal digits). All of these are wrappers around
    category() which return the Unicode-defined category of each
    character.

    QChar further provides direction(), which indicates the "natural"
    writing direction of this character. The joining() function
    indicates how the character joins with its neighbors (needed
    mostly for Arabic) and finally mirrored(), which indicates whether
    the character needs to be mirrored when it is printed in its
    "unnatural" writing direction.

    Composed Unicode characters (like &aring;) can be converted to
    decomposed Unicode ("a" followed by "ring above") by using
    decomposition().

    In Unicode, comparison is not necessarily possible and case
    conversion is very difficult at best. Unicode, covering the
    "entire" world, also includes most of the world's case and sorting
    problems. Qt tries, but not very hard: operator==() and friends
    will do comparison based purely on the numeric Unicode value (code
    point) of the characters, and upper() and lower() will do case
    changes when the character has a well-defined upper/lower-case
    equivalent. There is no provision for locale-dependent case
    folding rules or comparison; these functions are meant to be fast
    so they can be used unambiguously in data structures. (See
    QString::localeAwareCompare() though.)

    The conversion functions include unicode() (to a scalar), latin1()
    (to scalar, but converts all non-Latin1 characters to 0), row()
    (gives the Unicode row), cell() (gives the Unicode cell),
    digitValue() (gives the integer value of any of the numerous digit
    characters), and a host of constructors.

    More information can be found in the document \link unicode.html
    About Unicode. \endlink

    \sa QString QCharRef
*/

/*!
    \enum QChar::Category

    This enum maps the Unicode character categories.

    The following characters are normative in Unicode:

    \value Mark_NonSpacing  Unicode class name Mn

    \value Mark_SpacingCombining  Unicode class name Mc

    \value Mark_Enclosing  Unicode class name Me

    \value Number_DecimalDigit  Unicode class name Nd

    \value Number_Letter  Unicode class name Nl

    \value Number_Other  Unicode class name No

    \value Separator_Space  Unicode class name Zs

    \value Separator_Line  Unicode class name Zl

    \value Separator_Paragraph  Unicode class name Zp

    \value Other_Control  Unicode class name Cc

    \value Other_Format  Unicode class name Cf

    \value Other_Surrogate  Unicode class name Cs

    \value Other_PrivateUse  Unicode class name Co

    \value Other_NotAssigned  Unicode class name Cn


    The following categories are informative in Unicode:

    \value Letter_Uppercase  Unicode class name Lu

    \value Letter_Lowercase  Unicode class name Ll

    \value Letter_Titlecase  Unicode class name Lt

    \value Letter_Modifier  Unicode class name Lm

    \value Letter_Other Unicode class name Lo

    \value Punctuation_Connector  Unicode class name Pc

    \value Punctuation_Dash  Unicode class name Pd

    \value Punctuation_Open  Unicode class name Ps

    \value Punctuation_Close  Unicode class name Pe

    \value Punctuation_InitialQuote  Unicode class name Pi

    \value Punctuation_FinalQuote  Unicode class name Pf

    \value Punctuation_Other  Unicode class name Po

    \value Symbol_Math  Unicode class name Sm

    \value Symbol_Currency  Unicode class name Sc

    \value Symbol_Modifier  Unicode class name Sk

    \value Symbol_Other  Unicode class name So


    There are two categories that are specific to Qt:

    \value NoCategory  used when Qt is dazed and confused and cannot
    make sense of anything.

    \value Punctuation_Dask  old typo alias for Punctuation_Dash

*/

/*!
    \enum QChar::Direction

    This enum type defines the Unicode direction attributes. See \link
    http://www.unicode.org/ the Unicode Standard\endlink for a
    description of the values.

    In order to conform to C/C++ naming conventions "Dir" is prepended
    to the codes used in the Unicode Standard.
*/

/*!
    \enum QChar::Decomposition

    This enum type defines the Unicode decomposition attributes. See
    \link http://www.unicode.org/ the Unicode Standard\endlink for a
    description of the values.
*/

/*!
    \enum QChar::Joining

    This enum type defines the Unicode joining attributes. See \link
    http://www.unicode.org/ the Unicode Standard\endlink for a
    description of the values.
*/

/*!
    \enum QChar::CombiningClass

    This enum type defines names for some of the Unicode combining
    classes. See \link http://www.unicode.org/ the Unicode
    Standard\endlink for a description of the values.
*/

/*!
    \fn void QChar::setCell( uchar cell )
    \internal
*/

/*!
    \fn void QChar::setRow( uchar row )
    \internal
*/


/*!
    \fn QChar::QChar()

    Constructs a null QChar (one that isNull()).
*/


/*!
    \fn QChar::QChar( char c )

    Constructs a QChar corresponding to ASCII/Latin1 character \a c.
*/


/*!
    \fn QChar::QChar( uchar c )

    Constructs a QChar corresponding to ASCII/Latin1 character \a c.
*/


/*!
    \fn QChar::QChar( uchar c, uchar r )

    Constructs a QChar for Unicode cell \a c in row \a r.
*/


/*!
    \fn QChar::QChar( const QChar& c )

    Constructs a copy of \a c. This is a deep copy, if such a
    lightweight object can be said to have deep copies.
*/


/*!
    \fn QChar::QChar( ushort rc )

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn QChar::QChar( short rc )

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn QChar::QChar( uint rc )

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn QChar::QChar( int rc )

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn bool  QChar::networkOrdered ()

    \obsolete

    Returns TRUE if this character is in network byte order (MSB
    first); otherwise returns FALSE. This is platform dependent.
*/


/*!
    \fn bool QChar::isNull() const

    Returns TRUE if the character is the Unicode character 0x0000,
    i.e. ASCII NUL; otherwise returns FALSE.
*/

/*!
    \fn uchar QChar::cell () const

    Returns the cell (least significant byte) of the Unicode
    character.
*/

/*!
    \fn uchar QChar::row () const

    Returns the row (most significant byte) of the Unicode character.
*/

/*!
    Returns TRUE if the character is a printable character; otherwise
    returns FALSE. This is any character not of category Cc or Cn.

    Note that this gives no indication of whether the character is
    available in a particular \link QFont font\endlink.
*/
bool QChar::isPrint() const
{
    Category c = ::category( *this );
    return !(c == Other_Control || c == Other_NotAssigned);
}

/*!
    Returns TRUE if the character is a separator character
    (Separator_* categories); otherwise returns FALSE.
*/
bool QChar::isSpace() const
{
    if( ucs >= 9 && ucs <=13 ) return TRUE;
    Category c = ::category( *this );
    return c >= Separator_Space && c <= Separator_Paragraph;
}

/*!
    Returns TRUE if the character is a mark (Mark_* categories);
    otherwise returns FALSE.
*/
bool QChar::isMark() const
{
    Category c = ::category( *this );
    return c >= Mark_NonSpacing && c <= Mark_Enclosing;
}

/*!
    Returns TRUE if the character is a punctuation mark (Punctuation_*
    categories); otherwise returns FALSE.
*/
bool QChar::isPunct() const
{
    Category c = ::category( *this );
    return (c >= Punctuation_Connector && c <= Punctuation_Other);
}

/*!
    Returns TRUE if the character is a letter (Letter_* categories);
    otherwise returns FALSE.
*/
bool QChar::isLetter() const
{
    Category c = ::category( *this );
    return (c >= Letter_Uppercase && c <= Letter_Other);
}

/*!
    Returns TRUE if the character is a number (of any sort - Number_*
    categories); otherwise returns FALSE.

    \sa isDigit()
*/
bool QChar::isNumber() const
{
    Category c = ::category( *this );
    return c >= Number_DecimalDigit && c <= Number_Other;
}

/*!
    Returns TRUE if the character is a letter or number (Letter_* or
    Number_* categories); otherwise returns FALSE.
*/
bool QChar::isLetterOrNumber() const
{
    Category c = ::category( *this );
    return (c >= Letter_Uppercase && c <= Letter_Other)
	|| (c >= Number_DecimalDigit && c <= Number_Other);
}


/*!
    Returns TRUE if the character is a decimal digit
    (Number_DecimalDigit); otherwise returns FALSE.
*/
bool QChar::isDigit() const
{
    return (::category( *this ) == Number_DecimalDigit);
}


/*!
    Returns TRUE if the character is a symbol (Symbol_* categories);
    otherwise returns FALSE.
*/
bool QChar::isSymbol() const
{
    Category c = ::category( *this );
    return c >= Symbol_Math && c <= Symbol_Other;
}

/*!
    Returns the numeric value of the digit, or -1 if the character is
    not a digit.
*/
int QChar::digitValue() const
{
#ifndef QT_NO_UNICODETABLES
    const Q_INT8 *dec_row = QUnicodeTables::decimal_info[row()];
    if( !dec_row )
	return -1;
    return dec_row[cell()];
#else
    // ##### just latin1
    if ( ucs < '0' || ucs > '9' )
	return -1;
    else
	return ucs - '0';
#endif
}

/*!
    Returns the character category.

    \sa Category
*/
QChar::Category QChar::category() const
{
     return ::category( *this );
}

/*!
    Returns the character's direction.

    \sa Direction
*/
QChar::Direction QChar::direction() const
{
     return ::direction( *this );
}

/*!
    \warning This function is not supported (it may change to use
    Unicode character classes).

    Returns information about the joining properties of the character
    (needed for example, for Arabic).
*/
QChar::Joining QChar::joining() const
{
    return ::joining( *this );
}


/*!
    Returns TRUE if the character is a mirrored character (one that
    should be reversed if the text direction is reversed); otherwise
    returns FALSE.
*/
bool QChar::mirrored() const
{
    return ::mirrored( *this );
}

/*!
    Returns the mirrored character if this character is a mirrored
    character, otherwise returns the character itself.
*/
QChar QChar::mirroredChar() const
{
    return ::mirroredChar( *this );
}

#ifndef QT_NO_UNICODETABLES
// ### REMOVE ME 4.0
static QString shared_decomp;
#endif
/*!
    \nonreentrant

    Decomposes a character into its parts. Returns QString::null if no
    decomposition exists.
*/
const QString &QChar::decomposition() const
{
#ifndef QT_NO_UNICODETABLES
    const Q_UINT16 *r = QUnicodeTables::decomposition_info[row()];
    if(!r) return QString::null;

    Q_UINT16 pos = r[cell()];
    if(!pos) return QString::null;
    pos+=2;

    QString s;
    Q_UINT16 c;
    while((c = QUnicodeTables::decomposition_map[pos++]) != 0) s += QChar(c);
    // ### In 4.0, return s, and not shared_decomp.  shared_decomp
    // prevents this function from being reentrant.
    shared_decomp = s;
    return shared_decomp;
#else
    return QString::null;
#endif
}

/*!
    Returns the tag defining the composition of the character. Returns
    QChar::Single if no decomposition exists.
*/
QChar::Decomposition QChar::decompositionTag() const
{
#ifndef QT_NO_UNICODETABLES
    const Q_UINT16 *r = QUnicodeTables::decomposition_info[row()];
    if(!r) return QChar::Single;

    Q_UINT16 pos = r[cell()];
    if(!pos) return QChar::Single;

    return (QChar::Decomposition) QUnicodeTables::decomposition_map[pos];
#else
    return Single; // ########### FIX eg. just latin1
#endif
}

/*!
    Returns the combining class for the character as defined in the
    Unicode standard. This is mainly useful as a positioning hint for
    marks attached to a base character.

    The Qt text rendering engine uses this information to correctly
    position non spacing marks around a base character.
*/
unsigned char QChar::combiningClass() const
{
    return ::combiningClass( *this );
}


/*!
    Returns the lowercase equivalent if the character is uppercase;
    otherwise returns the character itself.
*/
QChar QChar::lower() const
{
     return ::lower( *this );
}

/*!
    Returns the uppercase equivalent if the character is lowercase;
    otherwise returns the character itself.
*/
QChar QChar::upper() const
{
     return ::upper( *this );
}

/*!
    \fn QChar::operator char() const

    Returns the Latin1 character equivalent to the QChar, or 0. This
    is mainly useful for non-internationalized software.

    \sa unicode()
*/

/*!
    \fn ushort QChar::unicode() const

    Returns the numeric Unicode value equal to the QChar. Normally,
    you should use QChar objects as they are equivalent, but for some
    low-level tasks (e.g. indexing into an array of Unicode
    information), this function is useful.
*/

/*!
    \fn ushort & QChar::unicode()

    \overload

    Returns a reference to the numeric Unicode value equal to the
    QChar.
*/

/*****************************************************************************
  Documentation of QChar related functions
 *****************************************************************************/

/*!
    \fn bool operator==( QChar c1, QChar c2 )

    \relates QChar

    Returns TRUE if \a c1 and \a c2 are the same Unicode character;
    otherwise returns FALSE.
*/

/*!
    \fn bool operator==( char ch, QChar c )

    \overload
    \relates QChar

    Returns TRUE if \a c is the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn bool operator==( QChar c, char ch )

    \overload
    \relates QChar

    Returns TRUE if \a c is the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn int operator!=( QChar c1, QChar c2 )

    \relates QChar

    Returns TRUE if \a c1 and \a c2 are not the same Unicode
    character; otherwise returns FALSE.
*/

/*!
    \fn int operator!=( char ch, QChar c )

    \overload
    \relates QChar

    Returns TRUE if \a c is not the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn int operator!=( QChar c, char ch )

    \overload
    \relates QChar

    Returns TRUE if \a c is not the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn int operator<=( QChar c1, QChar c2 )

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is less than
    that of \a c2, or they are the same Unicode character; otherwise
    returns FALSE.
*/

/*!
    \fn int operator<=( QChar c, char ch )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is less than or
    equal to that of the ASCII/Latin1 character \a ch; otherwise
    returns FALSE.
*/

/*!
    \fn int operator<=( char ch, QChar c )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is less than or equal to that of \a c; otherwise
    returns FALSE.
*/

/*!
    \fn int operator>=( QChar c1, QChar c2 )

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is greater than
    that of \a c2, or they are the same Unicode character; otherwise
    returns FALSE.
*/

/*!
    \fn int operator>=( QChar c, char ch )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is greater than
    or equal to that of the ASCII/Latin1 character \a ch; otherwise
    returns FALSE.
*/

/*!
    \fn int operator>=( char ch, QChar c )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is greater than or equal to that of \a c;
    otherwise returns FALSE.
*/

/*!
    \fn int operator<( QChar c1, QChar c2 )

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is less than
    that of \a c2; otherwise returns FALSE.
*/

/*!
    \fn int operator<( QChar c, char ch )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is less than that
    of the ASCII/Latin1 character \a ch; otherwise returns FALSE.
*/

/*!
    \fn int operator<( char ch, QChar c )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is less than that of \a c; otherwise returns
    FALSE.
*/

/*!
    \fn int operator>( QChar c1, QChar c2 )

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is greater than
    that of \a c2; otherwise returns FALSE.
*/

/*!
    \fn int operator>( QChar c, char ch )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is greater than
    that of the ASCII/Latin1 character \a ch; otherwise returns FALSE.
*/

/*!
    \fn int operator>( char ch, QChar c )

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is greater than that of \a c; otherwise returns
    FALSE.
*/

#ifndef QT_NO_UNICODETABLES

// small class used internally in QString::Compose()
class QLigature
{
public:
    QLigature( QChar c );

    Q_UINT16 first() { cur = ligatures; return cur ? *cur : 0; }
    Q_UINT16 next() { return cur && *cur ? *(cur++) : 0; }
    Q_UINT16 current() { return cur ? *cur : 0; }

    int match(QString & str, unsigned int index);
    QChar head();
    QChar::Decomposition tag();

private:
    Q_UINT16 *ligatures;
    Q_UINT16 *cur;
};

QLigature::QLigature( QChar c )
{
    const Q_UINT16 *r = QUnicodeTables::ligature_info[c.row()];
    if( !r )
	ligatures = 0;
    else
    {
	const Q_UINT16 pos = r[c.cell()];
	ligatures = (Q_UINT16 *)&(QUnicodeTables::ligature_map[pos]);
    }
    cur = ligatures;
}

QChar QLigature::head()
{
    if(current())
	return QChar(QUnicodeTables::decomposition_map[current()+1]);

    return QChar::null;
}

QChar::Decomposition QLigature::tag()
{
    if(current())
	return (QChar::Decomposition) QUnicodeTables::decomposition_map[current()];

    return QChar::Canonical;
}

int QLigature::match(QString & str, unsigned int index)
{
    unsigned int i=index;

    if(!current()) return 0;

    Q_UINT16 lig = current() + 2;
    Q_UINT16 ch;

    while ((i < str.length()) && (ch = QUnicodeTables::decomposition_map[lig])) {
	if (str[(int)i] != QChar(ch))
	    return 0;
	i++;
	lig++;
    }

    if (!QUnicodeTables::decomposition_map[lig])
    {
	return i-index;
    }
    return 0;
}


// this function is just used in QString::compose()
static inline bool format(QChar::Decomposition tag, QString & str,
			  int index, int len)
{
    unsigned int l = index + len;
    unsigned int r = index;

    bool left = FALSE, right = FALSE;

    left = ((l < str.length()) &&
	    ((str[(int)l].joining() == QChar::Dual) ||
	     (str[(int)l].joining() == QChar::Right)));
    if (r > 0) {
	r--;
	//printf("joining(right) = %d\n", str[(int)r].joining());
	right = (str[(int)r].joining() == QChar::Dual);
    }


    switch (tag) {
    case QChar::Medial:
	return (left & right);
    case QChar::Initial:
	return (left && !right);
    case QChar::Final:
	return (right);// && !left);
    case QChar::Isolated:
    default:
	return (!right && !left);
    }
} // format()
#endif

/*
  QString::compose() and visual() were developed by Gordon Tisher
  <tisher@uniserve.ca>, with input from Lars Knoll <knoll@mpi-hd.mpg.de>,
  who developed the unicode data tables.
*/
/*!
    \warning This function is not supported in Qt 3.x. It is provided
    for experimental and illustrative purposes only. It is mainly of
    interest to those experimenting with Arabic and other
    composition-rich texts.

    Applies possible ligatures to a QString. Useful when
    composition-rich text requires rendering with glyph-poor fonts,
    but it also makes compositions such as QChar(0x0041) ('A') and
    QChar(0x0308) (Unicode accent diaresis), giving QChar(0x00c4)
    (German A Umlaut).
*/
void QString::compose()
{
#ifndef QT_NO_UNICODETABLES
    unsigned int index=0, len;
    unsigned int cindex = 0;

    QChar code, head;

    QMemArray<QChar> dia;

    QString composed = *this;

    while (index < length()) {
	code = at(index);
	//printf("\n\nligature for 0x%x:\n", code.unicode());
	QLigature ligature(code);
	ligature.first();
	while(ligature.current()) {
	    if ((len = ligature.match(*this, index)) != 0) {
		head = ligature.head();
		unsigned short code = head.unicode();
		// we exclude Arabic presentation forms A and a few
		// other ligatures, which are undefined in most fonts
		if(!(code > 0xfb50 && code < 0xfe80) &&
		   !(code > 0xfb00 && code < 0xfb2a)) {
				// joining info is only needed for Arabic
		    if (format(ligature.tag(), *this, index, len)) {
			//printf("using ligature 0x%x, len=%d\n",code,len);
			// replace letter
			composed.replace(cindex, len, QChar(head));
			index += len-1;
			// we continue searching in case we have a final
			// form because medial ones are preferred.
			if ( len != 1 || ligature.tag() !=QChar::Final )
			    break;
		    }
		}
	    }
	    ligature.next();
	}
	cindex++;
	index++;
    }
    *this = composed;
#endif
}


// These macros are used for efficient allocation of QChar strings.
// IMPORTANT! If you change these, make sure you also change the
// "delete unicode" statement in ~QStringData() in qstring.h correspondingly!

#define QT_ALLOC_QCHAR_VEC( N ) (QChar*) new char[ sizeof(QChar)*( N ) ]
#define QT_DELETE_QCHAR_VEC( P ) delete[] ((char*)( P ))


/*!
    This utility function converts the 8-bit string \a ba to Unicode,
    returning the result.

    The caller is responsible for deleting the return value with
    delete[].
*/

QChar* QString::latin1ToUnicode( const QByteArray& ba, uint* len )
{
    if ( ba.isNull() ) {
	*len = 0;
	return 0;
    }
    int l = 0;
    while ( l < (int)ba.size() && ba[l] )
	l++;
    char* str = ba.data();
    QChar *uc = new QChar[ l ];   // Can't use macro, since function is public
    QChar *result = uc;
    if ( len )
	*len = l;
    while (l--)
	*uc++ = *str++;
    return result;
}

static QChar* internalLatin1ToUnicode( const QByteArray& ba, uint* len )
{
    if ( ba.isNull() ) {
	*len = 0;
	return 0;
    }
    int l = 0;
    while ( l < (int)ba.size() && ba[l] )
	l++;
    char* str = ba.data();
    QChar *uc = QT_ALLOC_QCHAR_VEC( l );
    QChar *result = uc;
    if ( len )
	*len = l;
    while (l--)
	*uc++ = *str++;
    return result;
}

/*!
    \overload

    This utility function converts the '\0'-terminated 8-bit string \a
    str to Unicode, returning the result and setting \a *len to the
    length of the Unicode string.

    The caller is responsible for deleting the return value with
    delete[].
*/

QChar* QString::latin1ToUnicode( const char *str, uint* len, uint maxlen )
{
    QChar* result = 0;
    uint l = 0;
    if ( str ) {
	if ( maxlen != (uint)-1 ) {
	    while ( l < maxlen && str[l] )
		l++;
	} else {
	    // Faster?
	    l = strlen( str );
	}
	QChar *uc = new QChar[ l ]; // Can't use macro since function is public
	result = uc;
	uint i = l;
	while ( i-- )
	    *uc++ = *str++;
    }
    if ( len )
	*len = l;
    return result;
}

static QChar* internalLatin1ToUnicode( const char *str, uint* len,
				      uint maxlen = (uint)-1 )
{
    QChar* result = 0;
    uint l = 0;
    if ( str ) {
	if ( maxlen != (uint)-1 ) {
	    while ( l < maxlen && str[l] )
		l++;
	} else {
	    // Faster?
	    l = strlen( str );
	}
	QChar *uc = QT_ALLOC_QCHAR_VEC( l );
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
    This utility function converts \a l 16-bit characters from \a uc
    to ASCII, returning a '\0'-terminated string.

    The caller is responsible for deleting the resultant string with
    delete[].
*/
char* QString::unicodeToLatin1(const QChar *uc, uint l)
{
    if (!uc) {
	return 0;
    }
    char *a = new char[l+1];
    char *result = a;
    while (l--) {
	*a++ = (uc->unicode() > 0xff) ? '?' : (char)uc->unicode();
	uc++;
    }
    *a = '\0';
    return result;
}

/*****************************************************************************
  QString member functions
 *****************************************************************************/

/*!
    \class QString qstring.h
    \reentrant

    \brief The QString class provides an abstraction of Unicode text
    and the classic C '\0'-terminated char array.

    \ingroup tools
    \ingroup shared
    \ingroup text
    \mainclass

    QString uses \link shclass.html implicit sharing\endlink, which
    makes it very efficient and easy to use.

    In all of the QString methods that take \c {const char *}
    parameters, the \c {const char *} is interpreted as a classic
    C-style '\0'-terminated ASCII string. It is legal for the \c
    {const char *} parameter to be 0. If the \c {const char *} is not
    '\0'-terminated, the results are undefined. Functions that copy
    classic C strings into a QString will not copy the terminating
    '\0' character. The QChar array of the QString (as returned by
    unicode()) is generally not terminated by a '\0'. If you need to
    pass a QString to a function that requires a C '\0'-terminated
    string use latin1().

    \keyword QString::null
    A QString that has not been assigned to anything is \e null, i.e.
    both the length and data pointer is 0. A QString that references
    the empty string ("", a single '\0' char) is \e empty. Both null
    and empty QStrings are legal parameters to the methods. Assigning
    \c{(const char *) 0} to QString gives a null QString. For
    convenience, \c QString::null is a null QString. When sorting,
    empty strings come first, followed by non-empty strings, followed
    by null strings. We recommend using \c{if ( !str.isNull() )} to
    check for a non-null string rather than \c{if ( !str )}; see \l
    operator!() for an explanation.

    Note that if you find that you are mixing usage of \l QCString,
    QString, and \l QByteArray, this causes lots of unnecessary
    copying and might indicate that the true nature of the data you
    are dealing with is uncertain. If the data is '\0'-terminated 8-bit
    data, use \l QCString; if it is unterminated (i.e. contains '\0's)
    8-bit data, use \l QByteArray; if it is text, use QString.

    Lists of strings are handled by the QStringList class. You can
    split a string into a list of strings using QStringList::split(),
    and join a list of strings into a single string with an optional
    separator using QStringList::join(). You can obtain a list of
    strings from a string list that contain a particular substring or
    that match a particular \link qregexp.html regex\endlink using
    QStringList::grep().

    <b>Note for C programmers</b>

    Due to C++'s type system and the fact that QString is implicitly
    shared, QStrings may be treated like ints or other simple base
    types. For example:

    \code
    QString boolToString( bool b )
    {
	QString result;
	if ( b )
	    result = "True";
	else
	    result = "False";
	return result;
    }
    \endcode

    The variable, result, is an auto variable allocated on the stack.
    When return is called, because we're returning by value, The copy
    constructor is called and a copy of the string is returned. (No
    actual copying takes place thanks to the implicit sharing, see
    below.)

    Throughout Qt's source code you will encounter QString usages like
    this:
    \code
    QString func( const QString& input )
    {
	QString output = input;
	// process output
	return output;
    }
    \endcode

    The 'copying' of input to output is almost as fast as copying a
    pointer because behind the scenes copying is achieved by
    incrementing a reference count. QString (like all Qt's implicitly
    shared classes) operates on a copy-on-write basis, only copying if
    an instance is actually changed.

    If you wish to create a deep copy of a QString without losing any
    Unicode information then you should use QDeepCopy.

    \sa QChar QCString QByteArray QConstString
*/

/*! \enum Qt::ComparisonFlags
\internal
*/
/*!
    \enum Qt::StringComparisonMode

    This enum type is used to set the string comparison mode when
    searching for an item. It is used by QListBox, QListView and
    QIconView, for example. We'll refer to the string being searched
    as the 'target' string.

    \value CaseSensitive The strings must match case sensitively.
    \value ExactMatch The target and search strings must match exactly.
    \value BeginsWith The target string begins with the search string.
    \value EndsWith The target string ends with the search string.
    \value Contains The target string contains the search string.

    If you OR these flags together (excluding \c CaseSensitive), the
    search criteria be applied in the following order: \c ExactMatch,
    \c BeginsWith, \c EndsWith, \c Contains.

    Matching is case-insensitive unless \c CaseSensitive is set. \c
    CaseSensitive may be OR-ed with any combination of the other
    flags.

*/
Q_EXPORT QStringData *QString::shared_null = 0;
QT_STATIC_CONST_IMPL QString QString::null;
QT_STATIC_CONST_IMPL QChar QChar::null;
QT_STATIC_CONST_IMPL QChar QChar::replacement((ushort)0xfffd);
QT_STATIC_CONST_IMPL QChar QChar::byteOrderMark((ushort)0xfeff);
QT_STATIC_CONST_IMPL QChar QChar::byteOrderSwapped((ushort)0xfffe);
QT_STATIC_CONST_IMPL QChar QChar::nbsp((ushort)0x00a0);

QStringData* QString::makeSharedNull()
{
    QString::shared_null = new QStringData;
#if defined( Q_OS_MAC )
    QString *that = const_cast<QString *>(&QString::null);
    that->d = QString::shared_null;
#endif
    return QString::shared_null;
}

/*!
    \fn QString::QString()

    Constructs a null string, i.e. both the length and data pointer
    are 0.

    \sa isNull()
*/

/*!
    Constructs a string of length one, containing the character \a ch.
*/
QString::QString( QChar ch )
{
    d = new QStringData( QT_ALLOC_QCHAR_VEC( 1 ), 1, 1 );
    d->unicode[0] = ch;
}

/*!
    Constructs an implicitly shared copy of \a s. This is very fast
    since it only involves incrementing a reference count.
*/
QString::QString( const QString &s ) :
    d(s.d)
{
    d->ref();
}

/*!
  \internal

  Private function.

  Constructs a string with preallocated space for \a size characters.

  The string is empty.

  \sa isNull()
*/

QString::QString( int size, bool /*dummy*/ )
{
    if ( size ) {
	int l = size;
	QChar* uc = QT_ALLOC_QCHAR_VEC( l );
	d = new QStringData( uc, 0, l );
    } else {
	d = shared_null ? shared_null : (shared_null=new QStringData);
	d->ref();
    }
}

/*!
    Constructs a string that is a deep copy of \a ba interpreted as a
    classic C string.
*/

QString::QString( const QByteArray& ba )
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	d = 0;
	*this = fromAscii( ba.data() );
	return;
    }
#endif
    uint l;
    QChar *uc = internalLatin1ToUnicode(ba,&l);
    d = new QStringData(uc,l,l);
}

/*!
    Constructs a string that is a deep copy of the first \a length
    characters in the QChar array.

    If \a unicode and \a length are 0, then a null string is created.

    If only \a unicode is 0, the string is empty but has \a length
    characters of space preallocated: QString expands automatically
    anyway, but this may speed up some cases a little. We recommend
    using the plain constructor and setLength() for this purpose since
    it will result in more readable code.

    \sa isNull() setLength()
*/

QString::QString( const QChar* unicode, uint length )
{
    if ( !unicode && !length ) {
	d = shared_null ? shared_null : makeSharedNull();
	d->ref();
    } else {
	QChar* uc = QT_ALLOC_QCHAR_VEC( length );
	if ( unicode )
	    memcpy(uc, unicode, length*sizeof(QChar));
	d = new QStringData(uc,unicode ? length : 0,length);
    }
}

/*!
    Constructs a string that is a deep copy of \a str, interpreted as
    a classic C string.

    If \a str is 0, then a null string is created.

    This is a cast constructor, but it is perfectly safe: converting a
    Latin1 const char* to QString preserves all the information. You
    can disable this constructor by defining \c QT_NO_CAST_ASCII when
    you compile your applications. You can also make QString objects
    by using setLatin1(), fromLatin1(), fromLocal8Bit(), and
    fromUtf8(). Or whatever encoding is appropriate for the 8-bit data
    you have.

    \sa isNull()
*/

QString::QString( const char *str )
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	d = 0;
	*this = fromAscii( str );
	return;
    }
#endif
    uint l;
    QChar *uc = internalLatin1ToUnicode(str,&l);
    d = new QStringData(uc,l,l);
}

#ifndef QT_NO_STL
/*!
    Constructs a string that is a deep copy of \a str.

    This is the same as fromAscii(\a str).
*/

QString::QString( const std::string &str )
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	d = 0;
	*this = fromAscii( str.c_str() );
	return;
    }
#endif
    uint l;
    QChar *uc = internalLatin1ToUnicode(str.c_str(),&l);
    d = new QStringData(uc,l,l);
}
#endif

/*!
    \fn QString::~QString()

    Destroys the string and frees the string's data if this is the
    last reference to the string.
*/


/*!
    Deallocates any space reserved solely by this QString.

    If the string does not share its data with another QString
    instance, nothing happens; otherwise the function creates a new,
    unique copy of this string. This function is called whenever the
    string is modified.
*/

void QString::real_detach()
{
    setLength( length() );
}

void QString::deref()
{
    if ( d && d->deref() ) {
	if ( d != shared_null )
	    delete d;
	d = 0;
    }
}

void QStringData::deleteSelf()
{
    delete this;
}

/*!
    \fn QString& QString::operator=( QChar c )

    Sets the string to contain just the single character \a c.
*/

/*!
    \fn QString& QString::operator=( const std::string& s )

    \overload

    Makes a deep copy of \a s and returns a reference to the deep
    copy.
*/

/*!
    \fn QString& QString::operator=( char c )

    \overload

    Sets the string to contain just the single character \a c.
*/

/*!
    \overload

    Assigns a shallow copy of \a s to this string and returns a
    reference to this string. This is very fast because the string
    isn't actually copied.
*/
QString &QString::operator=( const QString &s )
{
    s.d->ref();
    deref();
    d = s.d;
    return *this;
}

/*!
    \overload

    Assigns a deep copy of \a cs, interpreted as a classic C string,
    to this string and returns a reference to this string.
*/
QString &QString::operator=( const QCString& cs )
{
    return setAscii(cs);
}


/*!
    \overload

    Assigns a deep copy of \a str, interpreted as a classic C string
    to this string and returns a reference to this string.

    If \a str is 0, then a null string is created.

    \sa isNull()
*/
QString &QString::operator=( const char *str )
{
    return setAscii(str);
}


/*!
    \fn bool QString::isNull() const

    Returns TRUE if the string is null; otherwise returns FALSE. A
    null string is always empty.

    \code
	QString a;          // a.unicode() == 0, a.length() == 0
	a.isNull();         // TRUE, because a.unicode() == 0
	a.isEmpty();        // TRUE
    \endcode

    \sa isEmpty(), length()
*/

/*!
    \fn bool QString::isEmpty() const

    Returns TRUE if the string is empty, i.e. if length() == 0;
    otherwise returns FALSE. Null strings are also empty.

    \code
	QString a( "" );
	a.isEmpty();        // TRUE
	a.isNull();         // FALSE

	QString b;
	b.isEmpty();        // TRUE
	b.isNull();         // TRUE
    \endcode

    \sa isNull(), length()
*/

/*!
    \fn uint QString::length() const

    Returns the length of the string.

    Null strings and empty strings have zero length.

    \sa isNull(), isEmpty()
*/

/*!
    If \a newLen is less than the length of the string, then the
    string is truncated at position \a newLen. Otherwise nothing
    happens.

    \code
	QString s = "truncate me";
	s.truncate( 5 );            // s == "trunc"
    \endcode

    \sa setLength()
*/

void QString::truncate( uint newLen )
{
    if ( newLen < d->len )
	setLength( newLen );
}

/*!
    Ensures that at least \a newLen characters are allocated to the
    string, and sets the length of the string to \a newLen. Any new
    space allocated contains arbitrary data.

    If \a newLen is 0, then the string becomes empty, unless the
    string is null, in which case it remains null.

    If it is not possible to allocate enough memory, the string
    remains unchanged.

    This function always detaches the string from other references to
    the same data.

    This function is useful for code that needs to build up a long
    string and wants to avoid repeated reallocation. In this example,
    we want to add to the string until some condition is true, and
    we're fairly sure that size is big enough:
    \code
	QString result;
	int resultLength = 0;
	result.setLength( newLen ) // allocate some space
	while ( ... ) {
	    result[resultLength++] = ... // fill (part of) the space with data
	}
	result.truncate[resultLength]; // and get rid of the undefined junk
    \endcode

    If \a newLen is an underestimate, the worst that will happen is
    that the loop will slow down.

    \sa truncate(), isNull(), isEmpty(), length()
*/

void QString::setLength( uint newLen )
{
    if ( d->count != 1 || newLen > d->maxl ||
	 ( newLen * 4 < d->maxl && d->maxl > 4 ) ) {
	// detach, grow or shrink
	uint newMax = computeNewMax( newLen );
	QChar* nd = QT_ALLOC_QCHAR_VEC( newMax );
	if ( nd ) {
	    uint len = QMIN( d->len, newLen );
	    if ( d->unicode )
		memcpy( nd, d->unicode, sizeof(QChar)*len );
	    deref();
	    d = new QStringData( nd, newLen, newMax );
	}
    } else {
	d->len = newLen;
	d->setDirty();
    }
}

/*!
    This function will return a string that replaces the lowest
    numbered occurrence of \c %1, \c %2, ..., \c %9 with \a a.

    The \a fieldwidth value specifies the minimum amount of space that
    \a a is padded to. A positive value will produce right-aligned
    text, whereas a negative value will produce left-aligned text.

    \code
	QString firstName( "Joe" );
	QString lastName( "Bloggs" );
	QString fullName;
	fullName = QString( "First name is '%1', last name is '%2'" )
		    .arg( firstName )
		    .arg( lastName );

	// fullName == First name is 'Joe', last name is 'Bloggs'
    \endcode

    Note that using arg() to construct sentences as we've done in the
    example above does not usually translate well into other languages
    because sentence structure and word order often differ between
    languages.

    If there is no place marker (\c %1 or \c %2, etc.), a warning
    message (qWarning()) is output and the text is appended at the
    end of the string. We recommend that the correct number of place
    markers is always used in production code.
*/
QString QString::arg( const QString& a, int fieldwidth ) const
{
    int pos, len;
    QString r = *this;

    if ( !findArg( pos, len ) ) {
	qWarning( "QString::arg(): Argument missing: %s, %s",
		  latin1(), a.latin1() );
	// Make sure the text at least appears SOMEWHERE
	r += ' ';
	pos = r.length();
	len = 0;
    }

    r.replace( pos, len, a );
    if ( fieldwidth < 0 ) {
	QString s;
	while ( (uint)-fieldwidth > a.length() ) {
	    s += ' ';
	    fieldwidth++;
	}
	r.insert( pos + a.length(), s );
    } else if ( fieldwidth ) {
	QString s;
	while ( (uint)fieldwidth > a.length() ) {
	    s += ' ';
	    fieldwidth--;
	}
	r.insert( pos, s );
    }

    return r;
}


/*!
    \overload

    The \a fieldwidth value specifies the minimum amount of space that
    \a a is padded to. A positive value will produce a right-aligned
    number, whereas a negative value will produce a left-aligned
    number.

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.

    \code
	QString str;
	str = QString( "Decimal 63 is %1 in hexadecimal" )
		.arg( 63, 0, 16 );
	// str == "Decimal 63 is 3f in hexadecimal"
    \endcode
*/
QString QString::arg( long a, int fieldwidth, int base ) const
{
    return arg( QString::number(a, base), fieldwidth );
}

/*!
    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/
QString QString::arg( ulong a, int fieldwidth, int base ) const
{
    return arg( QString::number(a, base), fieldwidth );
}

/*!
    \fn QString QString::arg( int a, int fieldwidth, int base ) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/

/*!
    \fn QString QString::arg( uint a, int fieldwidth, int base ) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/

/*!
    \fn QString QString::arg( short a, int fieldwidth, int base ) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/

/*!
    \fn QString QString::arg( ushort a, int fieldwidth, int base ) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/


/*!
    \overload

    \a a is assumed to be in the Latin1 character set.
*/
QString QString::arg( char a, int fieldwidth ) const
{
    QString c;
    c += a;
    return arg( c, fieldwidth );
}

/*!
    \overload
*/
QString QString::arg( QChar a, int fieldwidth ) const
{
    QString c;
    c += a;
    return arg( c, fieldwidth );
}

/*!
    \overload

    \target arg-formats

    Argument \a a is formatted according to the \a fmt format specified,
    which is 'g' by default and can be any of the following:

    \table
    \header \i Format \i Meaning
    \row \i \c e \i format as [-]9.9e[+|-]999
    \row \i \c E \i format as [-]9.9E[+|-]999
    \row \i \c f \i format as [-]9.9
    \row \i \c g \i use \c e or \c f format, whichever is the most concise
    \row \i \c G \i use \c E or \c f format, whichever is the most concise
    \endtable

    With 'e', 'E', and 'f', \a prec is the number of digits after the
    decimal point. With 'g' and 'G', \a prec is the maximum number of
    significant digits (trailing zeroes are omitted).

    \code
	double d = 12.34;
	QString ds = QString( "'E' format, precision 3, gives %1" )
			.arg( d, 0, 'E', 3 );
	// ds == "1.234E+001"
    \endcode
*/
QString QString::arg( double a, int fieldwidth, char fmt, int prec ) const
{
    return arg( QString::number( a, fmt, prec ), fieldwidth );
}


/*
    Just 1-digit arguments.
*/
bool QString::findArg( int& pos, int& len ) const
{
    char lowest=0;
    register const QChar *uc = d->unicode;
    const uint l = length();
    for (uint i = 0; i < l; i++) {
	if ( uc[i] == '%' && i+1<l ) {
	    QChar dig = uc[i+1];
	    if ( dig >= '0' && dig <= '9' ) {
		if ( !lowest || dig < lowest ) {
		    lowest = dig;
		    pos = i;
		    len = 2;
		}
	    }
	}
    }
    return lowest != 0;
}

/*!
    Safely builds a formatted string from the format string \a cformat
    and an arbitrary list of arguments. The format string supports all
    the escape sequences of printf() in the standard C library.

    The %s escape sequence expects a utf8() encoded string. The format
    string \e cformat is expected to be in latin1. If you need a
    Unicode format string, use arg() instead. For typesafe string
    building, with full Unicode support, you can use QTextOStream like
    this:

    \code
	QString str;
	QString s = ...;
	int x = ...;
	QTextOStream( &str ) << s << " : " << x;
    \endcode

    For \link QObject::tr() translations,\endlink especially if the
    strings contains more than one escape sequence, you should
    consider using the arg() function instead. This allows the order
    of the replacements to be controlled by the translator, and has
    Unicode support.

    \sa arg()
*/

#ifndef QT_NO_SPRINTF
QString &QString::sprintf( const char* cformat, ... )
{
    va_list ap;
    va_start( ap, cformat );

    if ( !cformat || !*cformat ) {
	// Qt 1.x compat
	*this = fromLatin1( "" );
	return *this;
    }
    QString format = fromAscii( cformat );

    QRegExp escape( "%#?0?-? ?\\+?'?[0-9*]*\\.?[0-9*]*h?l?L?q?Z?" );
    QString result;
    uint last = 0;
    int pos;
    int len = 0;

    for (;;) {
	pos = escape.search( format, last );
	len = escape.matchedLength();
	// Non-escaped text
	if ( pos > (int)last )
	    result += format.mid( last, pos - last );
	if ( pos < 0 ) {
	    // The rest
	    if ( last < format.length() )
		result += format.mid( last );
	    break;
	}
	last = pos + len + 1;

	// Escape
	QString f = format.mid( pos, len );
	uint width, decimals;
	int params = 0;
	int wpos = f.find('*');
	if ( wpos >= 0 ) {
	    params++;
	    width = va_arg( ap, int );
	    if ( f.find('*', wpos + 1) >= 0 ) {
		decimals = va_arg( ap, int );
		params++;
	    } else {
		decimals = 0;
	    }
	} else {
	    decimals = width = 0;
	}
	QString replacement;
	if ( format[pos + len] == 's' || format[pos + len] == 'S' ||
	     format[pos + len] == 'c' )
	{
	    bool rightjust = ( f.find('-') < 0 );
	    // %-5s really means left adjust in sprintf

	    if ( wpos < 0 ) {
		QRegExp num( fromLatin1("[0-9]+") );
		int p = num.search( f );
		int nlen = num.matchedLength();
		int q = f.find( '.' );
		if ( q < 0 || (p < q && p >= 0) )
		    width = f.mid( p, nlen ).toInt();
		if ( q >= 0 ) {
		    p = num.search( f, q );
		    // "decimals" is used to specify string truncation
		    if ( p >= 0 )
			decimals = f.mid( p, nlen ).toInt();
		}
	    }

	    if ( format[pos + len] == 's' ) {
		QString s = QString::fromUtf8( va_arg(ap, char*) );
		replacement = ( decimals <= 0 ) ? s : s.left( decimals );
	    } else {
		int ch = va_arg(ap, int);
		replacement = QChar((ushort)ch);
	    }
	    if ( replacement.length() < width ) {
		replacement = rightjust
		    ? replacement.rightJustify(width)
		    : replacement.leftJustify(width);
	    }
	} else if ( format[pos+len] == '%' ) {
	    replacement = '%';
	} else if ( format[pos+len] == 'n' ) {
	    int* n = va_arg(ap, int*);
	    *n = result.length();
	} else {
	    char in[64], out[330];
	    strncpy(in,f.latin1(),63);
	    out[0] = '\0';
	    char fch = format[pos+len].latin1();
	    in[f.length()] = fch;
	    switch ( fch ) {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
		{
		    int value = va_arg( ap, int );
		    switch ( params ) {
		    case 0:
			::sprintf( out, in, value );
			break;
		    case 1:
			::sprintf( out, in, width, value );
			break;
		    case 2:
			::sprintf( out, in, width, decimals, value );
		    }
		}
		break;
	    case 'e':
	    case 'E':
	    case 'f':
	    case 'g':
	    case 'G':
		{
		    double value = va_arg( ap, double );
		    switch ( params ) {
		    case 0:
			::sprintf( out, in, value );
			break;
		    case 1:
			::sprintf( out, in, width, value );
			break;
		    case 2:
			::sprintf( out, in, width, decimals, value );
		    }
		}
		break;
	    case 'p':
		{
		    void* value = va_arg( ap, void * );
		    switch ( params ) {
		    case 0:
			::sprintf( out, in, value );
			break;
		    case 1:
			::sprintf( out, in, width, value );
			break;
		    case 2:
			::sprintf( out, in, width, decimals, value );
		    }
		}
	    }
	    replacement = fromAscii( out );
	}
	result += replacement;
    }
    *this = result;

    va_end( ap );
    return *this;
}
#endif

/*!
    Fills the string with \a len characters of value \a c, and returns
    a reference to the string.

    If \a len is negative (the default), the current string length is
    used.

    \code
	QString str;
	str.fill( 'g', 5 );      // string == "ggggg"
    \endcode
*/

QString& QString::fill( QChar c, int len )
{
    if ( len < 0 )
	len = length();
    if ( len == 0 ) {
	*this = "";
    } else {
	deref();
	QChar * nd = QT_ALLOC_QCHAR_VEC( len );
	d = new QStringData(nd,len,len);
	while (len--) *nd++ = c;
    }
    return *this;
}


/*!
  \fn QString QString::copy() const

  \obsolete

  In Qt 2.0 and later, all calls to this function are needless. Just
  remove them.
*/

/*!
    \overload

    Finds the first occurrence of the character \a c, starting at
    position \a index. If \a index is -1, the search starts at the
    last character; if -2, at the next to last character and so on.
    (See findRev() for searching backwards.)

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.

    Returns the position of \a c or -1 if \a c could not be found.
*/

int QString::find( QChar c, int index, bool cs ) const
{
    const uint l = length();
    if ( index < 0 )
	index += l;
    if ( (uint)index >= l )
	return -1;
    register const QChar *uc = unicode()+index;
    const QChar *end = unicode() + l;
    if ( cs ) {
	while ( uc < end && *uc != c )
	    uc++;
    } else {
	c = ::lower( c );
	while ( uc < end && ::lower( *uc ) != c )
	    uc++;
    }
    if ( uint(uc - unicode()) >= l )
	return -1;
    return (int)(uc - unicode());
}

/* an implementation of the Boyer-Moore search algorithm
*/

/* initializes the skiptable to know haw far ahead we can skip on a wrong match
*/
static void bm_init_skiptable( const QString &pattern, uint *skiptable, bool cs )
{
    int i = 0;
    register uint *st = skiptable;
    int l = pattern.length();
    while ( i++ < 0x100/8 ) {
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
    }
    const QChar *uc = pattern.unicode();
    if ( cs ) {
	while( l-- ) {
	    skiptable[ uc->cell() ] = l;
	    uc++;
	}
    } else {
	while( l-- ) {
	    skiptable[ ::lower( *uc ).cell() ] = l;
	    uc++;
	}
    }
}

static int bm_find( const QString &str, int index, const QString &pattern, uint *skiptable, bool cs )
{
    const uint l = str.length();
    if ( pattern.isEmpty() )
	return index > (int)l ? -1 : index;

    const QChar *uc = str.unicode();
    const QChar *puc = pattern.unicode();
    const uint pl = pattern.length();
    const uint pl_minus_one = pl - 1;

    register const QChar *current = uc + index + pl_minus_one;
    const QChar *end = uc + l;
    if ( cs ) {
	while( current < end ) {
	    uint skip = skiptable[ current->cell() ];
	    if ( !skip ) {
		// possible match
		while( skip < pl ) {
		    if ( *(current - skip ) != puc[pl_minus_one-skip] )
			break;
		    skip++;
		}
		if ( skip > pl_minus_one ) { // we have a match
		    return (current - uc) - skip + 1;
		}
		// in case we don't have a match we are a bit inefficient as we only skip by one
		// when we have the non matching char in the string.
		if ( skiptable[ (current-skip)->cell() ] == pl )
		    skip = pl - skip;
		else
		    skip = 1;
	    }
	    current += skip;
	}
    } else {
	while( current < end ) {
	    uint skip = skiptable[ ::lower( *current ).cell() ];
	    if ( !skip ) {
		// possible match
		while( skip < pl ) {
		    if ( ::lower( *(current - skip) ) != ::lower( puc[pl_minus_one-skip] ) )
			break;
		    skip++;
		}
		if ( skip > pl_minus_one ) // we have a match
		    return (current - uc) - skip + 1;
		// in case we don't have a match we are a bit inefficient as we only skip by one
		// when we have the non matching char in the string.
		if ( skiptable[ ::lower( (current - skip)->cell() ) ] == pl )
		    skip = pl - skip;
		else
		    skip = 1;
	    }
	    current += skip;
	}
    }
    // not found
    return -1;
}


#define REHASH( a ) \
    if ( sl_minus_1 < sizeof(uint) * CHAR_BIT ) \
	hashHaystack -= (a) << sl_minus_1; \
    hashHaystack <<= 1

/*!
    \overload

    Finds the first occurrence of the string \a str, starting at
    position \a index. If \a index is -1, the search starts at the
    last character, if it is -2, at the next to last character and so
    on. (See findRev() for searching backwards.)

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.

    Returns the position of \a str or -1 if \a str could not be found.
*/

int QString::find( const QString& str, int index, bool cs ) const
{
    const uint l = length();
    const uint sl = str.length();
    if ( index < 0 )
	index += l;
    if ( sl + index > l )
	return -1;
    if ( !sl )
	return index;

    if ( sl == 1 )
	return find( *str.unicode(), index, cs );

    // we use the Boyer-Moore algorithm in cases where the overhead
    // for the hash table should pay off, otherwise we use a simple
    // hash function
    if ( l > 500 && sl > 5 ) {
	uint skiptable[0x100];
	bm_init_skiptable( str, skiptable, cs );
	return bm_find( *this, index, str, skiptable, cs );
    }

    /*
      We use some hashing for efficiency's sake. Instead of
      comparing strings, we compare the hash value of str with that of
      a part of this QString. Only if that matches, we call ucstrncmp
      or ucstrnicmp.
    */
    const QChar* needle = str.unicode();
    const QChar* haystack = unicode() + index;
    const QChar* end = unicode() + (l-sl);
    const uint sl_minus_1 = sl-1;
    uint hashNeedle = 0, hashHaystack = 0, i;

    if ( cs ) {
	for ( i = 0; i < sl; ++i ) {
	    hashNeedle = ((hashNeedle<<1) + needle[i].unicode() );
	    hashHaystack = ((hashHaystack<<1) + haystack[i].unicode() );
	}
	hashHaystack -= (haystack+sl_minus_1)->unicode();

	while ( haystack <= end ) {
	    hashHaystack += (haystack+sl_minus_1)->unicode();
 	    if ( hashHaystack == hashNeedle
		 && ucstrncmp( needle, haystack, sl ) == 0 )
		return haystack-unicode();

	    REHASH( haystack->unicode() );
	    ++haystack;
	}
    } else {
	for ( i = 0; i < sl; ++i ) {
	    hashNeedle = ((hashNeedle<<1) +
			  ::lower( needle[i].unicode() ).unicode() );
	    hashHaystack = ((hashHaystack<<1) +
			    ::lower( haystack[i].unicode() ).unicode() );
	}

	hashHaystack -= ::lower(*(haystack+sl_minus_1)).unicode();
	while ( haystack <= end ) {
	    hashHaystack += ::lower(*(haystack+sl_minus_1)).unicode();
	    if ( hashHaystack == hashNeedle
		 && ucstrnicmp( needle, haystack, sl ) == 0 )
		return haystack-unicode();

	    REHASH( ::lower(*haystack).unicode() );
	    ++haystack;
	}
    }
    return -1;
}

/*!
    \fn int QString::findRev( const char* str, int index ) const

    Equivalent to findRev(QString(\a str), \a index).
*/

/*!
    \fn int QString::find( const char* str, int index ) const

    \overload

    Equivalent to find(QString(\a str), \a index).
*/

/*!
    \overload

    Finds the first occurrence of the character \a c, starting at
    position \a index and searching backwards. If the index is -1, the
    search starts at the last character, if it is -2, at the next to
    last character and so on.

    Returns the position of \a c or -1 if \a c could not be found.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.

    \code
	QString string( "bananas" );
	int i = string.findRev( 'a' );      // i == 5
    \endcode
*/

int QString::findRev( QChar c, int index, bool cs ) const
{
    const uint l = length();
    if ( index < 0 )
	index += l;
    if ( (uint)index >= l )
	return -1;
    const QChar *end = unicode();
    register const QChar *uc = end + index;
    if ( cs ) {
	while ( uc >= end && *uc != c )
	    uc--;
    } else {
	c = ::lower( c );
	while ( uc >= end && ::lower( *uc ) != c )
	    uc--;
    }
    return uc - end;
}

/*!
    \overload

    Finds the first occurrence of the string \a str, starting at
    position \a index and searching backwards. If the index is -1, the
    search starts at the last character, if it is -2, at the next to
    last character and so on.

    Returns the position of \a str or -1 if \a str could not be found.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.

    \code
    QString string("bananas");
    int i = string.findRev( "ana" );      // i == 3
    \endcode
*/

int QString::findRev( const QString& str, int index, bool cs ) const
{
    /*
      See QString::find() for explanations.
    */
    const uint l = length();
    if ( index < 0 )
	index += l;
    const uint sl = str.length();
    int delta = l-sl;
    if ( index < 0 || index > (int)l || delta < 0 )
	return -1;
    if ( index > delta )
	index = delta;

    if ( sl == 1 )
	return findRev( *str.unicode(), index, cs );

    const QChar* needle = str.unicode();
    const QChar* haystack = unicode() + index;
    const QChar* end = unicode();
    const uint sl_minus_1 = sl-1;
    const QChar* n = needle+sl_minus_1;
    const QChar* h = haystack+sl_minus_1;
    uint hashNeedle = 0, hashHaystack = 0, i;

    if ( cs ) {
	for ( i = 0; i < sl; ++i ) {
	    hashNeedle = ((hashNeedle<<1) + (n-i)->unicode() );
	    hashHaystack = ((hashHaystack<<1) + (h-i)->unicode() );
	}
	hashHaystack -= haystack->unicode();

	while ( haystack >= end ) {
	    hashHaystack += haystack->unicode();
 	    if ( hashHaystack == hashNeedle
		 && ucstrncmp( needle, haystack, sl ) == 0 )
		return haystack-unicode();
	    --haystack;
	    REHASH( (haystack+sl)->unicode() );
	}
    } else {
	for ( i = 0; i < sl; ++i ) {
	    hashNeedle = ((hashNeedle<<1)
			  + ::lower( (n-i)->unicode() ).unicode() );
	    hashHaystack = ((hashHaystack<<1)
			    + ::lower( (h-i)->unicode() ).unicode() );
	}
	hashHaystack -= ::lower(*haystack).unicode();

	while ( haystack >= end ) {
	    hashHaystack += ::lower(*haystack).unicode();
	    if ( hashHaystack == hashNeedle
		 && ucstrnicmp( needle, haystack, sl ) == 0 )
		return haystack-unicode();
	    --haystack;
	    REHASH( ::lower(*(haystack+sl)).unicode() );
	}
    }
    return -1;
}

#undef REHASH

/*!
    \enum QString::SectionFlags

    \value SectionDefault Empty fields are counted, leading and
    trailing separators are not included, and the separator is
    compared case sensitively.

    \value SectionSkipEmpty Treat empty fields as if they don't exist,
    i.e. they are not considered as far as \e start and \e end are
    concerned.

    \value SectionIncludeLeadingSep Include the leading separator (if
    any) in the result string.

    \value SectionIncludeTrailingSep Include the trailing separator
    (if any) in the result string.

    \value SectionCaseInsensitiveSeps Compare the separator
    case-insensitively.

    Any of the last four values can be OR-ed together to form a flag.

    \sa section()
*/

/*!
    \fn QString QString::section( QChar sep, int start, int end = 0xffffffff, int flags = SectionDefault ) const

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    character, \a sep. The returned string consists of the fields from
    position \a start to position \a end inclusive. If \a end is not
    specified, all fields from position \a start to the end of the
    string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behaviour, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \code
    QString csv( "forename,middlename,surname,phone" );
    QString s = csv.section( ',', 2, 2 );   // s == "surname"

    QString path( "/usr/local/bin/myapp" ); // First field is empty
    QString s = path.section( '/', 3, 4 );  // s == "bin/myapp"
    QString s = path.section( '/', 3, 3, SectionSkipEmpty ); // s == "myapp"
    \endcode

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \code
    QString csv( "forename,middlename,surname,phone" );
    QString s = csv.section( ',', -3, -2 );  // s == "middlename,surname"

    QString path( "/usr/local/bin/myapp" ); // First field is empty
    QString s = path.section( '/', -1 ); // s == "myapp"
    \endcode

    \sa QStringList::split()
*/

/*!
    \overload

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    string, \a sep. The returned string consists of the fields from
    position \a start to position \a end inclusive. If \a end is not
    specified, all fields from position \a start to the end of the
    string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behaviour, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \code
    QString data( "forename**middlename**surname**phone" );
    QString s = data.section( "**", 2, 2 ); // s == "surname"
    \endcode

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \code
    QString data( "forename**middlename**surname**phone" );
    QString s = data.section( "**", -3, -2 ); // s == "middlename**surname"
    \endcode

    \sa QStringList::split()
*/

QString QString::section( const QString &sep, int start, int end, int flags ) const
{
    const QChar *uc = unicode();
    if ( !uc )
	return QString();
    QString _sep = (flags & SectionCaseInsensitiveSeps) ? sep.lower() : sep;
    const QChar *uc_sep = _sep.unicode();
    if(!uc_sep)
	return QString();
    bool match = FALSE, last_match = TRUE;

    //find start
    int n = length(), sep_len = _sep.length();
    const QChar *begin = start < 0 ? uc + n : uc;
    while(start) {
	match = FALSE;
	int c = 0;
	for(const QChar *tmp = start < 0 ? begin - sep_len : begin;
	    c < sep_len && tmp < uc + n && tmp >= uc; tmp++, c++) {
	    if(flags & SectionCaseInsensitiveSeps) {
		if( ::lower( *tmp ) != *(uc_sep + c))
		    break;
	    } else {
		if( *tmp != *(uc_sep + c) )
		    break;
	    }
	    if(c == sep_len - 1) {
		match = TRUE;
		break;
	    }
	}
	if(start > 0 && (flags & SectionSkipEmpty) && match && last_match)
	    match = FALSE;
	last_match = match;

	if(start < 0) {
	    if(match) {
		begin -= sep_len;
		if(!++start)
		    break;
	    } else {
		if(start == -1 && begin == uc)
		    break;
		begin--;
	    }
	} else {
	    if(match) {
		if(!--start)
		    break;
		begin += sep_len;
	    } else {
		if(start == 1 && begin == uc + n)
		    break;
		begin++;
	    }
	}
	if(begin > uc + n || begin < uc)
	    return QString();
    }
    if(match && !(flags & SectionIncludeLeadingSep))
	begin+=sep_len;
    if(begin > uc + n || begin < uc)
	return QString();

    //now find last
    match = FALSE;
    const QChar *last = end < 0 ? uc + n : uc;
    if(end == -1) {
	int c = 0;
	for(const QChar *tmp = end < 0 ? last - sep_len : last;
	    c < sep_len && tmp < uc + n && tmp >= uc; tmp++, c++) {
	    if(flags & SectionCaseInsensitiveSeps) {
		if( ::lower( *tmp ) != *(uc_sep + c))
		    break;
	    } else {
		if( *tmp != *(uc_sep + c) )
		    break;
	    }
	    if(c == sep_len - 1) {
		match = TRUE;
		break;
	    }
	}
    } else {
	end++;
	last_match = TRUE;
	while(end) {
	    match = FALSE;
	    int c = 0;
	    for(const QChar *tmp = end < 0 ? last - sep_len : last;
		c < sep_len && tmp < uc + n && tmp >= uc; tmp++, c++) {
		if(flags & SectionCaseInsensitiveSeps) {
		    if( ::lower( *tmp ) != *(uc_sep + c))
			break;
		} else {
		    if( *tmp != *(uc_sep + c) )
			break;
		}
		if(c == sep_len - 1) {
		    match = TRUE;
		    break;
		}
	    }
	    if(end > 0 && (flags & SectionSkipEmpty) && match && last_match)
		match = FALSE;
	    last_match = match;

	    if(end < 0) {
		if(match) {
		    if(!++end)
			break;
		    last -= sep_len;
		} else {
		    last--;
		}
	    } else {
		if(match) {
		    last += sep_len;
		    if(!--end)
			break;
		} else {
		    last++;
		}
	    }
	    if(last >= uc + n) {
		last = uc + n;
		break;
	    } else if(last < uc) {
		return QString();
	    }
	}
    }
    if(match && !(flags & SectionIncludeTrailingSep))
	last -= sep_len;
    if(last < uc || last > uc + n || begin >= last)
	return QString();

    //done
    return QString(begin, last - begin);
}

#ifndef QT_NO_REGEXP
class section_chunk {
public:
    section_chunk(int l, QString s) { length = l; string = s; }
    int length;
    QString string;
};
/*!
    \overload

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    regular expression, \a reg. The returned string consists of the
    fields from position \a start to position \a end inclusive. If \a
    end is not specified, all fields from position \a start to the end
    of the string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behaviour, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \code
    QString line( "forename\tmiddlename  surname \t \t phone" );
    QRegExp sep( "\s+" );
    QString s = line.section( sep, 2, 2 ); // s == "surname"
    \endcode

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \code
    QString line( "forename\tmiddlename  surname \t \t phone" );
    QRegExp sep( "\\s+" );
    QString s = line.section( sep, -3, -2 ); // s == "middlename  surname"
    \endcode

    \warning Using this QRegExp version is much more expensive than
    the overloaded string and character versions.

    \sa QStringList::split() simplifyWhiteSpace()
*/

QString QString::section( const QRegExp &reg, int start, int end, int flags ) const
{
    const QChar *uc = unicode();
    if(!uc)
	return QString();

    QRegExp sep(reg);
    sep.setCaseSensitive(!(flags & SectionCaseInsensitiveSeps));

    QPtrList<section_chunk> l;
    l.setAutoDelete(TRUE);
    int n = length(), m = 0, last_m = 0, last = 0, last_len = 0;

    while( ( m = sep.search( *this, m ) ) != -1 ) {
	l.append(new section_chunk(last_len, QString(uc + last_m, m - last_m)));
	last_m = m;
	last_len = sep.matchedLength();
	if((m += sep.matchedLength()) >= n) {
	    last = 1;
	    break;
	}
    }
    if(!last)
	l.append(new section_chunk(last_len, QString(uc + last_m, n - last_m)));

    if(start < 0)
	start = l.count() + start;
    if(end == -1)
	end = l.count();
    else if(end < 0)
	end = l.count() + end;

    int i = 0;
    QString ret;
    for ( section_chunk *chk=l.first(); chk; chk=l.next(), i++ ) {
	if((flags & SectionSkipEmpty) && chk->length == (int)chk->string.length()) {
	    if(i <= start)
		start++;
	    end++;
	}
	if(i == start) {
	    ret = (flags & SectionIncludeLeadingSep) ? chk->string : chk->string.mid(chk->length);
	} else if(i > start) {
	    ret += chk->string;
	}
	if(i == end) {
	    if((chk=l.next()) && flags & SectionIncludeTrailingSep)
		ret += chk->string.left(chk->length);
	    break;
	}
    }
    return ret;
}
#endif

/*!
    \fn QString QString::section( char sep, int start, int end = 0xffffffff, int flags = SectionDefault ) const

    \overload
*/

/*!
    \fn QString QString::section( const char *sep, int start, int end = 0xffffffff, int flags = SectionDefault ) const

    \overload
*/


/*!
    Returns the number of times the character \a c occurs in the
    string.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.

    \code
    QString string( "Trolltech and Qt" );
    int i = string.contains( 't', FALSE );  // i == 3
    \endcode
*/

int QString::contains( QChar c, bool cs ) const
{
    int count = 0;
    const QChar *uc = unicode();
    if ( !uc )
	return 0;
    int n = length();
    if ( cs ) {
	while ( n-- )
	    if ( *uc++ == c )
		count++;
    } else {
	c = ::lower( c );
	while ( n-- ) {
	    if ( ::lower( *uc ) == c )
		count++;
	    uc++;
	}
    }
    return count;
}

/*!
    \overload

    Returns the number of times the string \a str occurs in the string.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.
*/
int QString::contains( const char* str, bool cs ) const
{
    return contains( QString(str), cs );
}

/*!
    \fn int QString::contains( char c, bool cs ) const

    \overload
*/

/*!
    \fn int QString::find( char c, int index, bool cs ) const

    \overload

    Find character \a c starting from position \a index.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.
*/

/*!
    \fn int QString::findRev( char c, int index, bool cs ) const

    \overload

    Find character \a c starting from position \a index and working
    backwards.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.
*/

/*!
    \overload

    Returns the number of times \a str occurs in the string.

    If \a cs is TRUE, the search is case sensitive; otherwise the
    search is case insensitive.

    This function counts overlapping strings, so in the example below,
    there are two instances of "ana" in "bananas".

    \code
    QString str( "bananas" );
    int i = str.contains( "ana" );  // i == 2
    \endcode

    \sa findRev()
*/

int QString::contains( const QString &str, bool cs ) const
{
    if ( isNull() )
	return 0;
    int count = 0;
    uint skiptable[0x100];
    bm_init_skiptable( str, skiptable, cs );
    int i = -1;
    // use boyer-moore for the ultimate speed experience
    while ( ( i = bm_find( *this, i+1, str, skiptable, cs ) ) != -1 )
	count++;
    return count;
}

/*!
    Returns a substring that contains the \a len leftmost characters
    of the string.

    The whole string is returned if \a len exceeds the length of the
    string.

    \code
	QString s = "Pineapple";
	QString t = s.left( 4 );    // t == "Pine"
    \endcode

    \sa right(), mid(), isEmpty()
*/

QString QString::left( uint len ) const
{
    if ( isEmpty() ) {
	return QString();
    } else if ( len == 0 ) {                    // ## just for 1.x compat:
	return fromLatin1( "" );
    } else if ( len >= length() ) {
	return *this;
    } else {
	QString s( len, TRUE );
	memcpy( s.d->unicode, d->unicode, len * sizeof(QChar) );
	s.d->len = len;
	return s;
    }
}

/*!
    Returns a string that contains the \a len rightmost characters of
    the string.

    If \a len is greater than the length of the string then the whole
    string is returned.

    \code
	QString string( "Pineapple" );
	QString t = string.right( 5 );   // t == "apple"
    \endcode

    \sa left(), mid(), isEmpty()
*/

QString QString::right( uint len ) const
{
    if ( isEmpty() ) {
	return QString();
    } else if ( len == 0 ) {                    // ## just for 1.x compat:
	return fromLatin1( "" );
    } else {
	uint l = length();
	if ( len >= l )
	    return *this;
	QString s( len, TRUE );
	memcpy( s.d->unicode, d->unicode+(l-len), len*sizeof(QChar) );
	s.d->len = len;
	return s;
    }
}

/*!
    Returns a string that contains the \a len characters of this
    string, starting at position \a index.

    Returns a null string if the string is empty or \a index is out of
    range. Returns the whole string from \a index if \a index + \a len
    exceeds the length of the string.

    \code
	QString s( "Five pineapples" );
	QString t = s.mid( 5, 4 );                  // t == "pine"
    \endcode

    \sa left(), right()
*/

QString QString::mid( uint index, uint len ) const
{
    uint slen = length();
    if ( isEmpty() || index >= slen ) {
	return QString();
    } else if ( len == 0 ) {                    // ## just for 1.x compat:
	return fromLatin1( "" );
    } else {
	if ( len > slen-index )
	    len = slen - index;
	if ( index == 0 && len == slen )
	    return *this;
	register const QChar *p = unicode()+index;
	QString s( len, TRUE );
	memcpy( s.d->unicode, p, len * sizeof(QChar) );
	s.d->len = len;
	return s;
    }
}

/*!
    Returns a string of length \a width that contains this string
    padded by the \a fill character.

    If \a truncate is FALSE and the length of the string is more than
    \a width, then the returned string is a copy of the string.

    If \a truncate is TRUE and the length of the string is more than
    \a width, then any characters in a copy of the string after length
    \a width are removed, and the copy is returned.

    \code
	QString s( "apple" );
	QString t = s.leftJustify( 8, '.' );        // t == "apple..."
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
    Returns a string of length \a width that contains the \a fill
    character followed by the string.

    If \a truncate is FALSE and the length of the string is more than
    \a width, then the returned string is a copy of the string.

    If \a truncate is TRUE and the length of the string is more than
    \a width, then the resulting string is truncated at position \a
    width.

    \code
	QString string( "apple" );
	QString t = string.rightJustify( 8, '.' );  // t == "...apple"
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
    Returns a lowercase copy of the string.

    \code
	QString string( "TROlltECH" );
	str = string.lower();   // str == "trolltech"
    \endcode

    \sa upper()
*/

QString QString::lower() const
{
    QString s(*this);
    int l=length();
    if ( l ) {
	s.real_detach(); // could do this only when we find a change
	register QChar *p=s.d->unicode;
	if ( p ) {
	    while ( l-- ) {
		*p = ::lower( *p );
		p++;
	    }
	}
    }
    return s;
}

/*!
    Returns an uppercase copy of the string.

    \code
	QString string( "TeXt" );
	str = string.upper();     // t == "TEXT"
    \endcode

    \sa lower()
*/

QString QString::upper() const
{
    QString s(*this);
    int l=length();
    if ( l ) {
	s.real_detach(); // could do this only when we find a change
	register QChar *p=s.d->unicode;
	if ( p ) {
	    while ( l-- ) {
		*p = ::upper( *p );
		p++;
	    }
	}
    }
    return s;
}


/*!
    Returns a string that has whitespace removed from the start and
    the end.

    Whitespace means any character for which QChar::isSpace() returns
    TRUE. This includes Unicode characters with decimal values 9
    (TAB), 10 (LF), 11 (VT), 12 (FF), 13 (CR) and 32 (Space), and may
    also include other Unicode characters.

    \code
	QString string = "   white space   ";
	QString s = string.stripWhiteSpace();       // s == "white space"
    \endcode

    \sa simplifyWhiteSpace()
*/

QString QString::stripWhiteSpace() const
{
    if ( isEmpty() )                            // nothing to do
	return *this;
    register const QChar *s = unicode();
    if ( !s->isSpace() && !s[length()-1].isSpace() )
	return *this;

    int start = 0;
    int end = length() - 1;
    while ( start<=end && s[start].isSpace() )  // skip white space from start
	start++;
    if ( start <= end ) {                          // only white space
	while ( end && s[end].isSpace() )           // skip white space from end
	    end--;
    }
    int l = end - start + 1;
    if ( l <= 0 )
    	return QString::fromLatin1("");

    QString result( l, TRUE );
    memcpy( result.d->unicode, &s[start], sizeof(QChar)*l );
    result.d->len = l;
    return result;
}


/*!
    Returns a string that has whitespace removed from the start and
    the end, and which has each sequence of internal whitespace
    replaced with a single space.

    Whitespace means any character for which QChar::isSpace() returns
    TRUE. This includes Unicode characters with decimal values 9
    (TAB), 10 (LF), 11 (VT), 12 (FF), 13 (CR), and 32 (Space).

    \code
	QString string = "  lots\t of\nwhite    space ";
	QString t = string.simplifyWhiteSpace();
	// t == "lots of white space"
    \endcode

    \sa stripWhiteSpace()
*/

QString QString::simplifyWhiteSpace() const
{
    if ( isEmpty() )
	return *this;
    QString result;
    result.setLength( length() );
    const QChar *from = unicode();
    const QChar *fromend = from+length();
    int outc=0;
    QChar *to   = result.d->unicode;
    for (;;) {
	while ( from!=fromend && from->isSpace() )
	    from++;
	while ( from!=fromend && !from->isSpace() )
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
    Inserts \a s into the string at position \a index.

    If \a index is beyond the end of the string, the string is
    extended with spaces to length \a index and \a s is then appended
    and returns a reference to the string.

    \code
	QString string( "I like fish" );
	str = string.insert( 2, "don't " );
	// str == "I don't like fish"
    \endcode

    \sa remove(), replace()
*/

QString &QString::insert( uint index, const QString &s )
{
    // the sub function takes care of &s == this case.
    return insert( index, s.unicode(), s.length() );
}

/*!
    \overload

    Inserts the character in \a s into the string at position \a index
    \a len number of times and returns a reference to the string.
*/

QString &QString::insert( uint index, const QChar* s, uint len )
{
    if ( len == 0 )
	return *this;
    uint olen = length();
    int nlen = olen + len;

    if ( s >= d->unicode && (uint)(s - d->unicode) < d->maxl ) {
	// Part of me - take a copy.
	QChar *tmp = QT_ALLOC_QCHAR_VEC( len );
	memcpy(tmp,s,len*sizeof(QChar));
	insert(index,tmp,len);
	QT_DELETE_QCHAR_VEC( tmp );
	return *this;
    }

    if ( index >= olen ) {                      // insert after end of string
	setLength( len + index );
	int n = index - olen;
	QChar* uc = d->unicode+olen;
	while (n--)
	    *uc++ = ' ';
	memcpy( d->unicode+index, s, sizeof(QChar)*len );
    } else {                                    // normal insert
	setLength( nlen );
	memmove( d->unicode + index + len, unicode() + index,
		 sizeof(QChar) * (olen - index) );
	memcpy( d->unicode + index, s, sizeof(QChar) * len );
    }
    return *this;
}

/*!
    \overload

    Insert \a c into the string at position \a index and returns a
    reference to the string.

    If \a index is beyond the end of the string, the string is
    extended with spaces (ASCII 32) to length \a index and \a c is
    then appended.
*/

QString &QString::insert( uint index, QChar c ) // insert char
{
    QString s( c );
    return insert( index, s );
}

/*!
    \fn QString& QString::insert( uint index, char c )

    \overload

    Insert character \a c at position \a index.
*/

/*!
    \fn QString &QString::prepend( const QString &s )

    Inserts \a s at the beginning of the string and returns a
    reference to the string.

    Equivalent to insert(0, \a s).

    \code
	QString string = "42";
	string.prepend( "The answer is " );
	// string == "The answer is 42"
    \endcode

    \sa insert()
*/

/*!
    \fn QString& QString::prepend( char ch )

    \overload

    Inserts \a ch at the beginning of the string and returns a
    reference to the string.

    Equivalent to insert(0, \a ch).

    \sa insert()
*/

/*!
    \fn QString& QString::prepend( QChar ch )

    \overload

    Inserts \a ch at the beginning of the string and returns a
    reference to the string.

    Equivalent to insert(0, \a ch).

    \sa insert()
*/

/*! \fn QString& QString::prepend( const QByteArray &s )
  \overload

  Inserts \a s at the beginning of the string and returns a reference to the string.

  Equivalent to insert(0, \a s).

  \sa insert()
 */

/*! \fn QString& QString::prepend( const std::string &s )
  \overload

  Inserts \a s at the beginning of the string and returns a reference to the string.

  Equivalent to insert(0, \a s).

  \sa insert()
 */

/*!
  \overload

  Inserts \a s at the beginning of the string and returns a reference to the string.

  Equivalent to insert(0, \a s).

  \sa insert()
 */
QString &QString::prepend( const char *s )
{
    return insert( 0, QString(s) );
}

/*!
    Removes \a len characters from the string starting at position \a
    index, and returns a reference to the string.

    If \a index is beyond the length of the string, nothing happens.
    If \a index is within the string, but \a index + \a len is beyond
    the end of the string, the string is truncated at position \a
    index.

    \code
	QString string( "Montreal" );
	string.remove( 1, 4 );      // string == "Meal"
    \endcode

    \sa insert(), replace()
*/

QString &QString::remove( uint index, uint len )
{
    uint olen = length();
    if ( index >= olen  ) {
	// range problems
    } else if ( index + len >= olen ) {  // index ok
	setLength( index );
    } else if ( len != 0 ) {
	real_detach();
	memmove( d->unicode+index, d->unicode+index+len,
		 sizeof(QChar)*(olen-index-len) );
	setLength( olen-len );
    }
    return *this;
}

/*! \overload

    Removes every occurrence of the character \a c in the string.
    Returns a reference to the string.

    This is the same as replace(\a c, "").
*/
QString &QString::remove( QChar c )
{
    int i = 0;
    while ( i < (int) length() ) {
	if ( constref(i) == c ) {
	    remove( i, 1 );
	} else {
	    i++;
	}
    }
    return *this;
}

/*! \overload

    \fn QString &QString::remove( char c )

    Removes every occurrence of the character \a c in the string.
    Returns a reference to the string.

    This is the same as replace(\a c, "").
*/

/*! \overload

    Removes every occurrence of \a str in the string. Returns a
    reference to the string.

    This is the same as replace(\a str, "").
*/
QString &QString::remove( const QString & str )
{
    int index = 0;
    if ( !str.isEmpty() ) {
	while ( (index = find(str, index)) != -1 )
	    remove( index, str.length() );
    }
    return *this;
}

/*! \overload

  Replaces every occurrence of \a c1 with the char \a c2.
  Returns a reference to the string.
*/
QString &QString::replace( QChar c1, QChar c2 )
{
     real_detach();
     uint i = 0;
     while ( i < d->len ) {
	  if ( d->unicode[i] == c1 )
	       d->unicode[i] = c2;
	  i++;
     }
     return *this;
}


#ifndef QT_NO_REGEXP_CAPTURE

/*! \overload

    Removes every occurrence of the regular expression \a rx in the
    string. Returns a reference to the string.

    This is the same as replace(\a rx, "").
*/

QString &QString::remove( const QRegExp & rx )
{
    return replace( rx, QString::null );
}

#endif

/*! \overload

    Removes every occurrence of \a str in the string. Returns a
    reference to the string.
*/
QString &QString::remove( const char *str )
{
    return remove( QString::fromLatin1(str) );
}

/*!
    Replaces \a len characters from the string with \a s, starting at
    position \a index, and returns a reference to the string.

    If \a index is beyond the length of the string, nothing is deleted
    and \a s is appended at the end of the string. If \a index is
    valid, but \a index + \a len is beyond the end of the string,
    the string is truncated at position \a index, then \a s is
    appended at the end.

    \code
	QString string( "Say yes!" );
	string = string.replace( 4, 3, "NO" );
	// string == "Say NO!"
    \endcode

    \sa insert(), remove()
*/

QString &QString::replace( uint index, uint len, const QString &s )
{
    return replace( index, len, s.unicode(), s.length() );
}

/*! \overload

    This is the same as replace(\a index, \a len, QString(\a c)).
*/
QString &QString::replace( uint index, uint len, QChar c )
{
    return replace( index, len, &c, 1 );
}

/*! \overload
    \fn QString &QString::replace( uint index, uint len, char c )

    This is the same as replace(\a index, \a len, QChar(\a c)).
*/

/*!
    \overload

    Replaces \a len characters with \a slen characters of QChar data
    from \a s, starting at position \a index, and returns a reference
    to the string.

    \sa insert(), remove()
*/

QString &QString::replace( uint index, uint len, const QChar* s, uint slen )
{
    real_detach();
    if ( len == slen && index + len <= length() ) {
	// Optimized common case: replace without size change
	memcpy( d->unicode+index, s, len * sizeof(QChar) );
    } else if ( s >= d->unicode && (uint)(s - d->unicode) < d->maxl ) {
	// Part of me - take a copy.
	QChar *tmp = QT_ALLOC_QCHAR_VEC( slen );
	memcpy( tmp, s, slen * sizeof(QChar) );
	replace( index, len, tmp, slen );
	QT_DELETE_QCHAR_VEC( tmp );
    } else {
	remove( index, len );
	insert( index, s, slen );
    }
    return *this;
}

/*! \overload

    Replaces every occurrence of the character \a c in the string
    with \a after. Returns a reference to the string.

    Example:
    \code
    QString s = "a,b,c";
    s.replace( QChar(','), " or " );
    // s == "a or b or c"
    \endcode
*/
QString &QString::replace( QChar c, const QString & after )
{
    return replace( QString( c ), after );
}

/*! \overload
    \fn QString &QString::replace( char c, const QString & after )

    Replaces every occurrence of the character \a c in the string
    with \a after. Returns a reference to the string.
*/

/*! \overload

    Replaces every occurrence of the string \a before in the string
    with the string \a after. Returns a reference to the string.

    Example:
    \code
    QString s = "Greek is Greek";
    s.replace( "Greek", "English" );
    // s == "English is English"
    \endcode
*/
QString &QString::replace( const QString & before, const QString & after )
{
    if ( before == after || isNull() )
	return *this;

    real_detach();

    int index = 0;
    uint skiptable[256];
    bm_init_skiptable( before, skiptable, TRUE );
    const int bl = before.length();
    const int al = after.length();

    if ( bl == al ) {
	if ( bl ) {
	    const QChar *auc = after.unicode();
	    while( (index = bm_find(*this, index, before, skiptable, TRUE) ) != -1 ) {
		memcpy( d->unicode+index, auc, al*sizeof(QChar) );
		index += bl;
	    }
	}
    } else if ( al < bl ) {
	const QChar *auc = after.unicode();
	uint to = 0;
	uint movestart = 0;
	uint num = 0;
	while( (index = bm_find(*this, index, before, skiptable, TRUE) ) != -1 ) {
	    if ( num ) {
		int msize = index - movestart;
		if ( msize > 0 ) {
		    memmove( d->unicode + to, d->unicode + movestart, msize*sizeof(QChar) );
		    to += msize;
		}
	    } else {
		to = index;
	    }
	    if ( al ) {
		memcpy( d->unicode+to, auc, al*sizeof(QChar) );
		to += al;
	    }
	    index += bl;
	    movestart = index;
	    num++;
	}
	if ( num ) {
	    int msize = d->len - movestart;
	    if ( msize > 0 )
		memmove( d->unicode + to, d->unicode + movestart, msize*sizeof(QChar) );
	    setLength( d->len - num*(bl-al) );
	}
    } else {
	// the most complex case. We don't want to loose performance by doing repeated
	// copies and reallocs of the string.
	while( index != -1 ) {
	    uint indices[4096];
	    uint pos = 0;
	    while( pos < 4095 ) {
		index = bm_find(*this, index, before, skiptable, TRUE);
		if ( index == -1 )
		    break;
		indices[pos++] = index;
		index += bl;
		// avoid infinite loop
		if ( !bl )
		    index++;
	    }
	    if ( !pos )
		break;

	    // we have a table of replacement positions, use them for fast replacing
	    int adjust = pos*(al-bl);
	    // index has to be adjusted in case we get back into the loop above.
	    if ( index != -1 )
		index += adjust;
	    uint newlen = d->len + adjust;
	    int moveend = d->len;
	    if ( newlen > d->len )
		setLength( newlen );

	    while( pos ) {
		pos--;
		int movestart = indices[pos] + bl;
		int insertstart = indices[pos] + pos*(al-bl);
		int moveto = insertstart + al;
		memmove( d->unicode + moveto, d->unicode + movestart, (moveend - movestart)*sizeof(QChar) );
		memcpy( d->unicode + insertstart, after.unicode(), al*sizeof(QChar) );
		moveend = movestart-bl;
	    }
	}
    }
    return *this;
}

#ifndef QT_NO_REGEXP_CAPTURE
/*! \overload

  Replaces every occurrence of the regexp \a rx in the string with \a str.
  Returns a reference to the string. For example:
  \code
    QString s = "banana";
    s.replace( QRegExp("an"), "" );
    // s == "ba"
  \endcode

  For regexps containing \link qregexp.html#capturing-text capturing
  parentheses \endlink, occurrences of <b>\\1</b>, <b>\\2</b>, ...,
  in \a str are replaced with \a{rx}.cap(1), cap(2), ...

  \code
    QString t = "A <i>bon mot</i>.";
    t.replace( QRegExp("<i>([^<]*)</i>"), "\\emph{\\1}" );
    // t == "A \\emph{bon mot}."
  \endcode

  \sa find(), findRev(), QRegExp::cap()
*/

QString &QString::replace( const QRegExp &rx, const QString &str )
{
    if ( isNull() )
	return *this;

    real_detach();

    QRegExp rx2 = rx;
    int index = 0;
    int numCaptures = rx2.numCaptures();
    int al = str.length();
    QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;

    if ( numCaptures > 0 ) {
	if ( numCaptures > 9 )
	    numCaptures = 9;

	const QChar *uc = str.unicode();
	int numBackRefs = 0;

	for ( int i = 0; i < al - 1; i++ ) {
	    if ( uc[i] == '\\' ) {
		int no = uc[i + 1].digitValue();
		if ( no > 0 && no <= numCaptures )
		    numBackRefs++;
	    }
	}

	/*
	  This is the harder case where we have back-references. We
	  don't try to optimize it.
	*/
	if ( numBackRefs > 0 ) {
	    int *capturePositions = new int[numBackRefs];
	    int *captureNumbers = new int[numBackRefs];
	    int j = 0;

	    for ( int i = 0; i < al - 1; i++ ) {
		if ( uc[i] == '\\' ) {
		    int no = uc[i + 1].digitValue();
		    if ( no > 0 && no <= numCaptures ) {
			capturePositions[j] = i;
			captureNumbers[j] = no;
			j++;
		    }
		}
	    }

	    while ( index <= (int)length() ) {
		index = rx2.search( *this, index, caretMode );
		if ( index == -1 )
		    break;

		QString str2 = str;
		for ( j = numBackRefs - 1; j >= 0; j-- )
		    str2.replace( capturePositions[j], 2,
				  rx2.cap(captureNumbers[j]) );

		replace( index, rx2.matchedLength(), str2 );
		index += str2.length();

		if ( rx2.matchedLength() == 0 ) {
		    // avoid infinite loop on 0-length matches (e.g., [a-z]*)
		    index++;
		} else if ( index == 0 ) {
		    caretMode = QRegExp::CaretWontMatch;
		}
	    }
	    delete[] capturePositions;
	    delete[] captureNumbers;
	    return *this;
	}
    }

    /*
      This is the simple and optimized case where we don't have
      back-references.
    */
    while ( index != -1 ) {
	struct {
	    int pos;
	    int length;
	} replacements[2048];

	uint pos = 0;
	int adjust = 0;
	while( pos < 2047 ) {
	    index = rx2.search( *this, index, caretMode );
	    if ( index == -1 )
		break;
	    int ml = rx2.matchedLength();
	    replacements[pos].pos = index;
	    replacements[pos++].length = ml;
	    index += ml;
	    adjust += al - ml;
	    // avoid infinite loop
	    if ( !ml )
		index++;
	}
	if ( !pos )
	    break;
	replacements[pos].pos = d->len;
	uint newlen = d->len + adjust;

	// to continue searching at the right position after we did
	// the first round of replacements
	if ( index != -1 )
	    index += adjust;
	QChar *newuc = QT_ALLOC_QCHAR_VEC( newlen + 1 );
	QChar *uc = newuc;
	int copystart = 0;
	uint i = 0;
	while( i < pos ) {
	    int copyend = replacements[i].pos;
	    int size = copyend - copystart;
	    memcpy( uc, d->unicode + copystart, size*sizeof(QChar) );
	    uc += size;
	    memcpy( uc, str.unicode(), al*sizeof( QChar ) );
	    uc += al;
	    copystart = copyend + replacements[i].length;
	    i++;
	}
	memcpy( uc, d->unicode + copystart,
		(d->len - copystart) * sizeof(QChar) );
	QT_DELETE_QCHAR_VEC( d->unicode );
	d->unicode = newuc;
	d->len = newlen;
	d->maxl = newlen + 1;
	d->setDirty();
	caretMode = QRegExp::CaretWontMatch;
    }
    return *this;
}
#endif

#ifndef QT_NO_REGEXP
/*!
    Finds the first match of the regular expression \a rx, starting
    from position \a index. If \a index is -1, the search starts at
    the last character; if -2, at the next to last character and so
    on. (See findRev() for searching backwards.)

    Returns the position of the first match of \a rx or -1 if no match
    was found.

    \code
	QString string( "bananas" );
	int i = string.find( QRegExp("an"), 0 );    // i == 1
    \endcode

    \sa findRev() replace() contains()
*/

int QString::find( const QRegExp &rx, int index ) const
{
    return rx.search( *this, index );
}

/*!
    \overload

    Finds the first match of the regexp \a rx, starting at position \a
    index and searching backwards. If the index is -1, the search
    starts at the last character, if it is -2, at the next to last
    character and so on. (See findRev() for searching backwards.)

    Returns the position of the match or -1 if no match was found.

    \code
	QString string( "bananas" );
	int i = string.findRev( QRegExp("an") );      // i == 3
    \endcode

    \sa find()
*/

int QString::findRev( const QRegExp &rx, int index ) const
{
    return rx.searchRev( *this, index );
}

/*!
    \overload

    Returns the number of times the regexp, \a rx, matches in the
    string.

    This function counts overlapping matches, so in the example below,
    there are four instances of "ana" or "ama".

    \code
	QString str = "banana and panama";
	QRegExp rxp = QRegExp( "a[nm]a", TRUE, FALSE );
	int i = str.contains( rxp );    // i == 4
    \endcode

    \sa find() findRev()
*/

int QString::contains( const QRegExp &rx ) const
{
    int count = 0;
    int index = -1;
    int len = length();
    while ( index < len - 1 ) {                 // count overlapping matches
	index = rx.search( *this, index + 1 );
	if ( index == -1 )
	    break;
	count++;
    }
    return count;
}

#endif //QT_NO_REGEXP

static bool ok_in_base( QChar c, int base )
{
    if ( base <= 10 )
	return c.isDigit() && c.digitValue() < base;
    else
	return c.isDigit() || (c >= 'a' && c < char('a'+base-10))
			   || (c >= 'A' && c < char('A'+base-10));
}

/*!
    Returns the string converted to a \c long value to the base \a
    base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

long QString::toLong( bool *ok, int base ) const
{
    const QChar *p = unicode();
    long val = 0;
    int l = length();
    const long max_mult = INT_MAX / base;
    bool is_ok = FALSE;
    int neg = 0;
    if ( !p )
	goto bye;
    while ( l && p->isSpace() )                 // skip leading space
	l--,p++;
    if ( !l )
	goto bye;
    if ( *p == '-' ) {
	l--;
	p++;
	neg = 1;
    } else if ( *p == '+' ) {
	l--;
	p++;
    }

    // NOTE: toULong() code is similar
    if ( !l || !ok_in_base(*p,base) )
	goto bye;
    while ( l && ok_in_base(*p,base) ) {
	l--;
	int dv;
	if ( p->isDigit() ) {
	    dv = p->digitValue();
	} else {
	    if ( *p >= 'a' && *p <= 'z' )
		dv = *p - 'a' + 10;
	    else
		dv = *p - 'A' + 10;
	}
	if ( val > max_mult ||
	    (val == max_mult && dv > (INT_MAX % base) + neg) )
	    goto bye;
	val = base * val + dv;
	p++;
    }
    if ( neg )
	val = -val;
    while ( l && p->isSpace() )                 // skip trailing space
	l--,p++;
    if ( !l )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
    Returns the string converted to an \c {unsigned long} value to the
    base \a base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

ulong QString::toULong( bool *ok, int base ) const
{
    const QChar *p = unicode();
    ulong val = 0;
    int l = length();
    const ulong max_mult = UINT_MAX / base;
    bool is_ok = FALSE;
    if ( !p )
	goto bye;
    while ( l && p->isSpace() )                 // skip leading space
	l--,p++;
    if ( !l )
	goto bye;
    if ( *p == '+' )
	l--,p++;

    // NOTE: toLong() code is similar
    if ( !l || !ok_in_base(*p,base) )
	goto bye;
    while ( l && ok_in_base(*p,base) ) {
	l--;
	uint dv;
	if ( p->isDigit() ) {
	    dv = p->digitValue();
	} else {
	    if ( *p >= 'a' && *p <= 'z' )
		dv = *p - 'a' + 10;
	    else
		dv = *p - 'A' + 10;
	}
	if ( val > max_mult || (val == max_mult && dv > UINT_MAX % base) )
	    goto bye;
	val = base * val + dv;
	p++;
    }

    while ( l && p->isSpace() )                 // skip trailing space
	l--,p++;
    if ( !l )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
    Returns the string converted to a \c short value to the base \a
    base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.
*/

short QString::toShort( bool *ok, int base ) const
{
    long v = toLong( ok, base );
    if ( ok && *ok && (v < -32768 || v > 32767) ) {
	*ok = FALSE;
	v = 0;
    }
    return (short)v;
}

/*!
    Returns the string converted to an \c {unsigned short} value to
    the base \a base, which is 10 by default and must be between 2 and
    36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.
*/

ushort QString::toUShort( bool *ok, int base ) const
{
    ulong v = toULong( ok, base );
    if ( ok && *ok && (v > 65535) ) {
	*ok = FALSE;
	v = 0;
    }
    return (ushort)v;
}


/*!
    Returns the string converted to an \c int value to the base \a
    base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \code
	QString str( "FF" );
	bool ok;
	int hex = str.toInt( &ok, 16 );     // hex == 255, ok == TRUE
	int dec = str.toInt( &ok, 10 );     // dec == 0, ok == FALSE
    \endcode

    \sa number()
*/

int QString::toInt( bool *ok, int base ) const
{
    return (int)toLong( ok, base );
}

/*!
    Returns the string converted to an \c{unsigned int} value to the
    base \a base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

uint QString::toUInt( bool *ok, int base ) const
{
    return (uint)toULong( ok, base );
}

/*!
    Returns the string converted to a \c double value.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \code
	QString string( "1234.56" );
	double a = string.toDouble();   // a == 1234.56
    \endcode

    \sa number()
*/

double QString::toDouble( bool *ok ) const
{
    char *end;

    const char *a = latin1();
    double val = strtod( a ? a : "", &end );
    if ( ok )
	*ok = ( a && *a && (end == 0 || (end - a) == (int)length()) );
    return val;
}

/*!
    Returns the string converted to a \c float value.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

float QString::toFloat( bool *ok ) const
{
    return (float)toDouble( ok );
}


/*!
    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.

    \code
	QString string;
	string = string.setNum( 1234 );     // string == "1234"
    \endcode
*/

QString &QString::setNum( long n, int base )
{
#if defined(QT_CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	qWarning( "QString::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    char   charbuf[65*sizeof(QChar)];
    QChar *buf = (QChar*)charbuf;
    QChar *p = &buf[64];
    int  len = 0;
    bool neg;
    if ( n < 0 ) {
	neg = TRUE;
	if ( n == INT_MIN ) {
	    // Cannot always negate this special case
	    QString s1, s2;
	    s1.setNum(n/base);
	    s2.setNum((-(n+base))%base);
	    *this = s1 + s2;
	    return *this;
	}
	n = -n;
    } else {
	neg = FALSE;
    }
    do {
	*--p = "0123456789abcdefghijklmnopqrstuvwxyz"[((int)(n%base))];
	n /= base;
	++len;
    } while ( n );
    if ( neg ) {
	*--p = '-';
	++len;
    }
    return setUnicode( p, len );
}

/*!
    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

QString &QString::setNum( ulong n, int base )
{
#if defined(QT_CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	qWarning( "QString::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    char   charbuf[65*sizeof(QChar)];
    QChar *buf = (QChar*)charbuf;
    QChar *p = &buf[64];
    int len = 0;
    do {
	*--p = "0123456789abcdefghijklmnopqrstuvwxyz"[((int)(n%base))];
	n /= base;
	len++;
    } while ( n );
    return setUnicode(p,len);
}

/*!
    \fn QString &QString::setNum( int n, int base )

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum( uint n, int base )

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum( short n, int base )

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum( ushort n, int base )

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \overload

    Sets the string to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    string.

    The format \a f can be 'f', 'F', 'e', 'E', 'g' or 'G'. See \link
    #arg-formats arg \endlink() for an explanation of the formats.
*/

QString &QString::setNum( double n, char f, int prec )
{
#if defined(QT_CHECK_RANGE)
    if ( !(f=='f' || f=='F' || f=='e' || f=='E' || f=='g' || f=='G') ) {
	qWarning( "QString::setNum: Invalid format char '%c'", f );
	f = 'f';
    }
#endif
    char format[20];
    char *fs = format; // generate format string: %.<prec>l<f>
    *fs++ = '%';
    if ( prec >= 0 ) {
	if ( prec > 99 ) // rather than crash in sprintf()
	    prec = 99;
	*fs++ = '.';
	if ( prec >= 10 ) {
	    *fs++ = prec / 10 + '0';
	    *fs++ = prec % 10 + '0';
	} else {
	    *fs++ = prec + '0';
	}
    }
    *fs++ = 'l';
    *fs++ = f;
    *fs = '\0';
#ifndef QT_NO_SPRINTF
    sprintf( format, n );
    return *this;
#else
    char buf[512];
    ::sprintf( buf, format, n );        // snprintf is unfortunately not portable
    return setLatin1(buf);
#endif
}

/*!
    \fn QString &QString::setNum( float n, char f, int prec )

    \overload

    Sets the string to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    string.

    The format \a f can be 'f', 'F', 'e', 'E', 'g' or 'G'. See \link
    #arg-formats arg \endlink() for an explanation of the formats.
*/


/*!
    A convenience function that returns a string equivalent of the
    number \a n to base \a base, which is 10 by default and must be
    between 2 and 36.

    \code
	long a = 63;
	QString str = QString::number( a, 16 );             // str == "3f"
	QString str = QString::number( a, 16 ).upper();     // str == "3F"
    \endcode

    \sa setNum()
*/
QString QString::number( long n, int base )
{
    QString s;
    s.setNum( n, base );
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QString QString::number( ulong n, int base )
{
    QString s;
    s.setNum( n, base );
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QString QString::number( int n, int base )
{
    QString s;
    s.setNum( n, base );
    return s;
}

/*!
    \overload

    A convenience factory function that returns a string
    representation of the number \a n to the base \a base, which is 10
    by default and must be between 2 and 36.

    \sa setNum()
*/
QString QString::number( uint n, int base )
{
    QString s;
    s.setNum( n, base );
    return s;
}

/*!
    \overload

    Argument \a n is formatted according to the \a f format specified,
    which is \c g by default, and can be any of the following:

    \table
    \header \i Format \i Meaning
    \row \i \c e \i format as [-]9.9e[+|-]999
    \row \i \c E \i format as [-]9.9E[+|-]999
    \row \i \c f \i format as [-]9.9
    \row \i \c g \i use \c e or \c f format, whichever is the most concise
    \row \i \c G \i use \c E or \c f format, whichever is the most concise
    \endtable

    With 'e', 'E', and 'f', \a prec is the number of digits after the
    decimal point. With 'g' and 'G', \a prec is the maximum number of
    significant digits (trailing zeroes are omitted).

    \code
    double d = 12.34;
    QString ds = QString( "'E' format, precision 3, gives %1" )
		    .arg( d, 0, 'E', 3 );
    // ds == "1.234E+001"
    \endcode

    \sa setNum()
    */
QString QString::number( double n, char f, int prec )
{
    QString s;
    s.setNum( n, f, prec );
    return s;
}


/*! \obsolete

  Sets the character at position \a index to \a c and expands the
  string if necessary, filling with spaces.

  This method is redundant in Qt 3.x, because operator[] will expand
  the string as necessary.
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

  \obsolete

  Returns a pointer to a '\0'-terminated classic C string.

  In Qt 1.x, this returned a char* allowing direct manipulation of the
  string as a sequence of bytes. In Qt 2.x where QString is a Unicode
  string, char* conversion constructs a temporary string, and hence
  direct character operations are meaningless.
*/

/*!
    \fn bool QString::operator!() const

    Returns TRUE if this is a null string; otherwise returns FALSE.

    \code
	QString name = getName();
	if ( !name )
	    name = "Rodney";
    \endcode

    Note that if you say

    \code
	QString name = getName();
	if ( name )
	    doSomethingWith(name);
    \endcode

    It will call "operator const char*()", which is inefficent; you
    may wish to define the macro \c QT_NO_ASCII_CAST when writing code
    which you wish to remain Unicode-clean.

    When you want the above semantics, use:

    \code
	QString name = getName();
	if ( !name.isNull() )
	    doSomethingWith(name);
    \endcode

    \sa isEmpty()
*/


/*!
    \fn QString& QString::append( const QString& str )

    Appends \a str to the string and returns a reference to the
    result.

    \code
	string = "Test";
	string.append( "ing" );        // string == "Testing"
    \endcode

    Equivalent to operator+=().
*/

/*!
    \fn QString& QString::append( char ch )

    \overload

    Appends character \a ch to the string and returns a reference to
    the result.

    Equivalent to operator+=().
*/

/*!
    \fn QString& QString::append( QChar ch )

    \overload

    Appends character \a ch to the string and returns a reference to
    the result.

    Equivalent to operator+=().
*/

/*! \fn QString& QString::append( const QByteArray &str )
  \overload

  Appends \a str to the string and returns a reference to the result.

  Equivalent to operator+=().
 */

/*! \fn QString& QString::append( const std::string &str )
  \overload

  Appends \a str to the string and returns a reference to the result.

  Equivalent to operator+=().
 */


/*! \fn QString& QString::append( const char *str )
  \overload

  Appends \a str to the string and returns a reference to the result.

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
    } else if ( isNull() && !str.isNull() ) {   // ## just for 1.x compat:
	*this = fromLatin1( "" );
    }
    return *this;
}

/*!
    \overload

  Appends \a str to the string and returns a reference to the string.
*/
QString& QString::operator+=( const char *str )
{
    if ( str ) {
#ifndef QT_NO_TEXTCODEC
	if ( QTextCodec::codecForCStrings() )
	    return operator+=( fromAscii( str ) );
#endif

	uint len1 = length();
	uint len2 = strlen( str );
	if ( len2 ) {
	    setLength(len1+len2);
	    uint i = 0;
	    while( i < len2 ) {
		d->unicode[len1+i] = str[i];
		i++;
	    }
	} else if ( isNull() ) {   // ## just for 1.x compat:
	    *this = fromLatin1( "" );
	}
    }
    return *this;
}

/*! \overload

  Appends \a c to the string and returns a reference to the string.
*/

QString &QString::operator+=( QChar c )
{
    setLength(length()+1);
    d->unicode[length()-1] = c;
    return *this;
}

/*!
    \overload

    Appends \a c to the string and returns a reference to the string.
*/

QString &QString::operator+=( char c )
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() )
	return operator+=( fromAscii( &c, 1 ) );
#endif
    setLength(length()+1);
    d->unicode[length()-1] = c;
    return *this;
}

/*!
  \fn QString &QString::operator+=( const QByteArray &str )
  \overload

  Appends \a str to the string and returns a reference to the string.
*/

/*!
  \fn QString &QString::operator+=( const std::string &str )
  \overload

  Appends \a str to the string and returns a reference to the string.
*/



/*!
    \fn char QChar::latin1() const

    Returns the Latin-1 value of this character, or 0 if it
    cannot be represented in Latin-1.
*/


/*!
    Returns a Latin-1 representation of the string. The
    returned value is undefined if the string contains non-Latin-1
    characters. If you want to convert strings into formats other than
    Unicode, see the QTextCodec classes.

    This function is mainly useful for boot-strapping legacy code to
    use Unicode.

    The result remains valid so long as one unmodified copy of the
    source string exists.

    \sa fromLatin1(), ascii(), utf8(), local8Bit()
*/
const char* QString::latin1() const
{
    if ( !d->ascii  || !d->islatin1 ) {
	d->ascii = unicodeToLatin1( d->unicode, d->len );
	d->islatin1 = TRUE;
    }
    return d->ascii;
}

/*!
    Returns an 8-bit ASCII representation of the string.

    If a codec has been set using QTextCodec::codecForCStrings(),
    it is used to convert Unicode to 8-bit char. Otherwise, this function
    does the same as latin1().

    \sa fromAscii(), latin1(), utf8(), local8Bit()
*/
const char* QString::ascii() const
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	if ( !d->ascii || d->islatin1 ) {
	    QCString s = QTextCodec::codecForCStrings()->fromUnicode( *this );
	    s.detach();
	    d->ascii = s.data();
	    d->islatin1 = FALSE;
	    s.resetRawData( s.data(), s.size() ); // we have stolen the data
	}
	return d->ascii;
    }
#endif // QT_NO_TEXTCODEC
    return latin1();
}

/*!
    Returns the string encoded in UTF-8 format.

    See QTextCodec for more diverse coding/decoding of Unicode strings.

    \sa fromUtf8(), ascii(), latin1(), local8Bit()
*/
QCString QString::utf8() const
{
    int l = length();
    int rlen = l*3+1;
    QCString rstr(rlen);
    uchar* cursor = (uchar*)rstr.data();
    const QChar *ch = d->unicode;
    for (int i=0; i<l; i++) {
	ushort u = ch->unicode();
 	if ( u < 0x80 ) {
 	    *cursor++ = (uchar)u;
 	} else {
 	    if ( u < 0x0800 ) {
		*cursor++ = 0xc0 | ((uchar) (u >> 6));
 	    } else {
 		*cursor++ = 0xe0 | ((uchar) (u >> 12));
 		*cursor++ = 0x80 | ( ((uchar) (u >> 6)) & 0x3f);
 	    }
 	    *cursor++ = 0x80 | ((uchar) (u&0x3f));
 	}
 	ch++;
    }
    rstr.truncate( cursor - (uchar*)rstr.data() );
    return rstr;
}

/*!
    Returns the Unicode string decoded from the first \a len
    characters of \a utf8, ignoring the rest of \a utf8. If \a len is
    -1 then the length of \a utf8 is used. If \a len is bigger than
    the length of \a utf8 then it will use the length of \a utf8.

    \code
	QString str = QString::fromUtf8( "123456789", 5 );
	// str == "12345"
    \endcode

    See QTextCodec for more diverse coding/decoding of Unicode strings.
*/
QString QString::fromUtf8( const char* utf8, int len )
{
    if ( !utf8 )
	return QString::null;

    if ( len < 0 )
	len = strlen( utf8 );
    QString result;
    result.setLength( len ); // worst case
    QChar *qch = (QChar *)result.unicode();
    ushort uc = 0;
    int need = 0;
    for (int i=0; i<len; i++) {
	uchar ch = utf8[i];
	if (need) {
	    if ( (ch&0xc0) == 0x80 ) {
		uc = (uc << 6) | (ch & 0x3f);
		need--;
		if ( !need ) {
		    *qch = uc;
		    qch++;
		}
	    } else {
		// error
		*qch = QChar::replacement;
		qch++;
		need = 0;
	    }
	} else {
	    if ( ch < 128 ) {
		*qch = ch;
		qch++;
	    } else if ( (ch&0xe0) == 0xc0 ) {
		uc = ch &0x1f;
		need = 1;
	    } else if ( (ch&0xf0) == 0xe0 ) {
		uc = ch &0x0f;
		need = 2;
	    }
	}
    }
    result.truncate( qch - result.unicode() );
    return result;
}

/*!
    Returns the Unicode string decoded from the first \a len
    characters of \a ascii, ignoring the rest of \a ascii. If \a len
    is -1 then the length of \a ascii is used. If \a len is bigger
    than the length of \a ascii then it will use the length of \a
    ascii.

    If a codec has been set using QTextCodec::codecForCStrings(),
    it is used to convert Unicode to 8-bit char. Otherwise, this function
    does the same as fromLatin1().

    This is the same as the QString(const char*) constructor, but you
    can make that constructor invisible if you compile with the define
    \c QT_NO_CAST_ASCII, in which case you can explicitly create a
    QString from 8-bit ASCII text using this function.

    \code
	QString str = QString::fromAscii( "123456789", 5 );
	// str == "12345"
    \endcode
 */
QString QString::fromAscii( const char* ascii, int len )
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	if ( !ascii )
	    return QString::null;
	if ( len < 0 )
	    len = strlen( ascii );
	if ( len == 0 || *ascii == '\0' )
	    return QString::fromLatin1( "" );
	return QTextCodec::codecForCStrings()->toUnicode( ascii, len );
    }
#endif
    return fromLatin1( ascii, len );
}


/*!
    Returns the Unicode string decoded from the first \a len
    characters of \a chars, ignoring the rest of \a chars. If \a len
    is -1 then the length of \a chars is used. If \a len is bigger
    than the length of \a chars then it will use the length of \a
    chars.

    \sa fromAscii()
*/
QString QString::fromLatin1( const char* chars, int len )
{
    uint l;
    QChar *uc;
    if ( len < 0 )
	 len = -1;
    uc = internalLatin1ToUnicode( chars, &l, len );
    return QString( new QStringData(uc, l, l), TRUE );
}

/*!
    \fn const QChar* QString::unicode() const

    Returns the Unicode representation of the string. The result
    remains valid until the string is modified.
*/

/*!
    Returns the string encoded in a locale-specific format. On X11,
    this is the QTextCodec::codecForLocale(). On Windows, it is a
    system-defined encoding. On Mac OS X, this always uses UTF-8 as
    the encoding.

    See QTextCodec for more diverse coding/decoding of Unicode
    strings.

    \sa fromLocal8Bit(), ascii(), latin1(), utf8()
*/

QCString QString::local8Bit() const
{
#ifdef QT_NO_TEXTCODEC
    return latin1();
#else
#ifdef Q_WS_X11
    QTextCodec* codec = QTextCodec::codecForLocale();
    return codec
	    ? codec->fromUnicode(*this)
	    : QCString(latin1());
#endif
#if defined( Q_WS_MACX )
    return utf8();
#endif
#if defined( Q_WS_MAC9 )
    return QCString(latin1()); //I'm evil..
#endif
#ifdef Q_WS_WIN
    return qt_winQString2MB( *this );
#endif
#ifdef Q_WS_QWS
    return utf8(); // ### if there is any 8 bit format supported?
#endif
#endif
}

/*!
    Returns the Unicode string decoded from the first \a len
    characters of \a local8Bit, ignoring the rest of \a local8Bit. If
    \a len is -1 then the length of \a local8Bit is used. If \a len is
    bigger than the length of \a local8Bit then it will use the length
    of \a local8Bit.

    \code
	QString str = QString::fromLocal8Bit( "123456789", 5 );
	// str == "12345"
    \endcode

    \a local8Bit is assumed to be encoded in a locale-specific format.

    See QTextCodec for more diverse coding/decoding of Unicode strings.
*/
QString QString::fromLocal8Bit( const char* local8Bit, int len )
{
#ifdef QT_NO_TEXTCODEC
    return fromLatin1( local8Bit, len );
#else

    if ( !local8Bit )
	return QString::null;
#ifdef Q_WS_X11
    QTextCodec* codec = QTextCodec::codecForLocale();
    if ( len < 0 )
	len = strlen( local8Bit );
    return codec
	    ? codec->toUnicode( local8Bit, len )
	    : fromLatin1( local8Bit, len );
#endif
#if defined( Q_WS_MAC )
    return fromUtf8(local8Bit,len);
#endif
// Should this be OS_WIN32?
#ifdef Q_WS_WIN
    if ( len >= 0 ) {
	QCString s(local8Bit,len+1);
	return qt_winMB2QString(s);
    }
    return qt_winMB2QString( local8Bit );
#endif
#ifdef Q_WS_QWS
    return fromUtf8(local8Bit,len);
#endif
#endif // QT_NO_TEXTCODEC
}

/*!
    \fn QString::operator const char *() const

    Returns latin1(). Be sure to see the warnings documented in the
    latin1() function. Note that for new code which you wish to be
    strictly Unicode-clean, you can define the macro \c
    QT_NO_ASCII_CAST when compiling your code to hide this function so
    that automatic casts are not done. This has the added advantage
    that you catch the programming error described in operator!().
*/

/*!
    \fn QString::operator std::string() const

    Returns ascii().
*/


/*!
    Returns the QString as a zero terminated array of unsigned shorts
    if the string is not null; otherwise returns zero.

    The result remains valid so long as one unmodified
    copy of the source string exists.
*/
const unsigned short *QString::ucs2() const
{
    if ( ! d->unicode )
	return 0;
    unsigned int len = d->len;
    if ( d->maxl < len + 1 ) {
	// detach, grow or shrink
	uint newMax = computeNewMax( len + 1 );
	QChar* nd = QT_ALLOC_QCHAR_VEC( newMax );
	if ( nd ) {
	    if ( d->unicode )
		memcpy( nd, d->unicode, sizeof(QChar)*len );
	    ((QString *)this)->deref();
	    ((QString *)this)->d = new QStringData( nd, len, newMax );
	}
    }
    d->unicode[len] = 0;
    return (unsigned short *) d->unicode;
}

/*!
  Constructs a string that is a deep copy of \a str, interpreted as a
  UCS2 encoded, zero terminated, Unicode string.

  If \a str is 0, then a null string is created.

  \sa isNull()
*/
QString QString::fromUcs2( const unsigned short *str )
{
    if ( !str ) {
	return QString::null;
    } else {
	int length = 0;
	while( str[length] != 0 )
	    length++;
	QChar* uc = QT_ALLOC_QCHAR_VEC( length );
	memcpy( uc, str, length*sizeof(QChar) );
	return QString( new QStringData( uc, length, length ), TRUE );
    }
}

/*!
  \fn QChar QString::at( uint ) const

    Returns the character at index \a i, or 0 if \a i is beyond the
    length of the string.

    \code
	const QString string( "abcdefgh" );
	QChar ch = string.at( 4 );
	// ch == 'e'
    \endcode

    If the QString is not const (i.e. const QString) or const& (i.e.
    const QString &), then the non-const overload of at() will be used
    instead.
*/

/*!
    \fn QChar QString::constref(uint i) const

    Returns the QChar at index \a i by value.

    Equivalent to at(\a i).

    \sa ref()
*/

/*!
    \fn QChar& QString::ref(uint i)

    Returns the QChar at index \a i by reference, expanding the string
    with QChar::null if necessary. The resulting reference can be
    assigned to, or otherwise used immediately, but becomes invalid
    once furher modifications are made to the string.

    \code
	QString string("ABCDEF");
	QChar ch = string.ref( 3 );         // ch == 'D'
    \endcode

    \sa constref()
*/

/*!
    \fn QChar QString::operator[]( int ) const

    Returns the character at index \a i, or QChar::null if \a i is
    beyond the length of the string.

    If the QString is not const (i.e., const QString) or const\&
    (i.e., const QString\&), then the non-const overload of operator[]
    will be used instead.
*/

/*!
    \fn QCharRef QString::operator[]( int )

    \overload

    The function returns a reference to the character at index \a i.
    The resulting reference can then be assigned to, or used
    immediately, but it will become invalid once further modifications
    are made to the original string.

    If \a i is beyond the length of the string then the string is
    expanded with QChar::nulls, so that the QCharRef references a
    valid (null) character in the string.

    The QCharRef internal class can be used much like a constant
    QChar, but if you assign to it, you change the original string
    (which will detach itself because of QString's copy-on-write
    semantics). You will get compilation errors if you try to use the
    result as anything but a QChar.
*/

/*!
    \fn QCharRef QString::at( uint i )

    \overload

    The function returns a reference to the character at index \a i.
    The resulting reference can then be assigned to, or used
    immediately, but it will become invalid once further modifications
    are made to the original string.

    If \a i is beyond the length of the string then the string is
    expanded with QChar::null.
*/

/*
  Internal chunk of code to handle the
  uncommon cases of at() above.
*/
void QString::subat( uint i )
{
    uint olen = d->len;
    if ( i >= olen ) {
	setLength( i+1 );               // i is index; i+1 is needed length
	for ( uint j=olen; j<=i; j++ )
	    d->unicode[j] = QChar::null;
    } else {
	// Just be sure to detach
	real_detach();
    }
}


/*!
    Resizes the string to \a len characters and copies \a unicode into
    the string. If \a unicode is 0, nothing is copied, but the
    string is still resized to \a len. If \a len is zero, then the
    string becomes a \link isNull() null\endlink string.

    \sa setLatin1(), isNull()
*/

QString& QString::setUnicode( const QChar *unicode, uint len )
{
    if ( len == 0 ) {                           // set to null string
	if ( d != shared_null ) {               // beware of nullstring being set to nullstring
	    deref();
	    d = shared_null ? shared_null : makeSharedNull();
	    d->ref();
	}
    } else if ( d->count != 1 || len > d->maxl ||
		( len * 4 < d->maxl && d->maxl > 4 ) ) {
	// detach, grown or shrink
	uint newMax = computeNewMax( len );
	QChar* nd = QT_ALLOC_QCHAR_VEC( newMax );
	if ( unicode )
	    memcpy( nd, unicode, sizeof(QChar)*len );
	deref();
	d = new QStringData( nd, len, newMax );
    } else {
	d->len = len;
	d->setDirty();
	if ( unicode )
	    memcpy( d->unicode, unicode, sizeof(QChar)*len );
    }
    return *this;
}

/*!
    Resizes the string to \a len characters and copies \a
    unicode_as_ushorts into the string (on some X11 client platforms
    this will involve a byte-swapping pass).

    If \a unicode_as_ushorts is 0, nothing is copied, but the string
    is still resized to \a len. If \a len is zero, the string becomes
    a \link isNull() null\endlink string.

    \sa setLatin1(), isNull()
*/
QString& QString::setUnicodeCodes( const ushort* unicode_as_ushorts, uint len )
{
     return setUnicode((const QChar*)unicode_as_ushorts, len);
}


/*!
    Sets this string to \a str, interpreted as a classic 8-bit ASCII C
    string. If \a len is -1 (the default), then it is set to
    strlen(str).

    If \a str is 0 a null string is created. If \a str is "", an empty
    string is created.

    \sa isNull(), isEmpty()
*/

QString &QString::setAscii( const char *str, int len )
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	*this = QString::fromAscii( str, len );
	return *this;
    }
#endif // QT_NO_TEXTCODEC
    return setLatin1( str, len );
}

/*!
    Sets this string to \a str, interpreted as a classic Latin1 C
    string. If \a len is -1 (the default), then it is set to
    strlen(str).

    If \a str is 0 a null string is created. If \a str is "", an empty
    string is created.

    \sa isNull(), isEmpty()
*/

QString &QString::setLatin1( const char *str, int len )
{
    if ( str == 0 )
	return setUnicode(0,0);
    if ( len < 0 )
	len = strlen( str );
    if ( len == 0 ) {                           // won't make a null string
	*this = QString::fromLatin1( "" );
    } else {
	setUnicode( 0, len );                   // resize but not copy
	QChar *p = d->unicode;
	while ( len-- )
	    *p++ = *str++;
    }
    return *this;
}

/*! \internal
 */
void QString::checkSimpleText() const
{
    QChar *p = d->unicode;
    QChar *end = p + d->len;
    while( p < end ) {
	ushort uc = p->unicode();
	// sort out regions of complex text formatting
	if ( uc > 0x058f && ( uc < 0x1100 || uc > 0xfb0f ) ) {
	    d->issimpletext = FALSE;
	    return;
	}
	p++;
    }
    d->issimpletext = TRUE;
}

/*! \fn bool QString::simpleText() const
  \internal
*/

/*! \internal
 */
bool QString::isRightToLeft() const
{
    int len = length();
    QChar *p = d->unicode;
    while( len-- ) {
	switch( ::direction( *p ) )
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    return FALSE;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    return TRUE;
	default:
	    break;
	}
	++p;
    }
    return FALSE;
}


/*!
    \fn int QString::compare( const QString & s1, const QString & s2 )

    Lexically compares \a s1 with \a s2 and returns an integer less
    than, equal to, or greater than zero if \a s1 is less than, equal
    to, or greater than \a s2.

    The comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would
    expect. Consider sorting user-interface strings with
    QString::localeAwareCompare().

    \code
	int a = QString::compare( "def", "abc" );   // a > 0
	int b = QString::compare( "abc", "def" );   // b < 0
	int c = QString::compare(" abc", "abc" );   // c == 0
    \endcode
*/

/*!
    \overload

   Lexically compares this string with \a s and returns an integer
   less than, equal to, or greater than zero if it is less than, equal
   to, or greater than \a s.
*/
int QString::compare( const QString& s ) const
{
    return ucstrcmp( *this, s );
}

/*!
    \fn int QString::localeAwareCompare( const QString & s1, const QString & s2 )

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa QString::compare() QTextCodec::locale()
*/

/*!
    \overload

    Compares this string with \a s.
*/

#if !defined(CSTR_LESS_THAN)
#define CSTR_LESS_THAN    1
#define CSTR_EQUAL        2
#define CSTR_GREATER_THAN 3
#endif

int QString::localeAwareCompare( const QString& s ) const
{
    // do the right thing for null and empty
    if ( isEmpty() || s.isEmpty() )
	return compare( s );

#if defined(Q_WS_WIN)
    int res;
    QT_WA( {
	const TCHAR* s1 = (TCHAR*)ucs2();
	const TCHAR* s2 = (TCHAR*)s.ucs2();
	res = CompareStringW( LOCALE_USER_DEFAULT, 0, s1, length(), s2, s.length() );
    } , {
	QCString s1 = local8Bit();
	QCString s2 = s.local8Bit();
	res = CompareStringA( LOCALE_USER_DEFAULT, 0, s1.data(), s1.length(), s2.data(), s2.length() );
    } );

    switch ( res ) {
    case CSTR_LESS_THAN:
	return -1;
    case CSTR_GREATER_THAN:
	return 1;
    default:
	return 0;
    }
#elif defined(Q_WS_X11)
    // declared in <string.h>
    int delta = strcoll( local8Bit(), s.local8Bit() );
    if ( delta == 0 )
	delta = ucstrcmp( *this, s );
    return delta;
#else
    return ucstrcmp( *this, s );
#endif
}

bool operator==( const QString &s1, const QString &s2 )
{
    if ( s1.unicode() == s2.unicode() )
	return TRUE;
    return (s1.length() == s2.length()) && s1.isNull() == s2.isNull() &&
	 (memcmp((char*)s1.unicode(),(char*)s2.unicode(),
		 s1.length()*sizeof(QChar)) == 0 );
}

bool operator!=( const QString &s1, const QString &s2 )
{ return !(s1==s2); }

bool operator<( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) < 0; }

bool operator<=( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) <= 0; }

bool operator>( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) > 0; }

bool operator>=( const QString &s1, const QString &s2 )
{ return ucstrcmp(s1,s2) >= 0; }


bool operator==( const QString &s1, const char *s2 )
{
    if ( !s2 )
	return s1.isNull();

    int len = s1.length();
    const QChar *uc = s1.unicode();
    while ( len ) {
	if ( !(*s2) || uc->unicode() != (uchar) *s2 )
	    return FALSE;
	++uc;
	++s2;
	--len;
    }
    return !*s2;
}

bool operator==( const char *s1, const QString &s2 )
{ return (s2 == s1); }

bool operator!=( const QString &s1, const char *s2 )
{ return !(s1==s2); }

bool operator!=( const char *s1, const QString &s2 )
{ return !(s1==s2); }

bool operator<( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) < 0; }

bool operator<( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) < 0; }

bool operator<=( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) <= 0; }

bool operator<=( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) <= 0; }

bool operator>( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) > 0; }

bool operator>( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) > 0; }

bool operator>=( const QString &s1, const char *s2 )
{ return ucstrcmp(s1,s2) >= 0; }

bool operator>=( const char *s1, const QString &s2 )
{ return ucstrcmp(s1,s2) >= 0; }


/*****************************************************************************
  Documentation for QString related functions
 *****************************************************************************/

/*!
    \fn bool operator==( const QString &s1, const QString &s2 )

    \relates QString

    Returns TRUE if \a s1 is equal to \a s2; otherwise returns FALSE.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) != 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator==( const QString &s1, const char *s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is equal to \a s2; otherwise returns FALSE.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) == 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator==( const char *s1, const QString &s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is equal to \a s2; otherwise returns FALSE.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) == 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator!=( const QString &s1, const QString &s2 )

    \relates QString

    Returns TRUE if \a s1 is not equal to \a s2; otherwise returns FALSE.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) != 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator!=( const QString &s1, const char *s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is not equal to \a s2; otherwise returns FALSE.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) != 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator!=( const char *s1, const QString &s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is not equal to \a s2; otherwise returns FALSE.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) != 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator<( const QString &s1, const char *s2 )

    \relates QString

    Returns TRUE if \a s1 is lexically less than \a s2; otherwise returns FALSE.
    The comparison is case sensitive.

    Equivalent to compare(\a s1, \a s2) \< 0.
*/

/*!
    \fn bool operator<( const char *s1, const QString &s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is lexically less than \a s2; otherwise returns FALSE.
    The comparison is case sensitive.

    Equivalent to compare(\a s1, \a s2) \< 0.
*/

/*!
    \fn bool operator<=( const QString &s1, const char *s2 )

    \relates QString

    Returns TRUE if \a s1 is lexically less than or equal to \a s2;
    otherwise returns FALSE.
    The comparison is case sensitive.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1,\a s2) \<= 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator<=( const char *s1, const QString &s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is lexically less than or equal to \a s2;
    otherwise returns FALSE.
    The comparison is case sensitive.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) \<= 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator>( const QString &s1, const char *s2 )

    \relates QString

    Returns TRUE if \a s1 is lexically greater than \a s2; otherwise
    returns FALSE.
    The comparison is case sensitive.

    Equivalent to compare(\a s1, \a s2) \> 0.
*/

/*!
    \fn bool operator>( const char *s1, const QString &s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is lexically greater than \a s2; otherwise
    returns FALSE.
    The comparison is case sensitive.

    Equivalent to compare(\a s1, \a s2) \> 0.
*/

/*!
    \fn bool operator>=( const QString &s1, const char *s2 )

    \relates QString

    Returns TRUE if \a s1 is lexically greater than or equal to \a s2;
    otherwise returns FALSE.
    The comparison is case sensitive.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) \>= 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn bool operator>=( const char *s1, const QString &s2 )

    \overload
    \relates QString

    Returns TRUE if \a s1 is lexically greater than or equal to \a s2;
    otherwise returns FALSE.
    The comparison is case sensitive.
    Note that a null string is not equal to a not-null empty string.

    Equivalent to compare(\a s1, \a s2) \>= 0.

    \sa isNull(), isEmpty()
*/

/*!
    \fn const QString operator+( const QString &s1, const QString &s2 )

    \relates QString

    Returns a string which is the result of concatenating the string
    \a s1 and the string \a s2.

    Equivalent to \a {s1}.append(\a s2).
*/

/*!
    \fn const QString operator+( const QString &s1, const char *s2 )

    \overload
    \relates QString

    Returns a string which is the result of concatenating the string
    \a s1 and character \a s2.

    Equivalent to \a {s1}.append(\a s2).
*/

/*!
    \fn const QString operator+( const char *s1, const QString &s2 )

    \overload
    \relates QString

    Returns a string which is the result of concatenating the
    character \a s1 and string \a s2.
*/

/*!
    \fn const QString operator+( const QString &s, char c )

    \overload
    \relates QString

    Returns a string which is the result of concatenating the string
    \a s and character \a c.

    Equivalent to \a {s}.append(\a c).
*/

/*!
    \fn const QString operator+( char c, const QString &s )

    \overload
    \relates QString

    Returns a string which is the result of concatenating the
    character \a c and string \a s.

    Equivalent to \a {s}.prepend(\a c).
*/


/*****************************************************************************
  QString stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QString

    Writes the string \a str to the stream \a s.

    See also \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QString &str )
{
    if ( s.version() == 1 ) {
	QCString l( str.latin1() );
	s << l;
    }
    else {
	int byteOrder = s.byteOrder();
	const QChar* ub = str.unicode();
	if ( ub || s.version() < 3 ) {
	    static const uint auto_size = 1024;
	    char t[auto_size];
	    char *b;
	    if ( str.length()*sizeof(QChar) > auto_size ) {
		b = new char[str.length()*sizeof(QChar)];
	    } else {
		b = t;
	    }
	    int l = str.length();
	    char *c=b;
	    while ( l-- ) {
		if ( byteOrder == QDataStream::BigEndian ) {
		    *c++ = (char)ub->row();
		    *c++ = (char)ub->cell();
		} else {
		    *c++ = (char)ub->cell();
		    *c++ = (char)ub->row();
		}
		ub++;
	    }
	    s.writeBytes( b, sizeof(QChar)*str.length() );
	    if ( str.length()*sizeof(QChar) > auto_size )
		delete [] b;
	} else {
	    // write null marker
	    s << (Q_UINT32)0xffffffff;
	}
    }
    return s;
}

/*!
    \relates QString

    Reads a string from the stream \a s into string \a str.

    See also \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QString &str )
{
#ifdef QT_QSTRING_UCS_4
#if defined(Q_CC_GNU)
#warning "operator>> not working properly"
#endif
#endif
    if ( s.version() == 1 ) {
	QCString l;
	s >> l;
	str = QString( l );
    }
    else {
	Q_UINT32 bytes;
	s >> bytes;                                     // read size of string
	if ( bytes == 0xffffffff ) {                    // null string
	    str = QString::null;
	} else if ( bytes > 0 ) {                       // not empty
	    int byteOrder = s.byteOrder();
	    str.setLength( bytes/2 );
	    QChar* ch = str.d->unicode;
	    static const uint auto_size = 1024;
	    char t[auto_size];
	    char *b;
	    if ( bytes > auto_size ) {
		b = new char[bytes];
	    } else {
		b = t;
	    }
	    s.readRawBytes( b, bytes );
	    int bt = bytes/2;
	    char *oldb = b;
	    while ( bt-- ) {
		if ( byteOrder == QDataStream::BigEndian )
		    *ch++ = (ushort) (((ushort)b[0])<<8) | (uchar)b[1];
		else
		    *ch++ = (ushort) (((ushort)b[1])<<8) | (uchar)b[0];
		b += 2;
	    }
	    if ( bytes > auto_size )
		delete [] oldb;
	} else {
	    str = "";
	}
    }
    return s;
}
#endif // QT_NO_DATASTREAM

/*****************************************************************************
  QConstString member functions
 *****************************************************************************/

/*!
  \class QConstString qstring.h
  \reentrant
  \ingroup text
  \brief The QConstString class provides string objects using constant Unicode data.

    In order to minimize copying, highly optimized applications can
    use QConstString to provide a QString-compatible object from
    existing Unicode data. It is then the programmer's responsibility
    to ensure that the Unicode data exists for the entire lifetime of
    the QConstString object.

    A QConstString is created with the QConstString constructor. The
    string held by the object can be obtained by calling string().
*/

/*!
    Constructs a QConstString that uses the first \a length Unicode
    characters in the array \a unicode. Any attempt to modify copies
    of the string will cause it to create a copy of the data, thus it
    remains forever unmodified.

    The data in \a unicode is not copied. The caller must be able to
    guarantee that \a unicode will not be deleted or modified.
*/
QConstString::QConstString( const QChar* unicode, uint length ) :
    QString( new QStringData( (QChar*)unicode, length, length ), TRUE )
{
}

/*!
    Destroys the QConstString, creating a copy of the data if other
    strings are still using it.
*/
QConstString::~QConstString()
{
    if ( d->count > 1 ) {
	QChar* cp = QT_ALLOC_QCHAR_VEC( d->len );
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

/*!
    Returns TRUE if the string starts with \a s; otherwise returns
    FALSE.

    \code
	QString string("Bananas");
	bool a = string.startsWith("Ban");      //  a == TRUE
    \endcode

    \sa endsWith()
*/
bool QString::startsWith( const QString& s ) const
{
    if ( isNull() )
	return s.isNull();
    if ( s.length() > length() )
	return FALSE;
    for ( int i =0; i < (int) s.length(); i++ ) {
	if ( d->unicode[i] != s[i] )
	    return FALSE;
    }
    return TRUE;
}

/*!
    Returns TRUE if the string ends with \a s; otherwise returns
    FALSE.

    \sa startsWith()
*/
bool QString::endsWith( const QString& s ) const
{
    if ( isNull() )
	return s.isNull();
    int pos = length() - s.length();
    if ( pos < 0 )
	return FALSE;
    for ( uint i = 0; i < s.length(); i++ ) {
	if ( d->unicode[pos+i] != s[(int)i] )
	    return FALSE;
    }
    return TRUE;
}

/*! \fn void QString::detach()
  If the string does not share its data with another QString instance,
  nothing happens; otherwise the function creates a new, unique copy of
  this string. This function is called whenever the map is modified. The
  implicit sharing mechanism is implemented this way.
*/

#if defined(Q_OS_WIN32)

#include <windows.h>

/*!
  \obsolete

  Returns a static Windows TCHAR* from a QString, adding NUL if \a
  addnul is TRUE.

  The lifetime of the return value is until the next call to this function,
  or until the last copy of str is deleted, whatever comes first.

  Use ucs2() instead.
*/
const void* qt_winTchar(const QString& str, bool)
{
    // So that the return value lives long enough.
    static QString str_cache;
    str_cache = str;
#ifdef UNICODE
    return str_cache.ucs2();
#else
    return str_cache.latin1();
#endif
}

/*!
    Makes a new '\0'-terminated Windows TCHAR* from a QString.
*/
void* qt_winTchar_new(const QString& str)
{
    if ( str.isNull() )
	return 0;
    int l = str.length()+1;
    TCHAR *tc = new TCHAR[ l ];
#ifdef UNICODE
    memcpy( tc, str.ucs2(), sizeof(TCHAR)*l );
#else
    memcpy( tc, str.latin1(), sizeof(TCHAR)*l );
#endif
    return tc;
}

/*!
    Makes a QString from a Windows TCHAR*.
*/
QString qt_winQString(void* tc)
{
#ifdef UNICODE
    return QString::fromUcs2( (ushort*)tc );
#else
    return QString::fromLatin1( (TCHAR *)tc );
#endif
}

QCString qt_winQString2MB( const QString& s, int uclen )
{
    if ( uclen < 0 )
	uclen = s.length();
    if ( s.isNull() )
	return QCString();
    if ( uclen == 0 )
	return QCString("");
    BOOL used_def;
    QCString mb(4096);
    int len;
    while ( !(len=WideCharToMultiByte(CP_ACP, 0, (const WCHAR*)s.unicode(), uclen,
		mb.data(), mb.size()-1, 0, &used_def)) )
    {
	int r = GetLastError();
	if ( r == ERROR_INSUFFICIENT_BUFFER ) {
	    mb.resize(1+WideCharToMultiByte( CP_ACP, 0,
				(const WCHAR*)s.unicode(), uclen,
				0, 0, 0, &used_def));
		// and try again...
	} else {
#ifndef QT_NO_DEBUG
	    // Fail.
	    qWarning("WideCharToMultiByte cannot convert multibyte text (error %d): %s (UTF8)",
		r, s.utf8().data());
#endif
	    break;
	}
    }
    mb[len]='\0';
    return mb;
}

// WATCH OUT: mblen must include the NUL (or just use -1)
QString qt_winMB2QString( const char* mb, int mblen )
{
    if ( !mb || !mblen )
	return QString::null;
    const int wclen_auto = 4096;
    WCHAR wc_auto[wclen_auto];
    int wclen = wclen_auto;
    WCHAR *wc = wc_auto;
    int len;
    while ( !(len=MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
		mb, mblen, wc, wclen )) )
    {
	int r = GetLastError();
	if ( r == ERROR_INSUFFICIENT_BUFFER ) {
	    if ( wc != wc_auto ) {
		qWarning("Size changed in MultiByteToWideChar");
		break;
	    } else {
		wclen = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
				    mb, mblen, 0, 0 );
		wc = new WCHAR[wclen];
		// and try again...
	    }
	} else {
	    // Fail.
	    qWarning("MultiByteToWideChar cannot convert multibyte text");
	    break;
	}
    }
    if ( len <= 0 )
	return QString::null;
    QString s( (QChar*)wc, len - 1 ); // len - 1: we don't want terminator
    if ( wc != wc_auto )
	delete [] wc;
    return s;
}

#endif // Q_OS_WIN32

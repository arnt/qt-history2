/****************************************************************************
**
** Implementation of QChar class.
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

// Don't define it while compiling this module, or USERS of Qt will
// not be able to link.
#ifdef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_FROM_ASCII
#endif
#ifdef QT_NO_CAST_TO_ASCII
#undef QT_NO_CAST_TO_ASCII
#endif
#include "qchar.h"
#include "qunicodetables_p.h"
#include "qtextcodec.h"

#ifndef QT_NO_CODEC_FOR_C_STRINGS
#ifdef QT_NO_TEXTCODEC
#define QT_NO_CODEC_FOR_C_STRINGS
#endif
#endif

/*!
    \class QChar qchar.h
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
    \fn void QChar::setCell(uchar cell)
    \internal
*/

/*!
    \fn void QChar::setRow(uchar row)
    \internal
*/


/*!
    \fn QChar::QChar()

    Constructs a null QChar (one that isNull()).
*/


/*!
    Constructs a QChar corresponding to ASCII/Latin1 character \a c.
*/
QChar::QChar(char c)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	// #####
	ucs =  QTextCodec::codecForCStrings()->toUnicode(&c, 1).at(0).unicode();
    else
#endif
	ucs = (unsigned char)c;
}

/*!
    Constructs a QChar corresponding to ASCII/Latin1 character \a c.
*/
QChar::QChar(uchar c)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	// #####
	ucs =  QTextCodec::codecForCStrings()->toUnicode((char *)&c, 1).at(0).unicode();
    else
#endif
	ucs = (unsigned char)c;
}


/*!
    \fn QChar::QChar(uchar c, uchar r)

    Constructs a QChar for Unicode cell \a c in row \a r.
*/


/*!
    \fn QChar::QChar(const QChar& c)

    Constructs a copy of \a c. This is a deep copy, if such a
    lightweight object can be said to have deep copies.
*/


/*!
    \fn QChar::QChar(ushort rc)

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn QChar::QChar(short rc)

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn QChar::QChar(uint rc)

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn QChar::QChar(int rc)

    Constructs a QChar for the character with Unicode code point \a rc.
*/


/*!
    \fn bool QChar::isNull() const

    Returns TRUE if the character is the Unicode character 0x0000
    (ASCII NUL); otherwise returns FALSE.
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
    Category c = ::category(*this);
    return !(c == Other_Control || c == Other_NotAssigned);
}

/*!
    Returns TRUE if the character is a separator character
    (Separator_* categories); otherwise returns FALSE.
*/
bool QChar::isSpace() const
{
    return ::isSpace(*this);
}

/*!
    Returns TRUE if the character is a mark (Mark_* categories);
    otherwise returns FALSE.
*/
bool QChar::isMark() const
{
    Category c = ::category(*this);
    return c >= Mark_NonSpacing && c <= Mark_Enclosing;
}

/*!
    Returns TRUE if the character is a punctuation mark (Punctuation_*
    categories); otherwise returns FALSE.
*/
bool QChar::isPunct() const
{
    Category c = ::category(*this);
    return (c >= Punctuation_Connector && c <= Punctuation_Other);
}

/*!
    Returns TRUE if the character is a letter (Letter_* categories);
    otherwise returns FALSE.
*/
bool QChar::isLetter() const
{
    Category c = ::category(*this);
    return (c >= Letter_Uppercase && c <= Letter_Other);
}

/*!
    Returns TRUE if the character is a number (of any sort - Number_*
    categories); otherwise returns FALSE.

    \sa isDigit()
*/
bool QChar::isNumber() const
{
    Category c = ::category(*this);
    return c >= Number_DecimalDigit && c <= Number_Other;
}

/*!
    Returns TRUE if the character is a letter or number (Letter_* or
    Number_* categories); otherwise returns FALSE.
*/
bool QChar::isLetterOrNumber() const
{
    Category c = ::category(*this);
    return (c >= Letter_Uppercase && c <= Letter_Other)
	|| (c >= Number_DecimalDigit && c <= Number_Other);
}


/*!
    Returns TRUE if the character is a decimal digit
    (Number_DecimalDigit); otherwise returns FALSE.
*/
bool QChar::isDigit() const
{
    return (::category(*this) == Number_DecimalDigit);
}


/*!
    Returns TRUE if the character is a symbol (Symbol_* categories);
    otherwise returns FALSE.
*/
bool QChar::isSymbol() const
{
    Category c = ::category(*this);
    return c >= Symbol_Math && c <= Symbol_Other;
}

/*!
    Returns the numeric value of the digit, or -1 if the character is
    not a digit.
*/
int QChar::digitValue() const
{
#ifndef QT_NO_UNICODETABLES
    register int pos = QUnicodeTables::decimal_info[row()];
    if( !pos )
	return -1;
    return QUnicodeTables::decimal_info[(pos<<8) + cell()];
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
     return ::category(*this);
}

/*!
    Returns the character's direction.

    \sa Direction
*/
QChar::Direction QChar::direction() const
{
     return ::direction(*this);
}

/*!
    \warning This function is not supported (it may change to use
    Unicode character classes).

    Returns information about the joining properties of the character
    (needed for example, for Arabic).
*/
QChar::Joining QChar::joining() const
{
    return ::joining(*this);
}


/*!
    Returns TRUE if the character is a mirrored character (one that
    should be reversed if the text direction is reversed); otherwise
    returns FALSE.
*/
bool QChar::mirrored() const
{
    return ::mirrored(*this);
}

/*!
    Returns the mirrored character if this character is a mirrored
    character, otherwise returns the character itself.
*/
QChar QChar::mirroredChar() const
{
    return ::mirroredChar(*this);
}

/*!
    Decomposes a character into its parts. Returns an empty string if
    no decomposition exists.
*/
QString QChar::decomposition() const
{
#ifndef QT_NO_UNICODETABLES
    register int pos = QUnicodeTables::decomposition_info[row()];
    if(!pos) return QString::null;

    pos = QUnicodeTables::decomposition_info[(pos<<8)+cell()];
    if(!pos) return QString::null;
    pos+=2;

    QString s;
    Q_UINT16 c;
    while ((c = QUnicodeTables::decomposition_map[pos++]) != 0)
	s += QChar(c);
    return s;
#else
    return QString();
#endif
}

/*!
    Returns the tag defining the composition of the character. Returns
    QChar::Single if no decomposition exists.
*/
QChar::Decomposition QChar::decompositionTag() const
{
#ifndef QT_NO_UNICODETABLES
    register int pos = QUnicodeTables::decomposition_info[row()];
    if(!pos) return QChar::Single;

    pos = QUnicodeTables::decomposition_info[(pos<<8)+cell()];
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
    return ::combiningClass(*this);
}


/*!
    Returns the lowercase equivalent if the character is uppercase;
    otherwise returns the character itself.
*/
QChar QChar::toLower() const
{
     return ::lower(*this);
}

/*!
    Returns the uppercase equivalent if the character is lowercase;
    otherwise returns the character itself.
*/
QChar QChar::toUpper() const
{
     return ::upper(*this);
}

/*!
    Returns the ascii character equivalent to the QChar, or 0. This
    is mainly useful for non-internationalized software.

    \sa unicode(), QTextCodec::codecForCStrings()
*/
char QChar::ascii() const
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	// #####
	return QTextCodec::codecForCStrings()->fromUnicode(QString(*this)).at(0);
#endif
    return ucs > 0xff ? 0 : (char) ucs;
}

/*!
    Converts the ascii character \a c to it's equivalent QChar. This
    is mainly useful for non-internationalized software.

    \sa unicode(), QTextCodec::codecForCStrings()
*/
QChar QChar::fromAscii(char c)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	// #####
	return QTextCodec::codecForCStrings()->toUnicode(&c, 1).at(0).unicode();
#endif
    return QChar((ushort) c);
}

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
    \fn bool operator==(QChar c1, QChar c2)

    \relates QChar

    Returns TRUE if \a c1 and \a c2 are the same Unicode character;
    otherwise returns FALSE.
*/

/*!
    \fn bool operator==(char ch, QChar c)

    \overload
    \relates QChar

    Returns TRUE if \a c is the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn bool operator==(QChar c, char ch)

    \overload
    \relates QChar

    Returns TRUE if \a c is the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn int operator!=(QChar c1, QChar c2)

    \relates QChar

    Returns TRUE if \a c1 and \a c2 are not the same Unicode
    character; otherwise returns FALSE.
*/

/*!
    \fn int operator!=(char ch, QChar c)

    \overload
    \relates QChar

    Returns TRUE if \a c is not the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn int operator!=(QChar c, char ch)

    \overload
    \relates QChar

    Returns TRUE if \a c is not the ASCII/Latin1 character \a ch;
    otherwise returns FALSE.
*/

/*!
    \fn int operator<=(QChar c1, QChar c2)

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is less than
    that of \a c2, or they are the same Unicode character; otherwise
    returns FALSE.
*/

/*!
    \fn int operator<=(QChar c, char ch)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is less than or
    equal to that of the ASCII/Latin1 character \a ch; otherwise
    returns FALSE.
*/

/*!
    \fn int operator<=(char ch, QChar c)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is less than or equal to that of \a c; otherwise
    returns FALSE.
*/

/*!
    \fn int operator>=(QChar c1, QChar c2)

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is greater than
    that of \a c2, or they are the same Unicode character; otherwise
    returns FALSE.
*/

/*!
    \fn int operator>=(QChar c, char ch)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is greater than
    or equal to that of the ASCII/Latin1 character \a ch; otherwise
    returns FALSE.
*/

/*!
    \fn int operator>=(char ch, QChar c)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is greater than or equal to that of \a c;
    otherwise returns FALSE.
*/

/*!
    \fn int operator<(QChar c1, QChar c2)

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is less than
    that of \a c2; otherwise returns FALSE.
*/

/*!
    \fn int operator<(QChar c, char ch)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is less than that
    of the ASCII/Latin1 character \a ch; otherwise returns FALSE.
*/

/*!
    \fn int operator<(char ch, QChar c)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is less than that of \a c; otherwise returns
    FALSE.
*/

/*!
    \fn int operator>(QChar c1, QChar c2)

    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c1 is greater than
    that of \a c2; otherwise returns FALSE.
*/

/*!
    \fn int operator>(QChar c, char ch)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of \a c is greater than
    that of the ASCII/Latin1 character \a ch; otherwise returns FALSE.
*/

/*!
    \fn int operator>(char ch, QChar c)

    \overload
    \relates QChar

    Returns TRUE if the numeric Unicode value of the ASCII/Latin1
    character \a ch is greater than that of \a c; otherwise returns
    FALSE.
*/

/****************************************************************************
**
** Implementation of QChar class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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

/*! \class QLatin1Char
    \brief The QLatin1Char class provides an 8-bit ASCII/Latin-1 character.

    \internal

    This class is only useful to avoid the codec for C strings business
    in the QChar(ch) constructor. You can avoid it by writing
    QChar(ch, 0).

    \ingroup text
*/

/*! \fn QLatin1Char::QLatin1Char(char ch)

*/

/*!
    \class QChar
    \brief The QChar class provides a 16-bit Unicode character.

    \ingroup text
    \reentrant

    Unicode characters are (so far) 16-bit entities without any
    markup or structure. This class represents such an entity. It is
    lightweight, so it can be used everywhere. Most compilers treat
    it like a "short int".  (In a few years, it may be necessary to
    make QChar 32-bit when more than 65536 Unicode code points have
    been defined and come into use.)

    QChar provides a full complement of testing/classification
    functions, converting to and from other formats, converting from
    composed to decomposed Unicode, and trying to compare and
    case-convert if you ask it to.

    The classification functions include functions like those in the
    standard C++ header \<cctype\> (formerly \<ctype.h\>), but
    operating on the full range of Unicode characters. They all
    return true if the character is a certain type of character;
    otherwise they return false. These classification functions are
    isNull() (returns true if the character is '\\0'), isPrint()
    (true if the character is any sort of printable character,
    including whitespace), isPunct() (any sort of punctation),
    isMark() (Unicode Mark), isLetter() (a letter), isNumber() (any
    sort of numeric character, not just 0-9), isLetterOrNumber(), and
    isDigit() (decimal digits). All of these are wrappers around
    category() which return the Unicode-defined category of each
    character.

    QChar also provides direction(), which indicates the "natural"
    writing direction of this character. The joining() function
    indicates how the character joins with its neighbors (needed
    mostly for Arabic) and finally hasMirrored(), which indicates
    whether the character needs to be mirrored when it is printed in
    its "unnatural" writing direction.

    Composed Unicode characters (like &aring;) can be converted to
    decomposed Unicode ("a" followed by "ring above") by using
    decomposition().

    In Unicode, comparison is not necessarily possible and case
    conversion is very difficult at best. Unicode, covering the
    "entire" world, also includes most of the world's case and
    sorting problems. operator==() and friends will do comparison
    based purely on the numeric Unicode value (code point) of the
    characters, and toUpper() and toLower() will do case changes when
    the character has a well-defined upper-/lower-case equivalent.
    For locale-dependent comparisons, use
    QString::localeAwareCompare().

    The conversion functions include unicode() (to a scalar),
    latin1() (to scalar, but converts all non-Latin-1 characters to
    0), row() (gives the Unicode row), cell() (gives the Unicode
    cell), digitValue() (gives the integer value of any of the
    numerous digit characters), and a host of constructors.

    QChar provides constructors and cast operators that make it easy
    to convert to and from traditional 8-bit \c{char}s. If you
    defined \c QT_NO_CAST_FROM_ASCII and \c QT_NO_CAST_TO_ASCII, as
    explained in the QString documentation, you will need to
    explicitly call fromAscii() or fromLatin1(), or use QLatin1Char,
    to construct a QChar from an 8-bit \c char, and you will need to
    call ascii() or latin1() to get the 8-bit value back.

    More information can be found in the document \link unicode.html
    About Unicode. \endlink

    \sa QString
*/

/*!
    \enum QChar::UnicodeVersion

    \value Unassigned
    \value Unicode_1_1
    \value Unicode_2_0
    \value Unicode_2_1_2
    \value Unicode_3_0
    \value Unicode_3_1
    \value Unicode_3_2
    \value Unicode_4_0
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

    \value DirAL
    \value DirAN
    \value DirB
    \value DirBN
    \value DirCS
    \value DirEN
    \value DirES
    \value DirET
    \value DirL
    \value DirLRE
    \value DirLRO
    \value DirNSM
    \value DirON
    \value DirPDF
    \value DirR
    \value DirRLE
    \value DirRLO
    \value DirS
    \value DirWS

*/

/*!
    \enum QChar::Decomposition

    This enum type defines the Unicode decomposition attributes. See
    \link http://www.unicode.org/ the Unicode Standard\endlink for a
    description of the values.

    \value NoDecomposition
    \value Canonical
    \value Circle
    \value Compat
    \value Final
    \value Font
    \value Fraction
    \value Initial
    \value Isolated
    \value Medial
    \value Narrow
    \value NoBreak
    \value Small
    \value Square
    \value Sub
    \value Super
    \value Vertical
    \value Wide

*/

/*!
    \enum QChar::Joining

    This enum type defines the Unicode joining attributes. See \link
    http://www.unicode.org/ the Unicode Standard\endlink for a
    description of the values.

    \value Center
    \value Dual
    \value OtherJoining
    \value Right

*/

/*!
    \enum QChar::CombiningClass

    This enum type defines names for some of the Unicode combining
    classes. See \link http://www.unicode.org/ the Unicode
    Standard\endlink for a description of the values.

    \value Combining_Above
    \value Combining_AboveAttached
    \value Combining_AboveLeft
    \value Combining_AboveLeftAttached
    \value Combining_AboveRight
    \value Combining_AboveRightAttached
    \value Combining_Below
    \value Combining_BelowAttached
    \value Combining_BelowLeft
    \value Combining_BelowLeftAttached
    \value Combining_BelowRight
    \value Combining_BelowRightAttached
    \value Combining_DoubleAbove
    \value Combining_DoubleBelow
    \value Combining_IotaSubscript
    \value Combining_Left
    \value Combining_LeftAttached
    \value Combining_Right
    \value Combining_RightAttached
*/

/*!
    \enum QChar::SpecialCharacter

    \value Null Character 0x0000 A QChar with this value isNull().
    \value Nbsp Non-breaking space
    \value ReplacementCharacter
    \value ObjectReplacementCharacter
    \value ByteOrderMark
    \value ByteOrderSwapped
    \value ParagraphSeparator
    \value LineSeparator
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

    Constructs a null QChar ('\\0').

    \sa isNull()
*/

/*! \fn QChar::QChar(QLatin1Char ch)

    Constructs a QChar corresponding to ASCII/Latin-1 character \a
    ch.
*/

/*!
    \fn QChar::QChar(SpecialCharacter ch)

    Constructs a QChar for the predefined character value \a ch.
*/

/*!
    Constructs a QChar corresponding to ASCII/Latin-1 character \a
    ch.
*/
QChar::QChar(char ch)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
        // #####
        ucs =  QTextCodec::codecForCStrings()->toUnicode(&ch, 1).at(0).unicode();
    else
#endif
        ucs = (uchar)ch;
}

/*!
    Constructs a QChar corresponding to ASCII/Latin-1 character \a ch.
*/
QChar::QChar(uchar ch)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
        // #####
        ucs =  QTextCodec::codecForCStrings()->toUnicode((char *)&ch, 1).at(0).unicode();
    else
#endif
        ucs = ch;
}

/*!
    \fn QChar::QChar(uchar cell, uchar row)

    Constructs a QChar for Unicode cell \a cell in row \a row.

    \sa cell(), row()
*/

/*!
    \fn QChar::QChar(ushort code)

    Constructs a QChar for the character with Unicode code point \a
    code.
*/


/*!
    \fn QChar::QChar(short code)

    Constructs a QChar for the character with Unicode code point \a
    code.
*/


/*!
    \fn QChar::QChar(uint code)

    Constructs a QChar for the character with Unicode code point \a
    code.
*/


/*!
    \fn QChar::QChar(int code)

    Constructs a QChar for the character with Unicode code point \a
    code.
*/


/*!
    \fn bool QChar::isNull() const

    Returns true if the character is the Unicode character 0x0000
    ('\\0'); otherwise returns false.
*/

/*! \fn uchar QChar::cell() const

    Returns the cell (least significant byte) of the Unicode
    character.

    \sa row()
*/

/*! \fn uchar QChar::row() const

    Returns the row (most significant byte) of the Unicode character.

    \sa cell()
*/

/*!
    Returns true if the character is a printable character; otherwise
    returns false. This is any character not of category Cc or Cn.

    Note that this gives no indication of whether the character is
    available in a particular font.
*/
bool QChar::isPrint() const
{
    Category c = ::category(*this);
    return c != Other_Control && c != Other_NotAssigned;
}

/*!
    Returns true if the character is a separator character
    (Separator_* categories); otherwise returns false.
*/
bool QChar::isSpace() const
{
    return ::isSpace(*this);
}

/*!
    Returns true if the character is a mark (Mark_* categories);
    otherwise returns false.
*/
bool QChar::isMark() const
{
    Category c = ::category(*this);
    return c >= Mark_NonSpacing && c <= Mark_Enclosing;
}

/*!
    Returns true if the character is a punctuation mark (Punctuation_*
    categories); otherwise returns false.
*/
bool QChar::isPunct() const
{
    Category c = ::category(*this);
    return (c >= Punctuation_Connector && c <= Punctuation_Other);
}

/*!
    Returns true if the character is a letter (Letter_* categories);
    otherwise returns false.
*/
bool QChar::isLetter() const
{
    Category c = ::category(*this);
    return (c >= Letter_Uppercase && c <= Letter_Other);
}

/*!
    Returns true if the character is a number (Number_* categories,
    not just 0-9); otherwise returns false.

    \sa isDigit()
*/
bool QChar::isNumber() const
{
    Category c = ::category(*this);
    return c >= Number_DecimalDigit && c <= Number_Other;
}

/*!
    Returns true if the character is a letter or number (Letter_* or
    Number_* categories); otherwise returns false.
*/
bool QChar::isLetterOrNumber() const
{
    Category c = ::category(*this);
    return (c >= Letter_Uppercase && c <= Letter_Other)
        || (c >= Number_DecimalDigit && c <= Number_Other);
}


/*!
    Returns true if the character is a decimal digit
    (Number_DecimalDigit); otherwise returns false.
*/
bool QChar::isDigit() const
{
    return (::category(*this) == Number_DecimalDigit);
}


/*!
    Returns true if the character is a symbol (Symbol_* categories);
    otherwise returns false.
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
    return QUnicodeTables::digitValue(ucs);
}

/*!
    Returns the character's \link Category category \endlink.
*/
QChar::Category QChar::category() const
{
    return ::category(*this);
}

/*!
    Returns the character's \link Direction direction \endlink.
*/
QChar::Direction QChar::direction() const
{
     return ::direction(*this);
}

/*!
    \preliminary

    Returns information about the joining properties of the character
    (needed for certain languages such as Arabic).
*/
QChar::Joining QChar::joining() const
{
    return ::joining(*this);
}


/*!
    Returns true if the character should be reversed if the text
    direction is reversed; otherwise returns false.

    \sa mirroredChar()
*/
bool QChar::hasMirrored() const
{
    return ::mirrored(*this);
}

/*!
    Returns the mirrored character if this character is a mirrored
    character; otherwise returns the character itself.

    \sa hasMirrored()
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
    return QUnicodeTables::decomposition(ucs);
}

/*!
    Returns the tag defining the composition of the character. Returns
    QChar::Single if no decomposition exists.
*/
QChar::Decomposition QChar::decompositionTag() const
{
    return QUnicodeTables::decompositionTag(ucs);
}

/*!
    Returns the combining class for the character as defined in the
    Unicode standard. This is mainly useful as a positioning hint for
    marks attached to a base character.

    The Qt text rendering engine uses this information to correctly
    position non-spacing marks around a base character.
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
    \fn char QChar::latin1() const

    Returns the Latin-1 character equivalent to the QChar, or 0. This
    is mainly useful for non-internationalized software.

    \sa ascii(), unicode(), QTextCodec::codecForCStrings()
*/

/*!
    Returns the ASCII character value of the QChar, or 0 if the
    character is not representable as ASCII (i.e., if unicode() \>=
    128). This is mainly useful for non-internationalized software.

    \sa latin1(), unicode(), QTextCodec::codecForCStrings()
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
    \fn QChar QChar::fromLatin1(char c)

    Converts the Latin-1 character \a c to its equivalent QChar. This
    is mainly useful for non-internationalized software.

    \sa fromAscii(), unicode(), QTextCodec::codecForCStrings()
*/

/*!
    Converts the ascii character \a c to its equivalent QChar. This
    is mainly useful for non-internationalized software.

###
    An alternative is to use QLatin1Char().

    \sa fromLatin1(), unicode(), QTextCodec::codecForCStrings()
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

/*! \fn ushort & QChar::unicode()

    Returns a reference to the numberic Unicode value of the QChar.
*/

/*! \fn ushort QChar::unicode() const

    \overload
*/

/*****************************************************************************
  Documentation of QChar related functions
 *****************************************************************************/

/*!
    \fn bool operator==(QChar c1, QChar c2)

    \relates QChar

    Returns true if \a c1 and \a c2 are the same Unicode character;
    otherwise returns false.
*/

/*!
    \fn int operator!=(QChar c1, QChar c2)

    \relates QChar

    Returns true if \a c1 and \a c2 are not the same Unicode
    character; otherwise returns false.
*/

/*!
    \fn int operator<=(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is less than
    or equal to that of \a c2; otherwise returns false.
*/

/*!
    \fn int operator>=(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is greater than
    or equal to that of \a c2; otherwise returns false.
*/

/*!
    \fn int operator<(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is less than
    that of \a c2; otherwise returns false.
*/

/*!
    \fn int operator>(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is greater than
    that of \a c2; otherwise returns false.
*/

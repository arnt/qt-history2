/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCHAR_H
#define QCHAR_H

#include "QtCore/qglobal.h"

class QString;

struct QLatin1Char
{
public:
    inline explicit QLatin1Char(char c) : ch(c) {}
    inline const char toLatin1() const { return ch; }
    inline const ushort unicode() const { return ushort(ch); }

private:
    const char ch;
};


class Q_CORE_EXPORT QChar {
public:
    QChar();
#ifndef QT_NO_CAST_FROM_ASCII
    QChar(char c);
    QChar(uchar c);
#endif
    QChar(QLatin1Char ch);
    QChar(uchar c, uchar r);
    inline QChar(ushort rc) : ucs(rc){}
    QChar(short rc);
    QChar(uint rc);
    QChar(int rc);
    enum SpecialCharacter {
        Null = 0x0000,
        Nbsp = 0x00a0,
        ReplacementCharacter = 0xfffd,
        ObjectReplacementCharacter = 0xfffc,
        ByteOrderMark = 0xfeff,
        ByteOrderSwapped = 0xfffe,
#ifdef QT_COMPAT
        null = Null,
        replacement = ReplacementCharacter,
        byteOrderMark = ByteOrderMark,
        byteOrderSwapped = ByteOrderSwapped,
        nbsp = Nbsp,
#endif
        ParagraphSeparator = 0x2029,
        LineSeparator = 0x2028
    };
    QChar(SpecialCharacter sc);

    // Unicode information

    enum Category
    {
        NoCategory,

        Mark_NonSpacing,          //   Mn
        Mark_SpacingCombining,    //   Mc
        Mark_Enclosing,           //   Me

        Number_DecimalDigit,      //   Nd
        Number_Letter,            //   Nl
        Number_Other,             //   No

        Separator_Space,          //   Zs
        Separator_Line,           //   Zl
        Separator_Paragraph,      //   Zp

        Other_Control,            //   Cc
        Other_Format,             //   Cf
        Other_Surrogate,          //   Cs
        Other_PrivateUse,         //   Co
        Other_NotAssigned,        //   Cn

        Letter_Uppercase,         //   Lu
        Letter_Lowercase,         //   Ll
        Letter_Titlecase,         //   Lt
        Letter_Modifier,          //   Lm
        Letter_Other,             //   Lo

        Punctuation_Connector,    //   Pc
        Punctuation_Dash,         //   Pd
        Punctuation_Dask = Punctuation_Dash, // oops
        Punctuation_Open,         //   Ps
        Punctuation_Close,        //   Pe
        Punctuation_InitialQuote, //   Pi
        Punctuation_FinalQuote,   //   Pf
        Punctuation_Other,        //   Po

        Symbol_Math,              //   Sm
        Symbol_Currency,          //   Sc
        Symbol_Modifier,          //   Sk
        Symbol_Other              //   So
    };

    enum Direction
    {
        DirL, DirR, DirEN, DirES, DirET, DirAN, DirCS, DirB, DirS, DirWS, DirON,
        DirLRE, DirLRO, DirAL, DirRLE, DirRLO, DirPDF, DirNSM, DirBN
    };

    enum Decomposition
    {
        NoDecomposition,
#ifdef QT_COMPAT
        Single = NoDecomposition,
#endif
        Canonical, Font, NoBreak, Initial, Medial,
        Final, Isolated, Circle, Super, Sub, Vertical,
        Wide, Narrow, Small, Square, Compat, Fraction
    };

    enum Joining
    {
        OtherJoining, Dual, Right, Center
    };

    enum CombiningClass
    {
        Combining_BelowLeftAttached       = 200,
        Combining_BelowAttached           = 202,
        Combining_BelowRightAttached      = 204,
        Combining_LeftAttached            = 208,
        Combining_RightAttached           = 210,
        Combining_AboveLeftAttached       = 212,
        Combining_AboveAttached           = 214,
        Combining_AboveRightAttached      = 216,

        Combining_BelowLeft               = 218,
        Combining_Below                   = 220,
        Combining_BelowRight              = 222,
        Combining_Left                    = 224,
        Combining_Right                   = 226,
        Combining_AboveLeft               = 228,
        Combining_Above                   = 230,
        Combining_AboveRight              = 232,

        Combining_DoubleBelow             = 233,
        Combining_DoubleAbove             = 234,
        Combining_IotaSubscript           = 240
    };

    enum UnicodeVersion {
        Unicode_Unassigned,
        Unicode_1_1,
        Unicode_2_0,
        Unicode_2_1_2,
        Unicode_3_0,
        Unicode_3_1,
        Unicode_3_2,
        Unicode_4_0
    };
    // ****** WHEN ADDING FUNCTIONS, CONSIDER ADDING TO QCharRef TOO

    int digitValue() const;
    QChar toLower() const;
    QChar toUpper() const;

    Category category() const;
    Direction direction() const;
    Joining joining() const;
    bool hasMirrored() const;
#ifdef QT_COMPAT
    inline QT_COMPAT bool mirrored() const { return hasMirrored(); }
#endif
    QChar mirroredChar() const;
    QString decomposition() const;
    Decomposition decompositionTag() const;
    unsigned char combiningClass() const;

    UnicodeVersion unicodeVersion() const;

    const char toAscii() const;
    inline const char toLatin1() const;
    inline const ushort unicode() const { return ucs; }
    inline ushort &unicode() { return ucs; }

    static QChar fromAscii(char c);
    static QChar fromLatin1(char c);

    inline bool isNull() const { return ucs == 0; }
    bool isPrint() const;
    bool isPunct() const;
    bool isSpace() const;
    bool isMark() const;
    bool isLetter() const;
    bool isNumber() const;
    bool isLetterOrNumber() const;
    bool isDigit() const;
    bool isSymbol() const;

    inline uchar cell() const { return uchar(ucs & 0xff); }
    inline uchar row() const { return uchar((ucs>>8)&0xff); }
    inline void setCell(uchar cell) { ucs = (ucs & 0xff00) + cell; }
    inline void setRow(uchar row) { ucs = (ushort(row)<<8) + (ucs&0xff); }

#ifdef QT_COMPAT
    inline QT_COMPAT QChar lower() const { return toLower(); }
    inline QT_COMPAT QChar upper() const { return toUpper(); }
    static inline QT_COMPAT bool networkOrdered() {
        return QSysInfo::ByteOrder == QSysInfo::BigEndian;
    }
    inline QT_COMPAT const char latin1() const { return toLatin1(); }
    inline QT_COMPAT const char ascii() const { return toAscii(); }
#endif

private:
#ifdef QT_NO_CAST_FROM_ASCII
    QChar(char c);
    QChar(uchar c);
#endif
    ushort ucs;
} Q_PACKED;

Q_DECLARE_TYPEINFO(QChar, Q_MOVABLE_TYPE);

inline QChar::QChar() : ucs(0) {}

inline const char QChar::toLatin1() const { return ucs > 0xff ? 0 : char(ucs); }
inline QChar QChar::fromLatin1(char c) { return QChar(ushort(c)); }

inline QChar::QChar(uchar c, uchar r) : ucs((r << 8) | c){}
inline QChar::QChar(short rc) : ucs(ushort(rc)){}
inline QChar::QChar(uint rc) : ucs(ushort(rc & 0xffff)){}
inline QChar::QChar(int rc) : ucs(ushort(rc & 0xffff)){}
inline QChar::QChar(SpecialCharacter s) : ucs(ushort(s)) {}
inline QChar::QChar(QLatin1Char ch) : ucs(ch.unicode()) {}

inline bool operator==(QChar c1, QChar c2) { return c1.unicode() == c2.unicode(); }
inline bool operator!=(QChar c1, QChar c2) { return c1.unicode() != c2.unicode(); }
inline bool operator<=(QChar c1, QChar c2) { return c1.unicode() <= c2.unicode(); }
inline bool operator>=(QChar c1, QChar c2) { return c1.unicode() >= c2.unicode(); }
inline bool operator<(QChar c1, QChar c2) { return c1.unicode() < c2.unicode(); }
inline bool operator>(QChar c1, QChar c2) { return c1.unicode() > c2.unicode(); }

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QChar &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QChar &);
#endif

#endif // QCHAR_H

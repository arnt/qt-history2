#ifndef QCHAR_H
#define QCHAR_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class QString;

struct QLatin1Char
{
public:
    inline explicit QLatin1Char(const char c) : ch(c) {}
    inline char latin1() const { return ch; }
    inline unsigned short unicode() const { return (ushort)ch; }
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
    QChar(uchar c, uchar r);
#ifdef Q_QDOC
    QChar(const QChar& c);
#endif
    inline QChar(ushort rc) : ucs(rc){}
    QChar(short rc);
    QChar(uint rc);
    QChar(int rc);
    QChar(const QLatin1Char &ch);
    enum SpecialChars {
	null = 0x0000,
	replacement = 0xfffd,
	byteOrderMark = 0xfeff,
	byteOrderSwapped = 0xfffe,
	nbsp = 0x00a0
    };
    QChar(SpecialChars sc);


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
        Single, Canonical, Font, NoBreak, Initial, Medial,
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

    // ****** WHEN ADDING FUNCTIONS, CONSIDER ADDING TO QCharRef TOO

    int digitValue() const;
    QChar toLower() const;
    QChar toUpper() const;

    Category category() const;
    Direction direction() const;
    Joining joining() const;
    bool mirrored() const;
    QChar mirroredChar() const;
    QString decomposition() const;
    Decomposition decompositionTag() const;
    unsigned char combiningClass() const;

    char ascii() const;
    char latin1() const;
    inline ushort unicode() const { return ucs; }
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

    inline uchar cell() const { return ((uchar) ucs & 0xff); }
    inline uchar row() const { return ((uchar) (ucs>>8)&0xff); }
    inline void setCell(uchar cell) { ucs = (ucs & 0xff00) + cell; }
    inline void setRow(uchar row) { ucs = (((ushort) row)<<8) + (ucs&0xff); }

#ifdef QT_COMPAT
    inline QT_COMPAT QChar lower() const { return toLower(); }
    inline QT_COMPAT QChar upper() const { return toUpper(); }
    static inline QT_COMPAT bool networkOrdered() {
	return QSysInfo::ByteOrder == QSysInfo::BigEndian;
    }
#endif

private:
    ushort ucs;
} Q_PACKED;

inline QChar::QChar() : ucs(0) {}

inline char QChar::latin1() const { return ucs > 0xff ? 0 : (char) ucs; }
inline QChar QChar::fromLatin1(char c) { return QChar((ushort) c); }

inline QChar::QChar(uchar c, uchar r) : ucs((r << 8) | c){}
inline QChar::QChar(short rc) : ucs((ushort) rc){}
inline QChar::QChar(uint rc) : ucs((ushort) (rc & 0xffff)){}
inline QChar::QChar(int rc) : ucs((ushort) (rc & 0xffff)){}
inline QChar::QChar(QChar::SpecialChars s) : ucs((ushort) s) {}
inline QChar::QChar(const QLatin1Char &ch) : ucs((ushort) ch.latin1()) {}

inline bool operator==(QChar c1, QChar c2) { return c1.unicode() == c2.unicode(); }
inline bool operator!=(QChar c1, QChar c2) { return c1.unicode() != c2.unicode(); }
inline bool operator<=(QChar c1, QChar c2) { return c1.unicode() <= c2.unicode(); }
inline bool operator>=(QChar c1, QChar c2) { return c1.unicode() >= c2.unicode(); }
inline bool operator<(QChar c1, QChar c2) { return c1.unicode() < c2.unicode(); }
inline bool operator>(QChar c1, QChar c2) { return c1.unicode() > c2.unicode(); }

Q_DECLARE_TYPEINFO(QChar, Q_MOVABLE_TYPE);

#endif

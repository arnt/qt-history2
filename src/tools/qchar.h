#ifndef QCHAR
#define QCHAR

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

#ifndef QT_NO_CODEC_FOR_C_STRINGS
#ifdef QT_NO_TEXTCODEC
#define QT_NO_CODEC_FOR_C_STRINGS
#endif
#endif

class QString;

class Q_EXPORT QChar {
public:
    QChar();
#ifndef QT_NO_CAST_FROM_ASCII
    explicit QChar(char c);
    explicit QChar(uchar c);
#endif
    QChar(uchar c, uchar r);
#ifdef Q_QDOC
    QChar(const QChar& c);
#endif
    QChar(ushort rc);
    QChar(short rc);
    QChar(uint rc);
    QChar(int rc);
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
    QChar lower() const;
    QChar upper() const;

    Category category() const;
    Direction direction() const;
    Joining joining() const;
    bool mirrored() const;
    QChar mirroredChar() const;
    QString decomposition() const;
    Decomposition decompositionTag() const;
    unsigned char combiningClass() const;

    const char ascii() const;
    const char latin1() const;
    inline const ushort unicode() const { return ucs; }
    inline ushort &unicode() { return ucs; }

    static QChar fromAscii(char c);
    static QChar fromLatin1(char c);

#ifndef QT_NO_CAST_TO_ASCII
    operator char() const;
#endif

    inline bool isNull() const { return unicode()==0; }
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

    static bool networkOrdered() {
	int wordSize;
	bool bigEndian = false;
	qSysInfo(&wordSize, &bigEndian);
	return bigEndian;
    }

    friend bool operator==(char ch, QChar c);
    friend bool operator==(QChar c, char ch);
    friend bool operator==(QChar c, unsigned short ch);
    friend bool operator==(unsigned short ch, QChar c);
    friend bool operator==(QChar c1, QChar c2);
    friend bool operator==(int ch, QChar c);
    friend bool operator==(QChar c, int ch);
    friend bool operator!=(QChar c1, QChar c2);
    friend bool operator!=(char ch, QChar c);
    friend bool operator!=(QChar c, char ch);
    friend bool operator!=(QChar c, unsigned short ch);
    friend bool operator!=(unsigned short ch, QChar c);
    friend bool operator!=(int ch, QChar c);
    friend bool operator!=(QChar c, int ch);
    friend bool operator<=(QChar c, char ch);
    friend bool operator<=(char ch, QChar c);
    friend bool operator<=(QChar c1, QChar c2);
    friend bool operator<=(QChar c, unsigned short ch);
    friend bool operator<=(unsigned short ch, QChar c);
    friend bool operator<=(int ch, QChar c);
    friend bool operator<=(QChar c, int ch);

private:
    ushort ucs;
} Q_PACKED;

// ##### circular dependency with QTextCodec. Need to fix somehow
#define QT_NO_CODEC_FOR_C_STRINGS

inline QChar::QChar() : ucs(0) {}

#ifndef QT_NO_CAST_FROM_ASCII
inline QChar::QChar(char c) : ucs((uchar)c)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	ucs =  QTextCodec::codecForCStrings()->toUnicode((uchar) c).ucs;
    else
#endif
	ucs = (unsigned char)c;
}
inline QChar::QChar(uchar c) : ucs(c)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	ucs =  QTextCodec::codecForCStrings()->toUnicode(c).ucs;
    else
#endif
	ucs = (unsigned char)c;
}
#endif
inline const char QChar::ascii() const
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	return QTextCodec::codecForCStrings()->fromUnicode(*this);
#endif
    return ucs > 0xff ? 0 : (char) ucs;
}
inline QChar QChar::fromAscii(char c)
{
#ifndef QT_NO_CODEC_FOR_C_STRINGS
    if (QTextCodec::codecForCStrings())
	return QTextCodec::codecForCStrings()->toUnicode((uchar) c);
#endif
    return QChar((ushort) c);
}
#ifndef QT_NO_CAST_TO_ASCII
inline QChar::operator char() const { return ascii(); }
#endif
inline const char QChar::latin1() const { return ucs > 0xff ? 0 : (char) ucs; }
inline QChar QChar::fromLatin1(char c) { return QChar((ushort) c); }

inline QChar::QChar(uchar c, uchar r) : ucs((r << 8) | c){}
inline QChar::QChar(ushort rc) : ucs(rc){}
inline QChar::QChar(short rc) : ucs((ushort) rc){}
inline QChar::QChar(uint rc) : ucs((ushort) (rc & 0xffff)){}
inline QChar::QChar(int rc) : ucs((ushort) (rc & 0xffff)){}
inline QChar::QChar(QChar::SpecialChars s) : ucs((ushort) s) {}

inline bool operator==(QChar c1, QChar c2) { return c1.ucs == c2.ucs; }
inline bool operator!=(QChar c1, QChar c2) { return c1.ucs != c2.ucs; }
inline bool operator<=(QChar c1, QChar c2) { return c1.ucs <= c2.ucs; }
inline bool operator>=(QChar c1, QChar c2) { return c2 <= c1; }
inline bool operator<(QChar c1, QChar c2) { return !(c2<=c1); }
inline bool operator>(QChar c1, QChar c2) { return !(c2>=c1); }

#ifndef QT_NO_CAST_FROM_ASCII
#ifndef QT_NO_CODEC_FOR_C_STRINGS
inline bool operator==(char ch, QChar c) { return ((uchar) ch) == c.ucs; }
inline bool operator==(QChar c, char ch) { return ((uchar) ch) == c.ucs; }
inline bool operator!=(char ch, QChar c) { return ((uchar)ch) != c.ucs; }
inline bool operator!=(QChar c, char ch) { return ((uchar) ch) != c.ucs; }
inline bool operator<=(QChar c, char ch) { return c.ucs <= ((uchar) ch); }
inline bool operator<=(char ch, QChar c) { return ((uchar) ch) <= c.ucs; }
inline bool operator>=(QChar c, char ch) { return ch <= c; }
inline bool operator>=(char ch, QChar c) { return c <= ch; }
inline bool operator<(QChar c, char ch) { return !(ch<=c); }
inline bool operator<(char ch, QChar c) { return !(c<=ch); }
inline bool operator>(QChar c, char ch) { return !(ch>=c); }
inline bool operator>(char ch, QChar c) { return !(c>=ch); }
#else
inline bool operator==(char ch, QChar c) { return QChar(ch) == c; }
inline bool operator==(QChar c, char ch) { return QChar(ch) == c; }
inline bool operator!=(char ch, QChar c) { return c != ch; }
inline bool operator!=(QChar c, char ch) { return QChar(ch) != c; }
inline bool operator<=(QChar c, char ch) { return c <= QChar(ch); }
inline bool operator<=(char ch, QChar c) { return QChar(ch) <= c; }
inline bool operator>=(QChar c, char ch) { return QChar(ch) <= c; }
inline bool operator>=(char ch, QChar c) { return QChar(c) <= ch; }
inline bool operator<(QChar c, char ch) { return !(ch<=c); }
inline bool operator<(char ch, QChar c) { return !(c<=ch); }
inline bool operator>(QChar c, char ch) { return !(ch>=c); }
inline bool operator>(char ch, QChar c) { return !(c>=ch); }
#endif
#endif

inline bool operator==(unsigned short ch, QChar c) { return ch == c.ucs; }
inline bool operator==(QChar c, unsigned short ch) { return ch == c.ucs; }
inline bool operator!=(unsigned short ch, QChar c) { return ch != c.ucs; }
inline bool operator!=(QChar c, unsigned short ch) { return ch != c.ucs; }
inline bool operator<=(unsigned short ch, QChar c) { return ch <= c.ucs; }
inline bool operator<=(QChar c, unsigned short ch) { return c.ucs <= ch; }
inline bool operator>=(QChar c, unsigned short ch) { return ch <= c; }
inline bool operator>=(unsigned short ch, QChar c) { return c <= ch; }
inline bool operator<(QChar c, unsigned short ch) { return !(ch<=c); }
inline bool operator<(unsigned short ch, QChar c) { return !(c<=ch); }
inline bool operator>(QChar c, unsigned short ch) { return !(ch>=c); }
inline bool operator>(unsigned short ch, QChar c) { return !(c>=ch); }

inline bool operator==(int ch, QChar c) { return ch == c.ucs; }
inline bool operator==(QChar c, int ch) { return ch == c.ucs; }
inline bool operator!=(int ch, QChar c) { return ch != c.ucs; }
inline bool operator!=(QChar c, int ch) { return ch != c.ucs; }
inline bool operator<=(int ch, QChar c) { return ch <= c.ucs; }
inline bool operator<=(QChar c, int ch) { return c.ucs <= ch; }
inline bool operator>=(QChar c, int ch) { return ch <= c; }
inline bool operator>=(int ch, QChar c) { return c <= ch; }
inline bool operator<(QChar c, int ch) { return !(ch<=c); }
inline bool operator<(int ch, QChar c) { return !(c<=ch); }
inline bool operator>(QChar c, int ch) { return !(ch>=c); }
inline bool operator>(int ch, QChar c) { return !(c>=ch); }

Q_DECLARE_TYPEINFO(QChar, Q_MOVABLE_TYPE);

#endif // QCHAR

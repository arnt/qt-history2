#ifndef QFONTENCODINGS_P_H
#define QFONTENCODINGS_P_H

#include <qfeatures.h>
#include <qtextcodec.h>

#include <qjpunicode.h>

#ifndef QT_NO_CODECS

class QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;
private:
    static const QJpUnicodeConv * convJP;
};

class QFontKsc5601Codec : public QTextCodec
{
public:
    QFontKsc5601Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;
};


/********


Name: GB_2312-80                                        [RFC1345,KXS2]
MIBenum: 57
Source: ECMA registry
Alias: iso-ir-58
Alias: chinese
Alias: csISO58GB231280

*/


class QFontGB2312Codec : public QTextCodec
{
public:
    QFontGB2312Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;
};

class QFontBig5Codec : public QTextCodec
{
public:
    QFontBig5Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
    bool canEncode( QChar ) const;
};


// ------------------------------------------------------------------
// the shaping codec for iso8859-6.8x fonts (see www.langbox.com)

class QFontArabic68Codec : public QTextCodec
{
public:
    QFontArabic68Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;

    QByteArray fromUnicode( const QString &str, int from, int len ) const;
    unsigned short characterFromUnicode(const QString &str, int pos) const;
};

#endif //QT_NO_CODECS

#endif

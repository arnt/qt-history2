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

#ifndef QFONTCODECS_P_H
#define QFONTCODECS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qfontencodings_x11.cpp and qfont_x11.cpp.  This header file may
// change from version to version without notice, or even be removed.
//
// We mean it.
//
//

#include "qglobal.h"
#include "qstring.h"
#include "qtextcodec.h"


#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS


class QJpUnicodeConv;


class Q_CORE_EXPORT QFontJis0201Codec : public QTextCodec
{
public:
    QFontJis0201Codec();

    const char *name() const;

    int mibEnum() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;
    int heuristicNameMatch(const char* hint) const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;
};


class Q_CORE_EXPORT QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();
    ~QFontJis0208Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QByteArray result, and also returning
    // the length of the result in lenInOut.
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;
    int heuristicNameMatch(const char* hint) const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;

private:
    QJpUnicodeConv *convJP;
};




class Q_CORE_EXPORT QFontKsc5601Codec : public QTextCodec
{
public:
    QFontKsc5601Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QByteArray result, and also returning
    // the length of the result in lenInOut.
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;
};




class Q_CORE_EXPORT QFontGb2312Codec : public QTextCodec
{
public:
    QFontGb2312Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QByteArray result, and also returning
    // the length of the result in lenInOut.
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;
};




class Q_CORE_EXPORT QFontGbkCodec : public QTextCodec
{
public:
    QFontGbkCodec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QByteArray result, and also returning
    // the length of the result in lenInOut.
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;
    int heuristicNameMatch(const char* hint) const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;
};




class Q_CORE_EXPORT QFontGb18030_0Codec : public QTextCodec
{
public:
    QFontGb18030_0Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QByteArray result, and also returning
    // the length of the result in lenInOut.
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;
};




class Q_CORE_EXPORT QFontBig5Codec : public QTextCodec
{
public:
    QFontBig5Codec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QByteArray result, and also returning
    // the length of the result in lenInOut.
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    int heuristicNameMatch(const char* hint) const;
    bool canEncode(QChar) const;
};



class Q_CORE_EXPORT QFontBig5hkscsCodec : public QTextCodec
{
public:
    QFontBig5hkscsCodec();

    // Return the official name for the encoding.
    const char* name() const ;

    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.
    int mibEnum() const ;

    // Converts len characters from chars to Unicode.
    QString toUnicode(const char* chars, int len) const ;

    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QByteArray result, and also returning
    // the length of the result in lenInOut.
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;
    int heuristicNameMatch(const char* hint) const;
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;
};


class Q_CORE_EXPORT QFontLaoCodec : public QTextCodec
{
public:
    QFontLaoCodec();

    const char *name() const;

    int mibEnum() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

    unsigned short characterFromUnicode(const QString &str, int pos) const;

    int heuristicContentMatch(const char *, int) const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar) const;
};

#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS

#endif // QFONTCODECS_P_H

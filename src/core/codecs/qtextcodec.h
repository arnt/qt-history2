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

#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODEC

class QTextCodec;
class QIODevice;
class QFont;

class Q_CORE_EXPORT QTextEncoder {
public:
    virtual ~QTextEncoder();
    virtual QByteArray fromUnicode(const QString& uc, int& lenInOut) = 0;
};

class Q_CORE_EXPORT QTextDecoder {
public:
    virtual ~QTextDecoder();
    virtual QString toUnicode(const char* chars, int len) = 0;
};

class Q_CORE_EXPORT QTextCodec {
public:
    virtual ~QTextCodec();

#ifndef QT_NO_CODECS
    static QTextCodec* loadCharmap(QIODevice*);
    static QTextCodec* loadCharmapFile(const QString& filename);
#endif //QT_NO_CODECS
    static QTextCodec* codecForMib(int mib);
    static QTextCodec* codecForName(const char* hint, int accuracy=0);
    static QTextCodec* codecForContent(const char* chars, int len);
    static QTextCodec* codecForIndex(int i);
    static QTextCodec* codecForLocale();
    static void setCodecForLocale(QTextCodec *c);

    static QTextCodec* codecForTr();
    static void setCodecForTr(QTextCodec *c);
    static QTextCodec* codecForCStrings();
    static void setCodecForCStrings(QTextCodec *c);

    static void deleteAllCodecs();

    static const char* locale();

    virtual const char* name() const = 0;
    virtual const char* mimeName() const;
    virtual int mibEnum() const = 0;

    virtual QTextDecoder* makeDecoder() const;
    virtual QTextEncoder* makeEncoder() const;

    virtual QString toUnicode(const char* chars, int len) const;
    virtual QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    virtual void fromUnicode(const QChar *in, unsigned short *out,  int length);

    QByteArray fromUnicode(const QString& uc) const;

    QString toUnicode(const QByteArray&, int len) const;
    QString toUnicode(const QByteArray&) const;
    QString toUnicode(const char* chars) const;
    virtual bool canEncode(QChar) const;
    virtual bool canEncode(const QString&) const;

    virtual int heuristicContentMatch(const char* chars, int len) const = 0;
    virtual int heuristicNameMatch(const char* hint) const;

    virtual QByteArray fromUnicode(const QString& uc, int from, int len) const;
    virtual unsigned short characterFromUnicode(const QString &str, int pos) const;

protected:
    QTextCodec();
    static int simpleHeuristicNameMatch(const char* name, const char* hint);

private:
    friend class QFont;
    friend class QFontEngineXLFD;

    static QTextCodec *cftr;
};

inline QTextCodec* QTextCodec::codecForTr() { return cftr; }
inline void QTextCodec::setCodecForTr(QTextCodec *c) { cftr = c; }
inline QTextCodec* QTextCodec::codecForCStrings() { return QString::codecForCStrings; }
inline void QTextCodec::setCodecForCStrings(QTextCodec *c) { QString::codecForCStrings = c; }

#endif // QT_NO_TEXTCODEC
#endif // QTEXTCODEC_H

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtextcodec.h#17 $
**
** Definition of QTextCodec class
**
** Created : 981015
**
** Copyright (C) 1998-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODEC

class QTextCodec;
class QIODevice;

class Q_EXPORT QTextEncoder {
public:
    virtual ~QTextEncoder();
    virtual QCString fromUnicode(const QString& uc, int& lenInOut) = 0;
};

class Q_EXPORT QTextDecoder {
public:
    virtual ~QTextDecoder();
    virtual QString toUnicode(const char* chars, int len) = 0;
};

class Q_EXPORT QTextCodec {
public:
    virtual ~QTextCodec();

#ifndef QT_NO_CODECS
    static QTextCodec* loadCharmap(QIODevice*);
    static QTextCodec* loadCharmapFile(QString filename);
#endif
    static QTextCodec* codecForMib(int mib);
    static QTextCodec* codecForName(const char* hint, int accuracy=0);
    static QTextCodec* codecForContent(const char* chars, int len);
    static QTextCodec* codecForIndex(int i);
    static QTextCodec* codecForLocale();

    static void deleteAllCodecs();

    static const char* locale();

    virtual const char* name() const = 0;
    virtual int mibEnum() const = 0;

    virtual QTextDecoder* makeDecoder() const;
    virtual QTextEncoder* makeEncoder() const;

    virtual QString toUnicode(const char* chars, int len) const;
    virtual QCString fromUnicode(const QString& uc, int& lenInOut) const;

    QCString fromUnicode(const QString& uc) const;
    QString toUnicode(const QByteArray&, int len) const;
    QString toUnicode(const QByteArray&) const;
    QString toUnicode(const char* chars) const;
    virtual bool canEncode( QChar ) const;
    virtual bool canEncode( const QString& ) const;

    virtual int heuristicContentMatch(const char* chars, int len) const = 0;
    virtual int heuristicNameMatch(const char* hint) const;

protected:
    QTextCodec();
    static int simpleHeuristicNameMatch(const char* name, const char* hint);
};
#endif // QT_NO_TEXTCODEC
#endif // QTEXTCODEC_H

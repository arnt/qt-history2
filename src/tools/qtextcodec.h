/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtextcodec.h#10 $
**
** Definition of QTextCodec class
**
** Created : 981015
**
** Copyright (C)1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#include "qstring.h"

class QTextCodec;
class QIODevice;

class QTextEncoder {
public:
    virtual ~QTextEncoder();
    virtual QCString fromUnicode(const QString& uc, int& lenInOut) = 0;
};

class QTextDecoder {
public:
    virtual ~QTextDecoder();
    virtual QString toUnicode(const char* chars, int len) = 0;
};

class QTextCodec {
public:
    virtual ~QTextCodec();

    static QTextCodec* loadCharmap(QIODevice*);
    static QTextCodec* loadCharmapFile(QString filename);

    static QTextCodec* codecForMib(int mib);
    static QTextCodec* codecForName(const char* hint);
    static QTextCodec* codecForContent(const char* chars, int len);
    static QTextCodec* codecForIndex(int i);
    static QTextCodec* codecForLocale();

    static void deleteAllCodecs();

    static const char* locale();

    virtual const char* name() const = 0;
    virtual int mib() const = 0;

    virtual QTextDecoder* makeDecoder() const;
    virtual QTextEncoder* makeEncoder() const;

    virtual QString toUnicode(const char* chars, int len) const;
    virtual QCString fromUnicode(const QString& uc, int& lenInOut) const;

    QCString fromUnicode(const QString& uc) const;
    QString toUnicode(const QByteArray&, int len) const;
    QString toUnicode(const QByteArray&) const;
    QString toUnicode(const char* chars) const;

    virtual int heuristicContentMatch(const char* chars, int len) const = 0;
    virtual int heuristicNameMatch(const char* hint) const;

protected:
    QTextCodec();
    static int simpleHeuristicNameMatch(const char* name, const char* hint);
};

#endif

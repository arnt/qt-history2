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
#ifndef QSIMPLECODEC_P_H
#define QSIMPLECODEC_P_H

#include "qtextcodec.h"

class QSimpleTextCodec: public QTextCodec
{
public:
    enum { numSimpleCodecs = 29 };
    QSimpleTextCodec(int);
    ~QSimpleTextCodec();

    QString toUnicode(const char* chars, int len) const;
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    unsigned short characterFromUnicode(const QString &str, int pos) const;

    const char* name() const;
    const char* mimeName() const;
    int mibEnum() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::canEncode;
#endif
    bool canEncode(QChar ch) const;

    void fromUnicode(const QChar *in, unsigned short *out, int length) const;

private:
    void buildReverseMap();

    int forwardIndex;
#ifndef Q_WS_QWS
    mutable QByteArray reverseMap;
#endif
};

#endif

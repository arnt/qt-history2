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
#ifndef QLATINCODEC_P_H
#define QLATINCODEC_P_H

#include "qtextcodec.h"

class QLatin1Codec : public QTextCodec
{
public:
#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
    using QTextCodec::toUnicode;
#endif
    QString toUnicode(const char* chars, int len) const;
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;
    unsigned short characterFromUnicode(const QString &str, int pos) const;

    const char* name() const;
    const char* mimeName() const;
    int mibEnum() const;
};



class QLatin15Codec: public QLatin1Codec
{
public:
    QString toUnicode(const char* chars, int len) const;
#if !defined(Q_NO_USING_KEYWORD)
    using QLatin1Codec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    void fromUnicode(const QChar *in, unsigned short *out, int length) const;
    unsigned short characterFromUnicode(const QString &str, int pos) const;

    const char* name() const;
    const char* mimeName() const;
    int mibEnum() const;
};

#endif

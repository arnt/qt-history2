/****************************************************************************
**
** Definition of QUtf{8,16}Codec class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QUTFCODEC_H
#define QUTFCODEC_H

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODEC

class Q_CORE_EXPORT QUtf8Codec : public QTextCodec {
public:
    virtual int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
};

class Q_CORE_EXPORT QUtf16Codec : public QTextCodec {
public:
    virtual int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;
    QTextEncoder* makeEncoder() const;

    int heuristicContentMatch(const char* chars, int len) const;
};

#endif //QT_NO_TEXTCODEC
#endif // QUTFCODEC_H

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

// Most of the code here was originally written by Ming-Che Chuang and
// is included in Qt with the author's permission, and the grateful
// thanks of the Trolltech team.

#ifndef QBIG5CODEC_P_H
#define QBIG5CODEC_P_H

#include "qtextcodec.h"

class QBig5Codec : public QTextCodec {
public:
    QByteArray name() const;
    int mibEnum() const;

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;
};

class QBig5hkscsCodec : public QTextCodec {
public:
    QByteArray name() const;
    int mibEnum() const;

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;
};



#ifdef Q_WS_X11
class Q_CORE_EXPORT QFontBig5Codec : public QTextCodec
{
public:
    QFontBig5Codec();

    QByteArray name() const;
    int mibEnum() const;

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;
};

class Q_CORE_EXPORT QFontBig5hkscsCodec : public QTextCodec
{
public:
    QFontBig5hkscsCodec();

    QByteArray name() const;
    int mibEnum() const;

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;
};

#endif // Q_WS_X11

#endif

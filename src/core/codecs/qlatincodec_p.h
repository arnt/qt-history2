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
    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

    const char* name() const;
    const char* mimeName() const;
    int mibEnum() const;
};



class QLatin15Codec: public QLatin1Codec
{
public:
    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

    const char* name() const;
    const char* mimeName() const;
    int mibEnum() const;
};

#endif

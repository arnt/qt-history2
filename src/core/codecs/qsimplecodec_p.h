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

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

    QByteArray name() const;
    QList<QByteArray> aliases() const;
    int mibEnum() const;

private:
    void buildReverseMap() const;

    int forwardIndex;
#ifndef Q_WS_QWS
    mutable QByteArray reverseMap;
#endif
};

#endif

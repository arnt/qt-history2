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
#ifndef QFONTJPCODEC_H
#define QFONTJPCODEC_H

#include "qtextcodec.h"
class QJpUnicodeConv;

class QFontJis0201Codec : public QTextCodec
{
public:
    QFontJis0201Codec();

    const char *QFontJis0201Codec::name() const;
    int QFontJis0201Codec::mibEnum() const;

    QByteArray QFontJis0201Codec::convertFromUnicode(const QChar *uc, int len,  ConverterState *) const;
    QString QFontJis0201Codec::convertToUnicode(const char*, int,  ConverterState *) const;
};

class QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();
    ~QFontJis0208Codec();

    const char* QFontJis0208Codec::name() const;
    int QFontJis0208Codec::mibEnum() const;

    QString QFontJis0208Codec::convertToUnicode(const char* /*chars*/, int /*len*/, ConverterState *) const;
    QByteArray QFontJis0208Codec::convertFromUnicode(const QChar *uc, int len, ConverterState *) const;
private:
    QJpUnicodeConv *convJP;
};
#endif

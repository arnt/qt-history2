#ifndef QICONVCODEC_P_H
#define QICONVCODEC_P_H

#include "qtextcodec.h"

#if defined(Q_OS_UNIX) && !defined(QT_NO_ICONV) && !defined(QT_BOOTSTRAPPED)

#include <iconv.h>

class QIconvCodec: public QTextCodec
{
private:
    mutable QTextCodec *utf16Codec;

public:
    QIconvCodec();
    ~QIconvCodec();

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

    QByteArray name() const;
    int mibEnum() const;

    static iconv_t createIconv_t(const char *to, const char *from);
};

#endif // Q_OS_UNIX && !QT_NO_ICONV && !QT_BOOTSTRAPPED

#endif // QICONVCODEC_P_H

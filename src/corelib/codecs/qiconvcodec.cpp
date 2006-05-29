/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qiconvcodec_p.h"

#include <errno.h>
#if defined (_XOPEN_UNIX) && !defined(Q_OS_QNX6) && !defined(Q_OS_OSF)
#include <langinfo.h>
#endif

QIconvCodec::QIconvCodec()
    : utf16Codec(0)
{
    utf16Codec = QTextCodec::codecForMib(1015);
    Q_ASSERT_X(utf16Codec != 0,
               "QIconvCodec::convertToUnicode",
               "internal error, UTF-16 codec not found");
    if (!utf16Codec) {
        fprintf(stderr, "QIconvCodec::convertToUnicode: internal error, UTF-16 codec not found\n");
        utf16Codec = reinterpret_cast<QTextCodec *>(~0);
    }
}

QIconvCodec::~QIconvCodec()
{
}

QString QIconvCodec::convertToUnicode(const char* chars, int len, ConverterState *) const
{
    if (utf16Codec == reinterpret_cast<QTextCodec *>(~0))
        return QString::fromAscii(chars, len);

    iconv_t cd = createIconv_t("UTF-16", 0);
    if (cd == reinterpret_cast<iconv_t>(-1)) {
        fprintf(stderr, "QIconvCodec::convertToUnicode: using ASCII for conversion, iconv_open failed\n");
        return QString::fromAscii(chars, len);
    }

    size_t inBytesLeft = len;
    // best case assumption, each byte is converted into one UTF-16 character, plus 2 bytes for the BOM
    QByteArray ba;
    size_t outBytesLeft = len * 2 + 2;
    ba.resize(outBytesLeft);
#ifdef GNU_LIBICONV
    // GNU doesn't disagree with POSIX :/
    const char *inBytes = chars;
#else
    char *inBytes = const_cast<char *>(chars);
#endif
    char *outBytes = ba.data();

    do {
        size_t ret = iconv(cd, &inBytes, &inBytesLeft, &outBytes, &outBytesLeft);
        if (ret == (size_t) -1) {
            switch (errno) {
            case EILSEQ:
            case EINVAL:
                {
                    ++inBytes;
                    --inBytesLeft;
                    break;
                }
            case E2BIG:
                {
                    int offset = ba.size() - outBytesLeft;
                    ba.resize(ba.size() * 2);
                    outBytes = ba.data() + offset;
                    outBytesLeft = ba.size() - offset;
                    break;
                }
            default:
                {
                    // note, cannot use qWarning() since we are implementing the codecForLocale :)
                    perror("QIconvCodec::convertToUnicode: using ASCII for conversion, iconv failed");
                    iconv_close(cd);
                    return QString::fromAscii(chars, len);
                }
            }
        }
    } while (inBytesLeft != 0);

    QString s = utf16Codec->toUnicode(ba.constData(), ba.size() - outBytesLeft);
    iconv_close(cd);
    return s;
}

QByteArray QIconvCodec::convertFromUnicode(const QChar *uc, int len, ConverterState *) const
{
    iconv_t cd = createIconv_t(0, "UTF-16");
    if (cd == reinterpret_cast<iconv_t>(-1)) {
        fprintf(stderr, "QIconvCodec::convertFromUnicode: using ASCII for conversion, iconv_open failed\n");
        return QString(uc, len).toAscii();
    }

    size_t outBytesLeft = len;
    QByteArray ba;
    ba.resize(outBytesLeft);
    char *outBytes = ba.data();

    // give iconv() a BOM
    QChar bom[] = { QChar(QChar::ByteOrderMark) };
#ifdef GNU_LIBICONV
    // GNU doesn't disagree with POSIX :/
    const char *inBytes = reinterpret_cast<const char *>(bom);
#else
    char *inBytes = reinterpret_cast<char *>(bom);
#endif
    size_t inBytesLeft = sizeof(bom);
    if (iconv(cd, &inBytes, &inBytesLeft, &outBytes, &outBytesLeft) == (size_t) -1) {
        perror("QIconvCodec::convertFromUnicode: using ASCII for conversion, iconv failed for BOM");
        return QString(uc, len).toAscii();
    }

    // now feed iconv() the real data
#ifdef GNU_LIBICONV
    // GNU doesn't disagree with POSIX :/
    inBytes = reinterpret_cast<const char *>(uc);
#else
    inBytes = const_cast<char *>(reinterpret_cast<const char *>(uc));
#endif
    inBytesLeft = len * sizeof(QChar);

    do {
        if (iconv(cd, &inBytes, &inBytesLeft, &outBytes, &outBytesLeft) == (size_t) -1) {
            switch (errno) {
            case EILSEQ:
            case EINVAL:
                {
                    ++inBytes;
                    --inBytesLeft;
                    break;
                }
            case E2BIG:
                {
                    int offset = ba.size() - outBytesLeft;
                    ba.resize(ba.size() * 2);
                    outBytes = ba.data() + offset;
                    outBytesLeft = ba.size() - offset;
                    break;
                }
            default:
                {
                    // note, cannot use qWarning() since we are implementing the codecForLocale :)
                    perror("QIconvCodec::convertFromUnicode: using ASCII for conversion, iconv failed");
                    iconv_close(cd);
                    return QString(uc, len).toAscii();
                }
            }
        }
    } while (inBytesLeft != 0);

    iconv_close(cd);
    ba.resize(ba.size() - outBytesLeft);
    return ba;
}

QByteArray QIconvCodec::name() const
{
    return "System";
}

int QIconvCodec::mibEnum() const
{
    return 0;
}

iconv_t QIconvCodec::createIconv_t(const char *to, const char *from)
{
    Q_ASSERT((to == 0 && from != 0) || (to != 0 && from == 0));

    iconv_t cd = (iconv_t) -1;
    char *codeset = 0;

#if defined (_XOPEN_UNIX) && !defined(Q_OS_QNX6) && !defined(Q_OS_OSF)
    codeset = nl_langinfo(CODESET);
    if (codeset)
        cd = iconv_open(to ? to : codeset, from ? from : codeset);
#endif

    if (cd == (iconv_t) -1) {
        // Very poorly defined and followed standards causes lots of
        // code to try to get all the cases... This logic is
        // duplicated in QTextCodec, so if you change it here, change
        // it there too.

        // Try to determine locale codeset from locale name assigned to
        // LC_CTYPE category.

        // First part is getting that locale name.  First try setlocale() which
        // definitely knows it, but since we cannot fully trust it, get ready
        // to fall back to environment variables.
        char * ctype = qstrdup(setlocale(LC_CTYPE, 0));

        // Get the first nonempty value from $LC_ALL, $LC_CTYPE, and $LANG
        // environment variables.
        char * lang = qstrdup(qgetenv("LC_ALL").constData());
        if (!lang || lang[0] == 0 || strcmp(lang, "C") == 0) {
            if (lang) delete [] lang;
            lang = qstrdup(qgetenv("LC_CTYPE").constData());
        }
        if (!lang || lang[0] == 0 || strcmp(lang, "C") == 0) {
            if (lang) delete [] lang;
            lang = qstrdup(qgetenv("LANG").constData());
        }

        // Now try these in order:
        // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
        // 2. CODESET from lang if it contains a .CODESET part
        // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
        // 4. locale (ditto)
        // 5. check for "@euro"

        // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
        codeset = ctype ? strchr(ctype, '.') : 0;
        if (codeset && *codeset == '.') {
            ++codeset;
            cd = iconv_open(to ? to : codeset, from ? from : codeset);
        }

        // 2. CODESET from lang if it contains a .CODESET part
        codeset = lang ? strchr(lang, '.') : 0;
        if (cd == (iconv_t) -1 && codeset && *codeset == '.') {
            ++codeset;
            cd = iconv_open(to ? to : codeset, from ? from : codeset);
        }

        // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
        if (cd == (iconv_t) -1 && ctype && *ctype != 0 && strcmp (ctype, "C") != 0)
            cd = iconv_open(to ? to : ctype, from ? from : ctype);


        // 4. locale (ditto)
        if (cd == (iconv_t) -1 && lang && *lang != 0)
            cd = iconv_open(to ? to : lang, from ? from : lang);

        // 5. "@euro"
        if (cd == (iconv_t) -1 && ctype && strstr(ctype, "@euro") || lang && strstr(lang, "@euro"))
            cd = iconv_open(to ? to : "ISO8859-15", from ? from : "ISO8859-15");

        delete [] ctype;
        delete [] lang;
    }

    return cd;
}

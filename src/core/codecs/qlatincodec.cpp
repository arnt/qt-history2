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

#include "qlatincodec_p.h"

QString QLatin1Codec::toUnicode(const char* chars, int len) const
{
    if (chars == 0)
        return QString::null;

    return QString::fromLatin1(chars, len);
}


QByteArray QLatin1Codec::fromUnicode(const QString& uc, int& len) const
{
    if (len <0 || len > (int)uc.length())
        len = uc.length();
    QByteArray r;
    r.resize(len);
    char *d = r.data();
    int i = 0;
    const QChar *ch = uc.unicode();
    while (i < len) {
        d[i] = ch->row() ? '?' : ch->cell();
        i++;
        ch++;
    }
    return r;
}

void QLatin1Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
        *out = in->row() ? 0 : in->cell();
        ++in;
        ++out;
    }
}

unsigned short QLatin1Codec::characterFromUnicode(const QString &str, int pos) const
{
    const QChar *ch = str.unicode() + pos;
    if (ch->row())
        return 0;
    return (unsigned short) ch->cell();
}


const char* QLatin1Codec::name() const
{
    return "ISO 8859-1";
}

const char* QLatin1Codec::mimeName() const
{
    return "ISO-8859-1";
}


int QLatin1Codec::mibEnum() const
{
    return 4;
}



QString QLatin15Codec::toUnicode(const char* chars, int len) const
{
    if (chars == 0)
        return QString::null;

    QString str = QString::fromLatin1(chars, len);
    QChar *uc = (QChar *)str.unicode();
    while(len--) {
        switch(uc->unicode()) {
            case 0xa4:
                *uc = 0x20ac;
                break;
            case 0xa6:
                *uc = 0x0160;
                break;
            case 0xa8:
                *uc = 0x0161;
                break;
            case 0xb4:
                *uc = 0x017d;
                break;
            case 0xb8:
                *uc = 0x017e;
                break;
            case 0xbc:
                *uc = 0x0152;
                break;
            case 0xbd:
                *uc = 0x0153;
                break;
            case 0xbe:
                *uc = 0x0178;
                break;
            default:
                break;
        }
        uc++;
    }
    return str;
}

static inline unsigned char
latin15CharFromUnicode(unsigned short uc, bool replacement = true)
{
    uchar c;
    if (uc < 0x0100) {
        if (uc > 0xa3 && uc < 0xbf) {
            switch(uc) {
            case 0xa4:
            case 0xa6:
            case 0xa8:
            case 0xb4:
            case 0xb8:
            case 0xbc:
            case 0xbd:
            case 0xbe:
                c = replacement ? '?' : 0;
                break;
            default:
                c = (unsigned char) uc;
                break;
            }
        } else {
            c = (unsigned char) uc;
        }
    } else {
        if (uc == 0x20ac)
            c = 0xa4;
        else if ((uc & 0xff00) == 0x0100) {
            switch(uc) {
            case 0x0160:
                c = 0xa6;
                break;
            case 0x0161:
                c = 0xa8;
                break;
            case 0x017d:
                c = 0xb4;
                break;
            case 0x017e:
                c = 0xb8;
                break;
            case 0x0152:
                c = 0xbc;
                break;
            case 0x0153:
                c = 0xbd;
                break;
            case 0x0178:
                c = 0xbe;
                break;
            default:
                c = replacement ? '?' : 0;
            }
        } else {
            c = replacement ? '?' : 0;
        }
    }
    return c;
}


void QLatin15Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
        *out = latin15CharFromUnicode(in->unicode(), false);
        ++in;
        ++out;
    }
}


QByteArray QLatin15Codec::fromUnicode(const QString& uc, int& len) const
{
    if (len <0 || len > (int)uc.length())
        len = uc.length();
    QByteArray r;
    r.resize(len);
    char *d = r.data();
    int i = 0;
    const QChar *ch = uc.unicode();
    while (i < len) {
        d[i] = latin15CharFromUnicode(ch->unicode());
        i++;
        ch++;
    }
    return r;
}

unsigned short QLatin15Codec::characterFromUnicode(const QString &str, int pos) const
{
    return latin15CharFromUnicode(str.unicode()[pos].unicode(), false);
}


const char* QLatin15Codec::name() const
{
    return "ISO 8859-15";
}

const char* QLatin15Codec::mimeName() const
{
    return "ISO-8859-15";
}


int QLatin15Codec::mibEnum() const
{
    return 111;
}


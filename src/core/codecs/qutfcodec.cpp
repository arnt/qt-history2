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

#include "qutfcodec_p.h"

#ifndef QT_NO_TEXTCODEC

int QUtf8Codec::mibEnum() const
{
    return 106;
}

QByteArray QUtf8Codec::convertFromUnicode(const QChar *uc, int len, ConverterState *state) const
{
    uchar replacement = '?';
    int rlen = 3*len;
    int surrogate_high = -1;
    if (state) {
        if (state->flags & ConvertInvalidToNull)
            replacement = 0;
        if (state->flags & WriteHeader)
            rlen += 3;
        if (state->remainingChars)
            surrogate_high = state->state_data[0];
    }

    QByteArray rstr;
    rstr.resize(rlen);
    uchar* cursor = (uchar*)rstr.data();
    const QChar *ch = uc;
    int invalid = 0;
    if (state && state->flags & WriteHeader) {
        *cursor++ = 0xef;
        *cursor++ = 0xbb;
        *cursor++ = 0xbf;
    }

    for (int i=0; i < len; i++) {
        uint u = ch->unicode();
        if (surrogate_high >= 0) {
            if (u >= 0xdc00 && u < 0xe000) {
                ++ch;
                ++i;
                u = (surrogate_high - 0xd800)*0x400 + (u - 0xdc00) + 0x10000;
                surrogate_high = -1;
            } else {
                // high surrogate without low
                *cursor = replacement;
                ++ch;
                ++invalid;
                surrogate_high = -1;
                continue;
            }
        } else if (u >= 0xdc00 && u < 0xe000) {
            // low surrogate without high
            *cursor = replacement;
            ++ch;
            ++invalid;
            continue;
        } else if (u >= 0xd800 && u < 0xdc00) {
            surrogate_high = u;
            ++ch;
            continue;
        }

        if (u < 0x80) {
            *cursor++ = (uchar)u;
        } else {
            if (u < 0x0800) {
                *cursor++ = 0xc0 | ((uchar) (u >> 6));
            } else {
                if (u > 0xffff) {
                    // see QString::fromUtf8() and QString::utf8() for explanations
                    if (u > 0x10fe00 && u < 0x10ff00) {
                        *cursor++ = (u - 0x10fe00);
                        ++ch;
                        continue;
                    } else {
                        *cursor++ = 0xf0 | ((uchar) (u >> 18));
                        *cursor++ = 0x80 | (((uchar) (u >> 12)) & 0x3f);
                    }
                } else {
                    *cursor++ = 0xe0 | ((uchar) (u >> 12)) & 0x3f;
                }
                *cursor++ = 0x80 | (((uchar) (u >> 6)) & 0x3f);
            }
            *cursor++ = 0x80 | ((uchar) (u&0x3f));
        }
        ++ch;
    }

    rstr.resize(cursor - (const uchar*)rstr.constData());
    if (state) {
        state->invalidChars += invalid;
        state->flags ^= WriteHeader;
        state->remainingChars = 0;
        if (surrogate_high >= 0) {
            state->remainingChars = 1;
            state->state_data[0] = surrogate_high;
        }
    }
    return rstr;
}

QString QUtf8Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
    bool headerdone = false;
    QChar replacement = QChar::replacement;
    int need = 0;
    uint uc = 0;
    if (state) {
        if (state->flags & KeepHeader)
            headerdone = true;
        if (state->flags & ConvertInvalidToNull)
            replacement = QChar::Null;
        need = state->remainingChars;
        if (need)
            uc = state->state_data[0];
    }
    if (!headerdone && len > 3
        && (uchar)chars[0] == 0xef && (uchar)chars[1] == 0xbb && (uchar)chars[2] == 0xbf) {
        // starts with a byte order mark
        chars += 3;
        len -= 3;
    }

    QString result;
    result.resize(len); // worst case
    QChar *qch = result.data();
    uchar ch;
    int invalid = 0;

    for (int i=0; i<len; i++) {
        ch = *chars++;
        if (need) {
            if ((ch&0xc0) == 0x80) {
                uc = (uc << 6) | (ch & 0x3f);
                need--;
                if (!need) {
                    if (uc > 0xffff) {
                        // surrogate pair
                        uc -= 0x10000;
                        unsigned short high = uc/0x400 + 0xd800;
                        unsigned short low = uc%0x400 + 0xdc00;
                        *qch++ = QChar(high);
                        *qch++ = QChar(low);
                    } else {
                        *qch++ = uc;
                    }
                }
            } else {
                // error
                *qch++ = QChar::ReplacementCharacter;
                ++invalid;
                need = 0;
            }
        } else {
            if (ch < 128) {
                *qch++ = QLatin1Char(ch);
            } else if ((ch & 0xe0) == 0xc0) {
                uc = ch & 0x1f;
                need = 1;
            } else if ((ch & 0xf0) == 0xe0) {
                uc = ch & 0x0f;
                need = 2;
            } else if ((ch&0xf8) == 0xf0) {
                uc = ch & 0x07;
                need = 3;
            }
        }
    }
    result.truncate(qch - result.unicode());
    if (state) {
        state->invalidChars += invalid;
        state->remainingChars = need;
        state->flags |= KeepHeader;
        state->state_data[0] = need ? uc : 0;
    }
    return result;
}

const char* QUtf8Codec::name() const
{
    return "UTF-8";
}

enum { Swap = 0, Data = 1 };

QByteArray QUtf16Codec::convertFromUnicode(const QChar *uc, int len, ConverterState *state) const
{
    int length =  2*len;
    if (state) {
        if (state->flags & WriteHeader)
            length += 2;
    }

    QByteArray d;
    d.resize(length);
    char *data = d.data();
    if (state && state->flags & WriteHeader) {
        QChar bom(QChar::ByteOrderMark);
        memcpy(d.data(), &bom, 2);
        d += 2;
    }
    memcpy(data, uc, len);

    if (state) {
        state->remainingChars = 0;
        state->flags ^= WriteHeader;
    }
    return d;
};

QString QUtf16Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
    bool headerdone = false;
    bool swap = false;
    bool half = false;
    uchar buf = 0;
    if (state) {
        if (state->flags & KeepHeader)
            headerdone = true;
        if (state->state_data[Swap])
            swap = true;
        if (state->remainingChars) {
            half = true;
            buf = state->state_data[Data];
        }
    }

    QString result;
    result.resize(len); // worst case
    QChar *qch = (QChar *)result.unicode();
    QChar ch;
    while (len--) {
        if (half) {
            if (swap) {
                ch.setRow(*chars++);
                ch.setCell(buf);
            } else {
                ch.setRow(buf);
                ch.setCell(*chars++);
            }
            if (!headerdone) {
                if (ch == QChar::ByteOrderSwapped) {
                    swap = !swap;
                } else if (ch == QChar::ByteOrderMark) {
                    // Ignore ZWNBSP
                } else {
                    *qch++ = ch;
                }
                headerdone = true;
            } else
                *qch++ = ch;
            half = false;
        } else {
            buf = *chars++;
            half = true;
        }
    }
    result.truncate(qch - result.unicode());

    if (state) {
        state->flags |= KeepHeader;
        state->state_data[Swap] = swap;
        if (half) {
            state->remainingChars = 1;
            state->state_data[Data] = buf;
        } else {
            state->remainingChars = 0;
            state->state_data[Data] = 0;
        }
    }
    return result;
}

int QUtf16Codec::mibEnum() const
{
    return 1000;
}

const char* QUtf16Codec::name() const
{
    return "ISO-10646-UCS-2";
}


#endif //QT_NO_TEXTCODEC

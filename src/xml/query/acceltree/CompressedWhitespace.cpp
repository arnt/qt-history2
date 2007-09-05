/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include <QString>
#include <QtDebug>

#include "CompressedWhitespace.h"

using namespace Patternist;

CompressedWhitespace::CharIdentifier CompressedWhitespace::toIdentifier(const QChar ch)
{
    switch(ch.unicode())
    {
        case ' ':
            return Space;
        case '\n':
            return LF;
        case '\r':
            return CR;
        case '\t':
            return Tab;
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "The caller must guarantee only whitespace is passed.");
            return Tab;
        }
    }
}

bool CompressedWhitespace::isEven(const int number)
{
    Q_ASSERT(number >= 0);
    return number % 2 == 0;
}

quint8 CompressedWhitespace::toCompressedChar(const QChar ch, const int len)
{
    Q_ASSERT(len > 0);
    Q_ASSERT(len <= MaxCharCount);

    return len + toIdentifier(ch);
}

QChar CompressedWhitespace::toChar(const CharIdentifier id)
{
    switch(id)
    {
        case Space: return QLatin1Char(' ');
        case CR:    return QLatin1Char('\r');
        case LF:    return QLatin1Char('\n');
        case Tab:   return QLatin1Char('\t');
        default:
                    {
                        Q_ASSERT_X(false, Q_FUNC_INFO, "Unexpected input");
                        return QChar();
                    }
    }
}

QString CompressedWhitespace::compress(const QStringRef &input)
{
    Q_ASSERT(!isEven(1) && isEven(0) && isEven(2));
    Q_ASSERT(!input.isEmpty());

    QString result;
    const int len = input.length();

    /* The amount of compressed characters. For instance, if input is
     * four spaces followed by one tab, compressedChars will be 2, and the resulting
     * QString will have a length of 1, two compressedChars stored in one QChar. */
    int compressedChars = 0;

    for(int i = 0; i < len; ++i)
    {
        const QChar c(input.at(i));

        int start = i;

        while(true)
        {
            if(i + 1 == input.length() || input.at(i + 1) != c)
                break;
            else
                ++i;
        }

        /* The length of subsequent whitespace characters in the input. */
        int wsLen = (i - start) + 1;

        /* We might get a sequence of whitespace that is so long, that we can't
         * store it in one unit/byte. In that case we chop it into as many subsequent
         * ones that is needed. */
        while(true)
        {
            const int unitLength = qMin(wsLen, int(MaxCharCount));
            qDebug() << "unitLength:" << unitLength << "wsLen:" << wsLen;
            wsLen -= unitLength;

            ushort resultCP = toCompressedChar(c, unitLength);

            if(isEven(compressedChars))
                result.inline_append(QChar(resultCP));
            else
            {
                resultCP = resultCP << 8;
                resultCP |= result.at(result.size() - 1).unicode();
                result[result.size() - 1] = resultCP;
            }

            ++compressedChars;

            if(wsLen == 0)
                break;
        }
    }

    return result;
}

QString CompressedWhitespace::decompress(const QString &input)
{
    Q_ASSERT(!input.isEmpty());
    const int len = input.length() * 2;
    QString retval;

    for(int i = 0; i < len; ++i)
    {
        ushort cp = input.at(i / 2).unicode();

        if(isEven(i))
            cp &= Lower8Bits;
        else
        {
            cp = cp >> 8;

            if(cp == 0)
                return retval;
        }

        const quint8 wsLen = cp & Lower6Bits;
        const quint8 id = cp & UpperTwoBits;

        /* Resize retval, and fill in on the top. */
        const int oldSize = retval.size();
        const int newSize = retval.size() + wsLen;
        retval.resize(newSize);
        const QChar ch(toChar(CharIdentifier(id)));

        for(int f = oldSize; f < newSize; ++f)
            retval[f] = ch;
    }

    return retval;
}

// vim: et:ts=4:sw=4:sts=4

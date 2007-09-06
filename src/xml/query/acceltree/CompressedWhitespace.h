/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_CompressedWhitespace_H
#define Patternist_CompressedWhitespace_H

#include <QtGlobal>

class QChar;
class QString;
class QStringRef;

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A compression facility for whitespace nodes.
     *
     * CompressedWhitespace compresses and decompresses strings that consists of
     * whitespace only, and do so with a scheme that is designed to do this
     * specialized task in an efficient way. The approach is simple: each
     * sequence of equal whitespace in the input gets coded into one byte,
     * where the first two bits signals the type, CharIdentifier, and the
     * remininding six bits is the count.
     *
     * For instance, this scheme manages to compress a sequence of spaces
     * followed by a new line into 16 bits(one QChar), and QString stores
     * strings of one QChar quite efficiently, by avoiding a heap allocation.
     *
     * There is no way to tell whether a QString is compressed or not.
     *
     * The compression scheme originates from Saxon, by Michael Kay.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CompressedWhitespace
    {
        public:
            /**
             * @short Compresses @p input into a compressed format, returned
             * as a QString.
             *
             * The caller guarantees that input is not empty
             * and consists only of whitespace.
             *
             * The returned format is opaque. There is no way to find out
             * whether a QString contains compressed data or not.
             *
             * @see decompress()
             */
            static QString compress(const QStringRef &input);

            /**
             * @short Decompresses @p input into a usual QString.
             *
             * @p input must be a QString as per returned from compress().
             *
             * @see compress()
             */
            static QString decompress(const QString &input);

        private:
            /**
             * We use the two upper bits for communicating what space it is.
             */
            enum CharIdentifier
            {
                Space   = 0x0,

                /**
                 * 0xA, \r
                 *
                 * Binary: 10000000
                 */
                CR      = 0x80,

                /**
                 * 0xD, \n
                 *
                 * Binary: 01000000
                 */
                LF      = 0x40,

                /**
                 * Binary: 11000000
                 */
                Tab     = 0xC0
            };

            enum Constants
            {
                /* We can at maximum store this many consecutive characters
                 * of one type. We use 6 bits for the count. */
                MaxCharCount = (1 << 6) - 1,

                /**
                 * Binary: 11111111
                 */
                Lower8Bits = (1 << 8) - 1,

                /**
                 * Binary: 111111
                 */
                Lower6Bits = (1 << 6) - 1,

                /*
                 * Binary: 11000000
                 */
                UpperTwoBits = 3 << 6
            };

            static inline CharIdentifier toIdentifier(const QChar ch);

            static inline quint8 toCompressedChar(const QChar ch, const int len);
            static inline QChar toChar(const CharIdentifier id);

            /**
             * @short Returns @c true if @p number is an even number, otherwise
             * @c false.
             */
            static inline bool isEven(const int number);

            /**
             * @short This class can only be used via its static members.
             */
            inline CompressedWhitespace();
            Q_DISABLE_COPY(CompressedWhitespace)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

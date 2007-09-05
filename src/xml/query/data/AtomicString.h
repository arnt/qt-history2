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

#ifndef Patternist_String_H
#define Patternist_String_H

#include <QUrl>

#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:string type.
     *
     *
     * This class was originally called String, and correspondingly the header
     * file was called String.h. However, this broke building on OS X, which
     * looks up file names case insensitively, and therefore found string.h.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo Documentation is missing/incomplete
     */
    class AtomicString : public AtomicValue
    {
    public:
        friend class CommonValues;

        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance representing @p value.
         *
         * @note This function does not remove the string literal escaping allowed in XPath 2.0
         */
        static AtomicString::Ptr fromValue(const QString &value);

        static inline AtomicString::Ptr fromValue(const QUrl &value)
        {
            return fromValue(value.toString());
        }

        /**
         * Get the Effective %Boolean Value of this string. A zero-length
         * string has an effective boolean value of @c false, in all other cases @c true.
         *
         * @returns @c false if the contained string has a zero-length, otherwise @c true.
         */
        virtual bool evaluateEBV(const PlainSharedPtr<DynamicContext> &) const;

        /**
         * The string value of a AtomicString instance is the value space.
         */
        virtual QString stringValue() const;

        virtual ItemType::Ptr type() const;

    protected:
        friend class StringComparator;
        friend class CompareFN;
        AtomicString(const QString &value);
        const QString m_value;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

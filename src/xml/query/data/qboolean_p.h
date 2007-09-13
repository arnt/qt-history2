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
****************************************************************************/

#ifndef Patternist_Boolean_H
#define Patternist_Boolean_H

#include "qitem_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:boolean type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class Q_AUTOTEST_EXPORT Boolean : public AtomicValue
    {
    public:
        typedef AtomicValue::Ptr Ptr;

        /**
         * @returns the boolean value this Boolean represents
         */
        static bool evaluateEBV(const Item::Iterator::Ptr &e,
                                const PlainSharedPtr<DynamicContext> &);

        static bool evaluateEBV(const Item &first,
                                const Item::Iterator::Ptr &e,
                                const PlainSharedPtr<DynamicContext> &);

        static bool evaluateEBV(const Item &item,
                                const PlainSharedPtr<DynamicContext> &context);

        virtual QString stringValue() const;

        /**
         * @returns a Boolean object instantiating @p value. Use True() or False()
         * if you already know what value you need.
         */
        static Boolean::Ptr fromValue(const bool value);

        /**
         * Creates a boolean value from a lexical representation. "true" and "1"
         * becomes @c true, while "false" and "0" becomes @c false.
         */
        static AtomicValue::Ptr fromLexical(const QString &val);

        /**
         * Get the Effective %Boolean Value of this boolean value. For <tt>xs:boolean</tt>, this
         * is simply the value.
         */
        virtual bool evaluateEBV(const PlainSharedPtr<DynamicContext> &) const;

        virtual ItemType::Ptr type() const;

    protected:
        friend class CommonValues;
        Boolean(const bool value);

    private:
        const bool m_value;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

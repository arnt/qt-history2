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

#ifndef Patternist_SortTuple_H
#define Patternist_SortTuple_H

#include "qitem_p.h"
#include "qitem_p.h"
#include "qitemtype_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Represents a value and its sort keys
     * in FLOWR's <tt>order by</tt> clause.
     *
     * SortTuple doesn't correspond to anything in the XPath Data Model and
     * can therefore well be described as an exotic implementation detail.
     * Most of its functions asserts because it makes no sense to
     * call them.
     *
     * SortTuple exclusively exists for use with the expressions OrderBy and
     * ReturnOrderBy, and acts as a carrier between those two for sort keys and
     * source values.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SortTuple : public AtomicValue
    {
    public:
        /**
         * @p aSortKeys may be empty.
         */
        inline SortTuple(const Item::Iterator::Ptr &aValue,
                         const Item::Vector &aSortKeys) : m_sortKeys(aSortKeys),
                                                          m_value(aValue)
        {
            Q_ASSERT(m_value);
            Q_ASSERT(!m_sortKeys.isEmpty());
        }

        /**
         * A smart pointer wrapping SortTuple instances.
         */
        typedef PlainSharedPtr<SortTuple> Ptr;

        virtual QString stringValue() const;

        /**
         * @short Always asserts.
         */
        virtual Item::Iterator::Ptr typedValue() const;

        /**
         * @short Always asserts.
         */
        virtual bool isAtomicValue() const;

        /**
         * @short Always asserts.
         */
        virtual bool isNode() const;

        /**
         * @short Always asserts.
         */
        virtual bool hasError() const;

        virtual ItemType::Ptr type() const;

        inline const Item::Vector &sortKeys() const
        {
            return m_sortKeys;
        }

        inline const Item::Iterator::Ptr &value() const
        {
            return m_value;
        }

    private:
        const Item::Vector          m_sortKeys;
        const Item::Iterator::Ptr   m_value;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

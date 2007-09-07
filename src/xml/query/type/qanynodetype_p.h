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

#ifndef Patternist_AnyNodeType_H
#define Patternist_AnyNodeType_H

#include "qatomictype_p.h"
#include "qitem_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Represents the <tt>node()</tt> item type.
     *
     * @ingroup Patternist_types
     * @see BuiltinNodeType
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AnyNodeType : public ItemType
    {
    public:

        typedef PlainSharedPtr<AnyNodeType> Ptr;

        virtual ~AnyNodeType();

        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;
        virtual bool itemMatches(const Item &item) const;
        virtual QString displayName(const NamePool::Ptr &np) const;

        virtual ItemType::Ptr xdtSuperType() const;

        virtual bool isNodeType() const;
        virtual bool isAtomicType() const;

        /**
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#acc-summ-typed-value">XQuery 1.0
         * and XPath 2.0 Data Model, G.15 dm:typed-value Accessor</a>
         */
        virtual ItemType::Ptr atomizedType() const;

        /**
         * @returns the node kind this node ItemType tests for. If it matches any node, zero is returned.
         */
        virtual Node::NodeKind nodeKind() const;

    protected:
        friend class BuiltinTypes;

        AnyNodeType();

    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

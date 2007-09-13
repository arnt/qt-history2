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

#ifndef Patternist_AnyItemType_H
#define Patternist_AnyItemType_H

#include "qatomictype_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{

    /**
     * @short Represents the <tt>item()</tt> item type.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AnyItemType : public ItemType
    {
    public:
        virtual ~AnyItemType();

        /**
         * @returns always "item()"
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * @returns always @c true
         */
        virtual bool itemMatches(const Item &item) const;

        /**
         * @returns always 0, item() is the super type in the
         * XPath Data Model hierarchy
         */
        virtual ItemType::Ptr xdtSuperType() const;

        /**
         * @returns always @c false
         */
        virtual bool isNodeType() const;

        /**
         * @returns always @c false
         */
        virtual bool isAtomicType() const;

        /**
         * @returns always @c true
         */
        virtual bool xdtTypeMatches(const ItemType::Ptr &type) const;

        /**
         * @returns xs:anyAtomicType
         */
        virtual ItemType::Ptr atomizedType() const;

    protected:
        friend class BuiltinTypes;
        AnyItemType();
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

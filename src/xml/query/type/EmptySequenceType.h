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

#ifndef Patternist_EmptySequenceType_H
#define Patternist_EmptySequenceType_H

#include "AtomicType.h"
#include "SequenceType.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Represents the <tt>empty-sequence()</tt> type.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class EmptySequenceType : public ItemType,
                              public SequenceType
    {
    public:
        typedef PlainSharedPtr<EmptySequenceType> Ptr;

        /**
         * Possibly surprisingly, this function also returns true for the @c none type.
         *
         * @returns @c true if @p other is NoneType or EmptySequenceType, otherwise @c false.
         */
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

        /**
         * @returns always @c false
         */
        virtual bool itemMatches(const Item &item) const;

        /**
         * @returns always "empty-sequence()"
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        virtual ItemType::Ptr xdtSuperType() const;

        virtual bool isNodeType() const;
        virtual bool isAtomicType() const;

        /**
         * @return always Cardinality::empty()
         */
        virtual Cardinality cardinality() const;

        /**
         * @returns always 'this' since it is also an ItemType
         */
        virtual ItemType::Ptr itemType() const;

        /**
         * @returns always @c xs:anyAtomicType
         */
        virtual ItemType::Ptr atomizedType() const;

    protected:
        friend class CommonSequenceTypes;
        EmptySequenceType();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

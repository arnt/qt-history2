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

#ifndef Patternist_EBVType_H
#define Patternist_EBVType_H

#include "qatomictype_p.h"
#include "qsequencetype_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Represents the type for which a value of can an Effective %Boolean Value
     * be extracted from.
     *
     * EBVType is an artificial type. It is not available to users of any host language
     * or is specified in any specification. It is used for implementing static type
     * checking for expressions such as IfThenClause and AndExpression.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class EBVType : public ItemType,
                    public SequenceType
    {
    public:
        typedef PlainSharedPtr<EBVType> Ptr;

        /**
         * @todo docs if it's an ebvable type, etc.
         */
        virtual bool itemMatches(const Item &item) const;
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * @note The semantical meaning of this type's item type can
         * surely be discussed. The function is provided due to
         * it being mandated by the SequenceType base class.
         *
         * @returns always 'this' since EBVType is also an ItemType
         */
        virtual ItemType::Ptr itemType() const;

        /**
         * @note The semantical meaning of this type's cardinality
         * can surely be discussed. The function is provided due to
         * it being mandated by the SequenceType base class.
         *
         * @returns always Cardinality::zeroOrMore()
         */
        virtual Cardinality cardinality() const;

        virtual bool isAtomicType() const;

        /**
         * @returns always @c null
         */
        virtual ItemType::Ptr atomizedType() const;

        /**
         * @returns always BuiltinTypes::item
         */
        virtual ItemType::Ptr xdtSuperType() const;

        /**
         * @returns always @c false
         */
        virtual bool isNodeType() const;

    protected:
        friend class CommonSequenceTypes;
        EBVType();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

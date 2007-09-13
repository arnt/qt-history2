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

#ifndef Patternist_AtomicType_H
#define Patternist_AtomicType_H

#include "qanysimpletype_p.h"
#include "qatomiccasterlocator_p.h"
#include "qatomiccomparatorlocator_p.h"
#include "qatomicmathematicianlocator_p.h"
#include "qatomictypedispatch_p.h"
#include "qitemtype_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    class Item;
    class SourceLocationReflection;

    /**
     * @short Base class for all classes that implements atomic types.
     *
     * AtomicType does not implement @c xs:anyAtomicType, it is the C++
     * base class for classes that implement atomic types, such as @c xs:anyAtomicType.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicType : public ItemType,
                       public AnySimpleType
    {
    public:

        typedef PlainSharedPtr<AtomicType> Ptr;

        virtual ~AtomicType();

        /**
         * Implements a generic algorithm which relies on wxsTypeMatches().
         *
         * @returns @c true depending on if @p item is an atomic type, and that
         * AtomicValue::itemType()'s SequenceType::itemType() matches this type.
         */
        virtual bool itemMatches(const Item &item) const;

        /**
         * @returns the result of SharedQName::displayName(), of the SharedQName
         * object returned from the name() function.
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * returns always @c false
         */
        virtual bool isNodeType() const;

        /**
         * returns always @c true
         */
        virtual bool isAtomicType() const;

        /**
         * Determines whether @p other is equal to this type, or is a
         * sub-type of this type.
         *
         * The implementation is generic, relying on operator==()
         * and xdtSuperType().
         */
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

        /**
         * @returns always 'this'
         */
        virtual ItemType::Ptr atomizedType() const;

        /**
         * @returns always SchemaType::SimpleTypeAtomic
         */
        virtual TypeCategory category() const;

        /**
         * @returns DerivationRestriction
         */
        virtual DerivationMethod derivationMethod() const;

        virtual AtomicTypeVisitorResult::Ptr
        accept(const PlainSharedPtr<AtomicTypeVisitor> &visitor,
               const SourceLocationReflection *const) const = 0;

        virtual AtomicTypeVisitorResult::Ptr
        accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
               const qint16 param,
               const SourceLocationReflection *const) const = 0;

        virtual AtomicComparatorLocator::Ptr comparatorLocator() const = 0;
        virtual AtomicMathematicianLocator::Ptr mathematicianLocator() const = 0;
        virtual AtomicCasterLocator::Ptr casterLocator() const = 0;

    protected:
        AtomicType();

    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

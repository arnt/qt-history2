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

#ifndef Patternist_NumericType_H
#define Patternist_NumericType_H

#include "AtomicType.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Represents the internal and abstract type @c fs:numeric.
     *
     * @see <a href="http://www.w3.org/TR/xquery-semantics/#dt-fs_numeric">XQuery 1.0
     * and XPath 2.0 Formal Semantics, Definition: fs:numeric</a>
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NumericType : public AtomicType
    {
    public:
        virtual ~NumericType();

        virtual bool itemMatches(const Item &item) const;
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

        /**
         * @returns always "numeric". That is, no namespace prefix
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * @returns always @c true
         */
        virtual bool isAbstract() const;

        /**
         * @returns always @c false
         */
        virtual bool isNodeType() const;

        /**
         * @returns always @c true
         */
        virtual bool isAtomicType() const;

        /**
         * @returns always xs:anyAtomicType
         */
        virtual SchemaType::Ptr wxsSuperType() const;

        /**
         * @returns always xs:anyAtomicType
         */
        virtual ItemType::Ptr xdtSuperType() const;

        /**
         * @returns @c null. It makes no sense to atomize the abstract type @c fs:numeric.
         */
        virtual ItemType::Ptr atomizedType() const;

        /**
         * NumericType cannot be visited. This function is only implemented
         * to satisfy the abstract super class's interface.
         *
         * @returns always a @c null pointer
         */
        virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
                                                    const SourceLocationReflection *const) const;

        /**
         * NumericType cannot be visited. This function is only implemented
         * to satisfy the abstract super class's interface.
         *
         * @returns always a @c null pointer
         */
        virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                                                    const qint16 op,
                                                    const SourceLocationReflection *const) const;

        /**
         * The type @c fs:numeric is an abstract type which therefore
         * cannot be involved in comparisons. Hence, this function returns
         * @c null. This function is only implemented to satisfy the abstract
         * super class's interface.
         *
         * @returns always a @c null pointer
         */
        virtual AtomicComparatorLocator::Ptr comparatorLocator() const;

        /**
         * The type @c fs:numeric is an abstract type which therefore
         * cannot be involved in arithmetics. Hence, this function returns
         * @c null. This function is only implemented to satisfy the abstract
         * super class's interface.
         *
         * @returns always a @c null pointer
         */
        virtual AtomicMathematicianLocator::Ptr mathematicianLocator() const;


        /**
         * The type @c fs:numeric is an abstract type which therefore
         * cannot be involved in casting. Hence, this function returns
         * @c null. This function is only implemented to satisfy the abstract
         * super class's interface.
         *
         * @returns always a @c null pointer
         */
        virtual AtomicCasterLocator::Ptr casterLocator() const;

    protected:
        friend class BuiltinTypes;
        NumericType();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

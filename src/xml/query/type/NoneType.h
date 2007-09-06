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

#ifndef Patternist_NoneType_H
#define Patternist_NoneType_H

#include "AtomicType.h"
#include "SequenceType.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Represents the special <tt>none</tt> type.
     *
     * @ingroup Patternist_types
     * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_content_models">XQuery 1.0 and
     * XPath 2.0 Formal Semantics, 2.4.3 Content models</a>
     * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_fnerror">XQuery 1.0 and XPath 2.0
     * Formal Semantics, 7.2.9 The fn:error function</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NoneType : public ItemType,
                     public SequenceType
    {
    public:
        typedef PlainSharedPtr<NoneType> Ptr;

        virtual bool itemMatches(const Item &item) const;
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

        /**
         * @returns always "none". That is, no namespace prefix
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * @note The semantical meaning of this type's item type can
         * surely be discussed. The function is provided due to
         * it being mandated by the SequenceType base class.
         *
         * @returns always 'this' since NoneType is also an ItemType
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

        /**
         * @returns always @c false
         */
        virtual bool isAtomicType() const;

        /**
         * This can be thought to be a weird function for this type(none). There
         * is no atomized type for none, perhaps the best from a conceptual perspective
         * would be to return @c null.
         *
         * This function returns BuiltinTypes::xsAnyAtomicType because
         * the generic type checking code inserts an Atomizer in the AST
         * when an error() function(or other node which has type none) is part of
         * an operator expression(value/general comparison, arithmetics). The Atomizer
         * returns the atomizedType() of its child, and by here returning xsAnyAtomicType,
         * static operator lookup is postponed to runtime. Subsequently, expressions like error()
         * works properly with other XPath expressions.
         */
        virtual ItemType::Ptr atomizedType() const;

        /**
         * @returns always @c false
         */
        virtual bool isNodeType() const;

        /**
         * @returns always item()
         */
        virtual ItemType::Ptr xdtSuperType() const;

        /**
         * @returns always @p other. The none type can be thought as
         * disappearing when attempting to find the union of it and
         * another type.
         */
        virtual const ItemType &operator|(const ItemType &other) const;

    protected:

        friend class CommonSequenceTypes;
        NoneType();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

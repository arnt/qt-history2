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

#ifndef Patternist_Untyped_H
#define Patternist_Untyped_H

#include "qanytype_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    class AtomicType;

    /**
     * @short Represents the complex W3C XML Schema type <tt>xs:untyped</tt>.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     * @see <a href="http://www.w3.org/TR/xpath-datamodel/#types-predefined">XQuery 1.0 and XPath 2.0
     * Data Model, 2.6.2 Predefined Types</a>
     */
    class Untyped : public AnyType
    {
    public:

        typedef PlainSharedPtr<Untyped> Ptr;

        /**
         * @returns always BuiltinTypes::xsAnyType.
         */
        virtual SchemaType::Ptr wxsSuperType() const;

        virtual QName name(const NamePool::Ptr &np) const;

        /**
         * @returns always <tt>xs:untypedAtomic</tt>
         */
        virtual ItemType::Ptr atomizedType() const;

        /**
         * @returns always SchemaType::ComplexType
         */
        virtual TypeCategory category() const;

        /**
         * @returns always NoDerivation
         */
        virtual DerivationMethod derivationMethod() const;

    protected:
        friend class BuiltinTypes;

        Untyped();
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_AnySimpleType_H
#define Patternist_AnySimpleType_H

#include "qanytype_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    class AtomicType;

    /**
     * @short Represents the @c xs:anySimpleType item type.
     *
     * @ingroup Patternist_types
     * @see <a href="http://www.w3.org/TR/xmlschema-2/#dt-anySimpleType">XML Schema Part 2:
     * Datatypes Second Edition, The simple ur-type definition</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AnySimpleType : public AnyType
    {
    public:
        friend class BuiltinTypes;

        virtual ~AnySimpleType();

        virtual QName name(const NamePool::Ptr &np) const;

        /**
         * @returns always @c xs:anySimpleType
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * @returns always BuiltinTypes::xsAnyType
         */
        virtual SchemaType::Ptr wxsSuperType() const;

        /**
         * xs:anySimpleType is the special "simple ur-type". Therefore this function
         * returns SchemaType::None
         *
         * @returns SchemaType::None
         */
        virtual TypeCategory category() const;

        /**
         * The simple ur-type is a "special restriction of the ur-type definition",
         * according to XML Schema Part 2: Datatypes Second Edition about xs:anySimpleType
         *
         * @returns DERIVATION_RESTRICTION
         */
        virtual SchemaType::DerivationMethod derivationMethod() const;

    protected:
        AnySimpleType();

    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_SchemaType_H
#define Patternist_SchemaType_H

#include "NamePool.h"
#include "SchemaComponent.h"
#include "PlainSharedPtr.h"
#include "QName.h"

template<typename N, typename M> class QHash;

QT_BEGIN_HEADER 

namespace Patternist
{
    class AtomicType;

    /**
     * @short Base class for W3C XML Schema types.
     *
     * This is the base class of all data types in a W3C XML Schema.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SchemaType : public SchemaComponent
    {
    public:

        typedef PlainSharedPtr<SchemaType> Ptr;
        typedef QHash<QName, SchemaType::Ptr> Hash;

        /**
         * Schema types are divided into different categories such as
         * complex type, atomic imple type, union simple type, and so forth. This
         * enumerator, which category() returns a value of, identifies what
         * category the type belong to.
         *
         * @todo Add docs & links for the enums
         */
        enum TypeCategory
        {
            None = 0,
            /**
             * A simple atomic type. These are also sometimes
             * referred to as primitive types. An example of this type is
             * xs:string.
             *
             * Formally speaking, a simple type with variety atomic.
             */
            SimpleTypeAtomic,
            SimpleTypeList,
            SimpleTypeUnion,
            ComplexType
        };

        enum DerivationMethod
        {
            DerivationRestriction   = 1,
            DerivationExtension     = 2,
            DerivationUnion         = 4,
            DerivationList          = 8,
            /**
             * Used for <tt>xs:anyType</tt>.
             */
            NoDerivation            = 16
        };

        SchemaType();
        virtual ~SchemaType();

        /**
         * Determines how this SchemaType is derived from its super type.
         *
         * @note Despite that DerivationMethod is designed for being
         * used for bitwise OR'd value, this function may only return one enum
         * value. If the type does not derive from any type, which is the case of
         * <tt>xs:anyType</tt>, this function returns NoDerivation.
         *
         * @see SchemaType::wxsSuperType()
         * @see <a href="http://www.w3.org/TR/DOM-Level-3-Core/core.html#TypeInfo-DerivationMethods">Document
         * Object Model (DOM) Level 3 Core Specification, Definition group DerivationMethods</a>
         * @returns a DerivationMethod enumerator signifiying how
         * this SchemaType is derived from its base type
         */
        virtual DerivationMethod derivationMethod() const = 0;

        /**
         * Determines whether the type is an abstract type.
         *
         * @note It is important a correct value is returned, since
         * abstract types must not be instantiated.
         */
        virtual bool isAbstract() const = 0;

        virtual QName name(const NamePool::Ptr &np) const = 0;
        virtual QString displayName(const NamePool::Ptr &np) const = 0;

        /**
         * @returns the W3C XML Schema base type that this type derives from. All types
         * returns an instance, except for the xs:anyType since it
         * is the very base type of all types, and it returns 0. Hence,
         * one can walk the hierarchy of a schema type by recursively calling
         * wxsSuperType, until zero is returned.
         *
         * This function walks the Schema hierarchy. Some simple types, the atomic types,
         * is also part of the XPath Data Model hierarchy, and their super type in that
         * hierarchy can be introspected with xdtSuperType().
         *
         * wxsSuperType() can be said to correspond to the {base type definition} property
         * in the Post Schema Valid Infoset(PSVI).
         *
         * @see ItemType::xdtSuperType()
         */
        virtual SchemaType::Ptr wxsSuperType() const = 0;

        /**
         * @returns @c true if @p other is identical to 'this' schema type or if @p other
         * is either directly or indirectly a base type of 'this'. Hence, calling
         * AnyType::wxsTypeMatches() with @p other as argument returns @c true for all types,
         * since all types have @c xs:anyType as super type.
         */
        virtual bool wxsTypeMatches(const SchemaType::Ptr &other) const = 0;

        virtual TypeCategory category() const = 0;

        /**
         * Determines whether the type is a simple type, by introspecting
         * the result of category().
         *
         * @note Do not re-implement this function, but instead override category()
         * and let it return an appropriate value.
         */
        bool isSimpleType() const;

        /**
         * Determines whether the type is a complex type, by introspecting
         * the result of category().
         *
         * @note Do not re-implement this function, but instead override category()
         * and let it return an appropriate value.
         */
        bool isComplexType() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

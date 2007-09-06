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

#ifndef Patternist_AnyType_H
#define Patternist_AnyType_H

#include "SchemaType.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    class AtomicType;

    /**
     * @short Represents the @c xs:anyType item type.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AnyType : public SchemaType
    {
    public:

        typedef PlainSharedPtr<AnyType> Ptr;
        friend class BuiltinTypes;

        virtual ~AnyType();

        virtual QName name(const NamePool::Ptr &np) const;

        /**
         * @returns always "xs:anyType"
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * @returns always @c false
         */
        virtual bool isAbstract() const;

        /**
         * @returns @c null, since <tt>xs:anyType</tt> has no base type, it is the ur-type.
         *
         * @returns always @c null
         */
        virtual SchemaType::Ptr wxsSuperType() const;

        /**
         * @returns @c true only if @p other is xsAnyType.
         */
        virtual bool wxsTypeMatches(const SchemaType::Ptr &other) const;

        /**
         * <tt>xs:anyType</tt> is the "ur-type" and special. Therefore, this function
         * returns SchemaType::None.
         *
         * @returns SchemaType::None
         */
        virtual TypeCategory category() const;

        /**
         * @returns always NoDerivation.
         */
        virtual DerivationMethod derivationMethod() const;

    protected:
        AnyType();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_AbstractFloatCasters_H
#define Patternist_AbstractFloatCasters_H

#include "qabstractfloat_p.h"
#include "qatomiccaster_p.h"
#include "qnumeric_p.h"

/**
 * @file
 * @short Contains classes sub-classing AtomicCaster and which
 * are responsible of casting an atomic value to AbstractFloat.
 */

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Casts a @c numeric value, such as @c xs:integer or @c xs:float, to @c xs:double or xs:float.
     *
     * castFrom() uses Numeric::toDouble() for doing the actual casting.
     *
     * @ingroup Patternist_xdm
     * @author Vincent Ricard <magic@magicninja.org>
     */
    template <const bool isDouble>
    class NumericToAbstractFloatCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:double or xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Vincent Ricard <magic@magicninja.org>
     */
    template <const bool isDouble>
    class StringToAbstractFloatCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a value of type @c xs:boolean to @c xs:double or xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Vincent Ricard <magic@magicninja.org>
     */
    template <const bool isDouble>
    class BooleanToAbstractFloatCaster : public AtomicCaster
    {
        public:
            virtual Item castFrom(const Item &from,
                                       const PlainSharedPtr<DynamicContext> &context) const;
    };

#include "qabstractfloatcasters.cpp"

   /* * no doxygen
    * @short Casts a @c numeric value, such as @c xs:integer or @c xs:float, to @c xs:double.
    *
    * castFrom() uses Numeric::toDouble() for doing the actual casting.
    *
    * @ingroup Patternist_xdm
    * @author Frans Englich <fenglich@trolltech.com>
    */
    typedef NumericToAbstractFloatCaster<true> NumericToDoubleCaster;

   /* * no doxygen
    * @short Casts a @c numeric value, such as @c xs:double or @c xs:decimal, to @c xs:float.
    *
    * castFrom() uses Numeric::toFloat() for doing the actual casting.
    *
    * @ingroup Patternist_xdm
    * @author Frans Englich <fenglich@trolltech.com>
    */
    typedef NumericToAbstractFloatCaster<false> NumericToFloatCaster;

    /* * no doxygen
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:double.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    typedef StringToAbstractFloatCaster<true> StringToDoubleCaster;

    /* * no doxygen
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    typedef StringToAbstractFloatCaster<false> StringToFloatCaster;

   /* * no doxygen
    * @short Casts a value of type @c xs:boolean to @c xs:double.
    *
    * @ingroup Patternist_xdm
    * @author Frans Englich <fenglich@trolltech.com>
    */
    typedef BooleanToAbstractFloatCaster<true> BooleanToDoubleCaster;

    /* * no doxygen
     * @short Casts a value of type @c xs:boolean to @c xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    typedef BooleanToAbstractFloatCaster<false> BooleanToFloatCaster;
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

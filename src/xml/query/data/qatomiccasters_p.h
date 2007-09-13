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

#ifndef Patternist_AtomicCasters_H
#define Patternist_AtomicCasters_H

#include "qatomiccaster_p.h"
#include "qdecimal_p.h"
#include "qderivedinteger_p.h"
#include "qderivedstring_p.h"
#include "qinteger_p.h"
#include "qvalidationerror_p.h"

/**
 * @file
 * @short Contains classes sub-classing AtomicCaster and which
 * are responsible of casting an atomic value to another type.
 */

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{

    /**
     * @short Casts any atomic value to @c xs:string.
     *
     * This class uses Item::stringValue() for retrieving a string
     * representation, and thus supports casting from atomic values
     * of any type.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const TypeOfDerivedString DerivedType>
    class ToStringCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const
        {
            Q_ASSERT(from);
            return DerivedString<DerivedType>::fromLexical(context->namePool(), from.stringValue());
        }
    };

    /**
     * @short Casts any atomic value to @c xs:untypedAtomic.
     *
     * This class uses Item::stringValue() for retrieving a string
     * representation, and thus supports casting from atomic values
     * of any type. The implementation is similar to ToStringCaster.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToUntypedAtomicCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a string value to @c xs:anyURI.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToAnyURICaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:hexBinary atomic value to @c xs:base64Binary.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class HexBinaryToBase64BinaryCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:base64Binary atomic value to @c xs:hexBinary.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Base64BinaryToHexBinaryCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:base64Binary.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToBase64BinaryCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:hexBinary.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToHexBinaryCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts any @c numeric value to @c xs:boolean.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NumericToBooleanCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts any string value, @c xs:string or @c xs:untypedAtomic, to @c xs:boolean.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToBooleanCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c numeric value, such as @c xs:double or @c xs:decimal, to @c xs:integer or
     * @c xs:decimal, depending on IsInteger.
     *
     * castFrom() uses Numeric::toInteger() for doing the actual casting.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template <const bool IsInteger>
    class NumericToDecimalCaster : public AtomicCaster
    {
    public:
        /**
         * Used by NumericToDerivedIntegerCaster in addition to this class.
         */
        static inline QString errorMessage()
        {
            return tr("When casting to %1 from %2, the source value cannot be %3.");
        }

        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const
        {
            const ItemType::Ptr t(from.type());
            const Numeric *const num = from.as<Numeric>();

            if(BuiltinTypes::xsDouble->xdtTypeMatches(t) || BuiltinTypes::xsFloat->xdtTypeMatches(t))
            {
                if(num->isInf() || num->isNaN())
                {
                    return ValidationError::createError(errorMessage()
                                                        .arg(formatType(context->namePool(), IsInteger ? BuiltinTypes::xsInteger : BuiltinTypes::xsDecimal))
                                                        .arg(formatType(context->namePool(), t))
                                                        .arg(formatData(num->stringValue())));
                }
            }

            if(IsInteger)
                return Integer::fromValue(num->toInteger());
            else
                return toItem(Decimal::fromValue(num->toDecimal()));
        }
    };

    /**
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:decimal.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToDecimalCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:integer.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToIntegerCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a value of type @c xs:boolean to @c xs:decimal.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BooleanToDecimalCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a value of type @c xs:boolean to @c xs:integer.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BooleanToIntegerCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a value to itself. Essentially, this AtomicCaster does nothing.
     *
     * Casting a value to the type of itself is defined to be a noop,
     * no operation. When it can be statically detected that will be done,
     * CastAs rewrites itself appropriately during compilation, but
     * in some cases insufficent data is available at compile time and then
     * is this class need on a case-per-case base at evaluation time.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SelfToSelfCaster : public AtomicCaster
    {
    public:

        /**
         * This function simply returns @p from.
         */
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:gYear.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToGYearCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:gDay.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToGDayCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:gMonth.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToGMonthCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:gYearMonth.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToGYearMonthCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:gYearMonth.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToGMonthDayCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:dateTime.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToDateTimeCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:time.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToTimeCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:date.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToDateCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:duration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToDurationCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:dayTimeDuration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToDayTimeDurationCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:string or @c xs:untypedAtomic atomic value to @c xs:yearMonthDuration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToYearMonthDurationCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };


    /**
     * @short Casts a @c xs:date or @c xs:dateTime atomic value to @c xs:gYear.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToGYearCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:date or @c xs:dateTime atomic value to @c xs:gYearMonth.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToGYearMonthCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:date or @c xs:dateTime atomic value to @c xs:gMonth.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToGMonthCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:date or @c xs:dateTime atomic value to @c xs:gMonthDay.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToGMonthDayCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts a @c xs:date or @c xs:dateTime atomic value to @c xs:gDay.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToGDayCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts an AbstractDateTime instance to DateTime.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToDateTimeCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts an AbstractDateTime instance to SchemaTime.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToDateCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts an AbstractDateTime instance to SchemaTime.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeToTimeCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts an AbstractDuration instance to Duration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDurationToDurationCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts an AbstractDuration instance to DayTimeDuration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDurationToDayTimeDurationCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts an AbstractDuration instance to YearMonthDuration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDurationToYearMonthDurationCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Casts an @c xs:string instance to a derived type of @c xs:integer.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const TypeOfDerivedInteger type>
    class StringToDerivedIntegerCaster : public AtomicCaster
    {
    public:
        virtual Item
        castFrom(const Item &from,
                 const PlainSharedPtr<DynamicContext> &context) const
        {
            return DerivedInteger<type>::fromLexical(context->namePool(), from.stringValue());
        }
    };

    /**
     * @short Casts an @c xs:boolean instance to a derived type of @c xs:integer.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const TypeOfDerivedInteger type>
    class BooleanToDerivedIntegerCaster : public AtomicCaster
    {
    public:
        virtual Item
        castFrom(const Item &from,
                 const PlainSharedPtr<DynamicContext> &context) const
        {
            return DerivedInteger<type>::fromValue(context->namePool(), from.as<AtomicValue>()->evaluateEBV(context) ? 1 : 0);
        }
    };

    /**
     * @short Casts an @c xs:boolean instance to a derived type of @c xs:integer.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const TypeOfDerivedString type>
    class AnyToDerivedStringCaster : public AtomicCaster
    {
    public:
        virtual Item
        castFrom(const Item &from,
                 const PlainSharedPtr<DynamicContext> &context) const
        {
            return DerivedString<type>::fromLexical(context->namePool(), from.stringValue());
        }
    };

    /**
     * @short Casts any @c numeric instance to a derived type of @c xs:integer.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const TypeOfDerivedInteger type>
    class NumericToDerivedIntegerCaster : public AtomicCaster
    {
    public:
        virtual Item
        castFrom(const Item &from,
                 const PlainSharedPtr<DynamicContext> &context) const
        {
            const ItemType::Ptr t(from.type());
            const Numeric *const num = from.as<Numeric>();

            if(BuiltinTypes::xsDouble->xdtTypeMatches(t) || BuiltinTypes::xsFloat->xdtTypeMatches(t))
            {
                if(num->isInf() || num->isNaN())
                {
                    return ValidationError::createError(NumericToDecimalCaster<false>::errorMessage()
                                                        .arg(formatType(context->namePool(), DerivedInteger<type>::itemType()))
                                                        .arg(formatType(context->namePool(), t))
                                                        .arg(formatData(num->stringValue())));
                }
            }

            return toItem(DerivedInteger<type>::fromValue(context->namePool(), from.as<Numeric>()->toInteger()));
        }
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_AtomicCasterLocators_H
#define Patternist_AtomicCasterLocators_H

#include "AtomicCasterLocator.h"
#include "AtomicCasters.h"
//#include "DerivedInteger.h"

/**
 * @file
 * @short Contains AtomicCasterLocator sub-classes that finds classes
 * which can perform casting from one atomic value to another.
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToStringCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToUntypedAtomicCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToAnyURICasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToBooleanCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToDoubleCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToFloatCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToDecimalCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToIntegerCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToBase64BinaryCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToHexBinaryCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToQNameCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToGYearCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToGDayCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToGMonthCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToGYearMonthCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToGMonthDayCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToDateTimeCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToDateCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToSchemaTimeCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToDurationCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToDayTimeDurationCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ToYearMonthDurationCasterLocator : public AtomicCasterLocator
    {
    public:
        using AtomicCasterLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const TypeOfDerivedInteger type>
    class ToDerivedIntegerCasterLocator : public ToIntegerCasterLocator
    {
    public:
        using ToIntegerCasterLocator::visit;

        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new BooleanToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeByte> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeInt> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeLong> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNegativeInteger> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNonNegativeInteger> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNonPositiveInteger> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypePositiveInteger> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeShort> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedByte> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedInt> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedLong> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedShort> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
        }
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const TypeOfDerivedString type>
    class ToDerivedStringCasterLocator : public ToStringCasterLocator
    {
    public:
        using ToStringCasterLocator::visit;

        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        // TODO TypeString not handled
        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNormalizedString> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeToken> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeLanguage> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNMTOKEN> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeName> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNCName> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeID> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeIDREF> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }

        virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeENTITY> *,
                                                   const SourceLocationReflection *const r) const
        {
            Q_UNUSED(r);
            return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
        }
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

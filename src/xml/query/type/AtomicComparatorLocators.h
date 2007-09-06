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

#ifndef Patternist_AtomicComparatorLocators_H
#define Patternist_AtomicComparatorLocators_H

#include "AtomicComparatorLocator.h"

/**
 * @file
 * @short Contains AtomicComparatorLocator sub-classes that finds classes
 * which can compare atomic values.
 */

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DoubleComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FloatComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DecimalComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class IntegerComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BooleanComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Base64BinaryComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class HexBinaryComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class QNameComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };


    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GYearComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GMonthComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GYearMonthComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GMonthDayComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GDayComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DateTimeComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SchemaTimeComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DateComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DurationComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DayTimeDurationComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class YearMonthDurationComparatorLocator : public AtomicComparatorLocator
    {
        using AtomicComparatorLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
                                                   const qint16 op,
                                                   const SourceLocationReflection *const) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

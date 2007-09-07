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

#ifndef Patternist_AtomicMathematicianLocators_H
#define Patternist_AtomicMathematicianLocators_H

#include "qatomicmathematician_p.h"
#include "qatomicmathematicianlocator_p.h"

/**
 * @file
 * @short Contains AtomicMathematicianLocator sub-classes that finds classes
 * which can perform arithmetics between atomic values.
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class DoubleMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class FloatMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class DecimalMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class IntegerMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class DateMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class SchemaTimeMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class DateTimeMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class DayTimeDurationMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

    /**
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo docs
     */
    class YearMonthDurationMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

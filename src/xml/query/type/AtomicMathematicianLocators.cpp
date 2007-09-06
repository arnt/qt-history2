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

#include "AbstractFloatMathematician.h"
#include "AtomicMathematicianLocators.h"
#include "AtomicMathematicians.h"

using namespace Patternist;

#define implMathVisit(ownerClass, visitor, mather, validOps)                            \
AtomicTypeVisitorResult::Ptr                                                            \
ownerClass##MathematicianLocator::visit(const visitor *, const qint16 op,               \
                                        const SourceLocationReflection *const r) const  \
{                                                                                       \
    Q_UNUSED(r)                                                                         \
    /* Note the extra paranteses around validOps. */                                    \
    if(((validOps) & AtomicMathematician::Operator(op)) == op)                                                         \
        return AtomicTypeVisitorResult::Ptr(new mather());                              \
    else                                                                                \
        return AtomicTypeVisitorResult::Ptr();                                          \
}

#define implReportingMathVisit(ownerClass, visitor, mather, validOps)                   \
AtomicTypeVisitorResult::Ptr                                                            \
ownerClass##MathematicianLocator::visit(const visitor *, const qint16 op,               \
                                        const SourceLocationReflection *const r) const  \
{                                                                                       \
    /* Note the extra paranteses around validOps. */                                    \
    if(((validOps) & AtomicMathematician::Operator(op)) == op)                                                         \
        return AtomicTypeVisitorResult::Ptr(new mather(r));                             \
    else                                                                                \
        return AtomicTypeVisitorResult::Ptr();                                          \
}

#define implRevReportingMathVisit(ownerClass, visitor, mather, validOps)                \
AtomicTypeVisitorResult::Ptr                                                            \
ownerClass##MathematicianLocator::visit(const visitor *, const qint16 op,               \
                                        const SourceLocationReflection *const r) const  \
{                                                                                       \
    /* Note the extra paranteses around validOps. */                                    \
    if(((validOps) & AtomicMathematician::Operator(op)) == op)                                                         \
        return AtomicTypeVisitorResult::Ptr(new OperandSwitcherMathematician(           \
                                            AtomicMathematician::Ptr(new mather(r))));  \
    else                                                                                \
        return AtomicTypeVisitorResult::Ptr();                                          \
}

static const AtomicMathematician::Operators AllMathOperators(AtomicMathematician::Add       |
                                                             AtomicMathematician::Div       |
                                                             AtomicMathematician::IDiv      |
                                                             AtomicMathematician::Mod       |
                                                             AtomicMathematician::Multiply  |
                                                             AtomicMathematician::Substract);

static const AtomicMathematician::Operators DivMultiply(AtomicMathematician::Multiply       |
                                                        AtomicMathematician::Div);

static const AtomicMathematician::Operators DurationOps(AtomicMathematician::Div            |
                                                        AtomicMathematician::Substract      |
                                                        AtomicMathematician::Add);

static const AtomicMathematician::Operators DTOps(AtomicMathematician::Substract            |
                                                  AtomicMathematician::Add);

implReportingMathVisit(Double,           DecimalType,            DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Double,           DoubleType,             DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Double,           FloatType,              DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Double,           IntegerType,            DoubleMathematician,    AllMathOperators)
implRevReportingMathVisit(Double,           YearMonthDurationType,  DurationNumericMathematician,  AtomicMathematician::Multiply)
implRevReportingMathVisit(Double,           DayTimeDurationType,    DurationNumericMathematician,  AtomicMathematician::Multiply)

implReportingMathVisit(Float,            DecimalType,            FloatMathematician,     AllMathOperators)
implReportingMathVisit(Float,            DoubleType,    DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Float,            FloatType,              FloatMathematician,     AllMathOperators)
implReportingMathVisit(Float,            IntegerType,            FloatMathematician,     AllMathOperators)
implRevReportingMathVisit(Float,            YearMonthDurationType,  DurationNumericMathematician,  AtomicMathematician::Multiply)
implRevReportingMathVisit(Float,            DayTimeDurationType,    DurationNumericMathematician,  AtomicMathematician::Multiply)

implReportingMathVisit(Decimal, DecimalType,            DecimalMathematician,   AllMathOperators)
implReportingMathVisit(Decimal,          DoubleType,    DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Decimal,          FloatType,              FloatMathematician,     AllMathOperators)
implReportingMathVisit(Decimal, IntegerType,            DecimalMathematician,   AllMathOperators)
implRevReportingMathVisit(Decimal,          YearMonthDurationType,  DurationNumericMathematician,  AtomicMathematician::Multiply)
implRevReportingMathVisit(Decimal,          DayTimeDurationType,    DurationNumericMathematician,  AtomicMathematician::Multiply)

implReportingMathVisit(Integer, DecimalType,            DecimalMathematician,   AllMathOperators)
implReportingMathVisit(Integer,          DoubleType,    DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Integer,          FloatType,              FloatMathematician,     AllMathOperators)
implReportingMathVisit(Integer, IntegerType,            IntegerMathematician,   AllMathOperators)
implRevReportingMathVisit(Integer,          YearMonthDurationType,  DurationNumericMathematician,  AtomicMathematician::Multiply)
implRevReportingMathVisit(Integer,          DayTimeDurationType,    DurationNumericMathematician,  AtomicMathematician::Multiply)

implRevReportingMathVisit(DayTimeDuration,  DateTimeType,           DateTimeDurationMathematician,       AtomicMathematician::Add)
implRevReportingMathVisit(DayTimeDuration,  DateType,               DateTimeDurationMathematician,       AtomicMathematician::Add)
implMathVisit(DayTimeDuration,  DayTimeDurationType,    DurationDurationMathematician, DurationOps)
implReportingMathVisit(DayTimeDuration,  DecimalType,   DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(DayTimeDuration,  DoubleType,    DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(DayTimeDuration,  FloatType,     DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(DayTimeDuration,  IntegerType,   DurationNumericMathematician,  DivMultiply)
implRevReportingMathVisit(DayTimeDuration,  SchemaTimeType,               DateTimeDurationMathematician,       AtomicMathematician::Add)

implRevReportingMathVisit(YearMonthDuration, DateTimeType,          DateTimeDurationMathematician,       AtomicMathematician::Add)
implRevReportingMathVisit(YearMonthDuration, DateType,              DateTimeDurationMathematician,       AtomicMathematician::Add)
implReportingMathVisit(YearMonthDuration, DecimalType,  DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(YearMonthDuration, DoubleType,   DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(YearMonthDuration, FloatType,    DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(YearMonthDuration, IntegerType,  DurationNumericMathematician,  DivMultiply)
implMathVisit(YearMonthDuration, YearMonthDurationType, DurationDurationMathematician, DurationOps)

implMathVisit(Date,              DateType,              AbstractDateTimeMathematician,
              AtomicMathematician::Substract)
implReportingMathVisit(Date,     YearMonthDurationType, DateTimeDurationMathematician,       DTOps)
implReportingMathVisit(Date,     DayTimeDurationType,   DateTimeDurationMathematician,       DTOps)

implMathVisit(SchemaTime,              SchemaTimeType,              AbstractDateTimeMathematician,
              AtomicMathematician::Substract)
implReportingMathVisit(SchemaTime,     DayTimeDurationType,   DateTimeDurationMathematician,       DTOps)

implMathVisit(DateTime,          DateTimeType,          AbstractDateTimeMathematician,
              AtomicMathematician::Substract)
implReportingMathVisit(DateTime, YearMonthDurationType, DateTimeDurationMathematician,       DTOps)
implReportingMathVisit(DateTime, DayTimeDurationType,   DateTimeDurationMathematician,       DTOps)

#undef implMathVisit
#undef implReportingMathVisit
#undef implRevReportingMathVisit

// vim: et:ts=4:sw=4:sts=4

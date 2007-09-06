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
***************************************************************************
*/

#include "AtomicComparators.h"

#include "AtomicComparatorLocators.h"

using namespace Patternist;

static const AtomicComparator::Operators AllCompOperators(AtomicComparator::OperatorNotEqual        |
                                                          AtomicComparator::OperatorGreaterOrEqual  |
                                                          AtomicComparator::OperatorLessOrEqual);
/* --------------------------------------------------------------- */
#define addVisitor(owner, type, comp, validOps)                                 \
AtomicTypeVisitorResult::Ptr                                                    \
owner##ComparatorLocator::visit(const type *,                                   \
                                const qint16 op,                                \
                                const SourceLocationReflection *const) const    \
{                                                                               \
    /* Note the extra paranteses around validOps. */                            \
    if(((validOps) & AtomicComparator::Operator(op)) == op)                     \
        return AtomicTypeVisitorResult::Ptr(new comp());                        \
    else                                                                        \
        return AtomicTypeVisitorResult::Ptr();                                  \
}
/* --------------------------------------------------------------- */

/* ----------- xs:string, xs:anyURI, xs:untypedAtomic  ----------- */
addVisitor(String,  StringType,         StringComparator,
           AllCompOperators)
addVisitor(String,  UntypedAtomicType,  StringComparator,
           AllCompOperators)
addVisitor(String,  AnyURIType,         StringComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* ------------------------- xs:hexBinary ------------------------ */
addVisitor(HexBinary,   HexBinaryType,        BinaryDataComparator,
           AtomicComparator::OperatorEqual |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ----------------------- xs:base64Binary ----------------------- */
addVisitor(Base64Binary,    Base64BinaryType,    BinaryDataComparator,
           AtomicComparator::OperatorEqual |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:boolean ------------------------- */
addVisitor(Boolean,     BooleanType,        BooleanComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:double -------------------------- */
addVisitor(Double,      DoubleType,     AbstractFloatComparator,
           AllCompOperators)
addVisitor(Double,      FloatType,      AbstractFloatComparator,
           AllCompOperators)
addVisitor(Double,      DecimalType,    AbstractFloatComparator,
           AllCompOperators)
addVisitor(Double,      IntegerType,    AbstractFloatComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* --------------------------- xs:float -------------------------- */
addVisitor(Float,   DoubleType,     AbstractFloatComparator,
           AllCompOperators)
addVisitor(Float,   FloatType,      AbstractFloatComparator,
           AllCompOperators)
addVisitor(Float,   DecimalType,    AbstractFloatComparator,
           AllCompOperators)
addVisitor(Float,   IntegerType,    AbstractFloatComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:decimal ------------------------- */
addVisitor(Decimal,     DoubleType,     AbstractFloatComparator,
           AllCompOperators)
addVisitor(Decimal,     FloatType,      AbstractFloatComparator,
           AllCompOperators)
addVisitor(Decimal,     DecimalType,    DecimalComparator,
           AllCompOperators)
addVisitor(Decimal,     IntegerType,    DecimalComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* ------------------------- xs:integer -------------------------- */
addVisitor(Integer,     DoubleType,     AbstractFloatComparator,
           AllCompOperators)
addVisitor(Integer,     FloatType,      AbstractFloatComparator,
           AllCompOperators)
addVisitor(Integer,     DecimalType,    DecimalComparator,
           AllCompOperators)
addVisitor(Integer,     IntegerType,    IntegerComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:QName --------------------------- */
addVisitor(QName,       QNameType,          QNameComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:gYear --------------------------- */
addVisitor(GYear,       GYearType,          AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:gDay ---------------------------- */
addVisitor(GDay,        GDayType,           AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:gMonth -------------------------- */
addVisitor(GMonth,      GMonthType,         AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------------ xs:gYearMonth ------------------------ */
addVisitor(GYearMonth,  GYearMonthType,     AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------------ xs:gMonthDay ------------------------- */
addVisitor(GMonthDay,   GMonthDayType,      AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------------ xs:dateTime -------------------------- */
addVisitor(DateTime,    DateTimeType,    AbstractDateTimeComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:time ---------------------------- */
addVisitor(SchemaTime,        SchemaTimeType,       AbstractDateTimeComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:date ---------------------------- */
addVisitor(Date,        DateType,       AbstractDateTimeComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* ------------------------ xs:duration -------------------------- */
addVisitor(Duration,        DayTimeDurationType,        AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(Duration,        DurationType,               AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(Duration,        YearMonthDurationType,      AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------ xs:dayTimeDuration ------------------------ */
addVisitor(DayTimeDuration,     DayTimeDurationType,    AbstractDurationComparator,
           AllCompOperators)
addVisitor(DayTimeDuration,     DurationType,           AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(DayTimeDuration,     YearMonthDurationType,  AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------- xs:yearMonthDuration --------------------- */
addVisitor(YearMonthDuration,   DayTimeDurationType,    AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(YearMonthDuration,   DurationType,           AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(YearMonthDuration,   YearMonthDurationType,  AbstractDurationComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */
#undef addVisitor

// vim: et:ts=4:sw=4:sts=4

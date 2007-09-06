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

#include "AtomicCasterLocator.h"

using namespace Patternist;

#define implCasterVisit(type)                                                                           \
AtomicTypeVisitorResult::Ptr AtomicCasterLocator::visit(const type *,                                   \
                                                        const SourceLocationReflection *const) const    \
{                                                                                                       \
    return AtomicTypeVisitorResult::Ptr();                                                              \
}

implCasterVisit(AnyAtomicType)
implCasterVisit(AnyURIType)
implCasterVisit(Base64BinaryType)
implCasterVisit(BooleanType)
implCasterVisit(DateTimeType)
implCasterVisit(DateType)
implCasterVisit(DayTimeDurationType)
implCasterVisit(DecimalType)
implCasterVisit(DoubleType)
implCasterVisit(DurationType)
implCasterVisit(FloatType)
implCasterVisit(GDayType)
implCasterVisit(GMonthDayType)
implCasterVisit(GMonthType)
implCasterVisit(GYearMonthType)
implCasterVisit(GYearType)
implCasterVisit(HexBinaryType)
implCasterVisit(IntegerType)
implCasterVisit(NOTATIONType)
implCasterVisit(QNameType)
implCasterVisit(StringType)
implCasterVisit(SchemaTimeType)
implCasterVisit(UntypedAtomicType)
implCasterVisit(YearMonthDurationType)

#undef implCasterVisit

// vim: et:ts=4:sw=4:sts=4

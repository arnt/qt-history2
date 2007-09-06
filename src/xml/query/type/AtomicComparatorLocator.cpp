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

#include "AtomicComparatorLocator.h"

using namespace Patternist;

AtomicComparatorLocator::AtomicComparatorLocator()
{
}

AtomicComparatorLocator::~AtomicComparatorLocator()
{
}

#define implCompVisit(type)                                                             \
AtomicTypeVisitorResult::Ptr                                                            \
AtomicComparatorLocator::visit(const type *,                                            \
                               const qint16,                                            \
                               const SourceLocationReflection *const) const             \
{                                                                                       \
    return AtomicTypeVisitorResult::Ptr();                                              \
}

implCompVisit(AnyAtomicType)
implCompVisit(AnyURIType)
implCompVisit(Base64BinaryType)
implCompVisit(BooleanType)
implCompVisit(DateTimeType)
implCompVisit(DateType)
implCompVisit(DayTimeDurationType)
implCompVisit(DecimalType)
implCompVisit(DoubleType)
implCompVisit(DurationType)
implCompVisit(FloatType)
implCompVisit(GDayType)
implCompVisit(GMonthDayType)
implCompVisit(GMonthType)
implCompVisit(GYearMonthType)
implCompVisit(GYearType)
implCompVisit(HexBinaryType)
implCompVisit(IntegerType)
implCompVisit(NOTATIONType)
implCompVisit(QNameType)
implCompVisit(StringType)
implCompVisit(SchemaTimeType)
implCompVisit(UntypedAtomicType)
implCompVisit(YearMonthDurationType)
#undef implCompVisit

// vim: et:ts=4:sw=4:sts=4

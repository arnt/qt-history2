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

#include "qatomicmathematicianlocator_p.h"

using namespace Patternist;

AtomicMathematicianLocator::~AtomicMathematicianLocator()
{
}

#define implVisit(type)                                                                                     \
AtomicTypeVisitorResult::Ptr AtomicMathematicianLocator::visit(const type *, const qint16,                  \
                                                               const SourceLocationReflection *const) const \
{                                                                                                           \
    return AtomicTypeVisitorResult::Ptr();                                                                  \
}

implVisit(AnyAtomicType)
implVisit(AnyURIType)
implVisit(Base64BinaryType)
implVisit(BooleanType)
implVisit(DateTimeType)
implVisit(DateType)
implVisit(DayTimeDurationType)
implVisit(DecimalType)
implVisit(DoubleType)
implVisit(DurationType)
implVisit(FloatType)
implVisit(GDayType)
implVisit(GMonthDayType)
implVisit(GMonthType)
implVisit(GYearMonthType)
implVisit(GYearType)
implVisit(HexBinaryType)
implVisit(IntegerType)
implVisit(NOTATIONType)
implVisit(QNameType)
implVisit(StringType)
implVisit(SchemaTimeType)
implVisit(UntypedAtomicType)
implVisit(YearMonthDurationType)
#undef implVisit

// vim: et:ts=4:sw=4:sts=4

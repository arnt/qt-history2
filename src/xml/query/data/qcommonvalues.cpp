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

#include <limits>

#include "AbstractFloat.h"
#include "AnyURI.h"
#include "Boolean.h"
#include "Decimal.h"
#include "Integer.h"
#include "ListIterator.h"
#include "AtomicString.h"
#include "UntypedAtomic.h"

#include "CommonValues.h"

using namespace Patternist;

// STATIC DATA
const AtomicString::Ptr               CommonValues::EmptyString
                                    (new AtomicString(QString(QLatin1String(""))));
const AtomicString::Ptr               CommonValues::TrueString
                                    (new AtomicString(QLatin1String("true")));
const AtomicString::Ptr               CommonValues::FalseString
                                    (new AtomicString(QLatin1String("false")));

const UntypedAtomic::Ptr        CommonValues::UntypedAtomicTrue
                                    (new UntypedAtomic(QLatin1String("true")));
const UntypedAtomic::Ptr        CommonValues::UntypedAtomicFalse
                                    (new UntypedAtomic(QLatin1String("false")));

const AtomicValue::Ptr              CommonValues::BooleanTrue
                                    (new Boolean(true));
const AtomicValue::Ptr              CommonValues::BooleanFalse(new Boolean(false));

const AtomicValue::Ptr               CommonValues::DoubleNaN
                                    (Double::fromValue(std::numeric_limits<xsDouble>::quiet_NaN()));

const AtomicValue::Ptr                CommonValues::FloatNaN
                                    (Float::fromValue(std::numeric_limits<xsFloat>::quiet_NaN()));

const Item                          CommonValues::IntegerZero
                                    (Integer::fromValue(0));

const AtomicValue::Ptr               CommonValues::EmptyAnyURI
                                    (AnyURI::fromValue(QLatin1String("")));

const AtomicValue::Ptr               CommonValues::DoubleOne
                                    (Double::fromValue(1));
const AtomicValue::Ptr                CommonValues::FloatOne
                                    (Float::fromValue(1));
const AtomicValue::Ptr              CommonValues::DecimalOne
                                    (Decimal::fromValue(1));
const Item                          CommonValues::IntegerOne
                                    (Integer::fromValue(1));
const Item                          CommonValues::IntegerOneNegative
                                    (Integer::fromValue(-1));

const AtomicValue::Ptr               CommonValues::DoubleZero
                                    (Double::fromValue(0));
const AtomicValue::Ptr                CommonValues::FloatZero
                                    (Float::fromValue(0));
const AtomicValue::Ptr              CommonValues::DecimalZero
                                    (Decimal::fromValue(0));

const Item::EmptyIterator::Ptr  CommonValues::emptyIterator
                                    (new Item::EmptyIterator());

const AtomicValue::Ptr               CommonValues::NegativeInfDouble
                                    (Double::fromValue(-std::numeric_limits<xsDouble>::infinity()));
const AtomicValue::Ptr               CommonValues::InfDouble
                                    (Double::fromValue(std::numeric_limits<xsDouble>::infinity()));
const AtomicValue::Ptr                CommonValues::NegativeInfFloat
                                    (Float::fromValue(-std::numeric_limits<xsFloat>::infinity()));
const AtomicValue::Ptr                CommonValues::InfFloat
                                    (Float::fromValue(std::numeric_limits<xsFloat>::infinity()));

const DayTimeDuration::Ptr      CommonValues::DayTimeDurationZero
                                    (DayTimeDuration::fromSeconds(0));
const DayTimeDuration::Ptr      CommonValues::YearMonthDurationZero
                                    (YearMonthDuration::fromComponents(true, 0, 0));

// vim: et:ts=4:sw=4:sts=4


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

#include "AnyURI.h"
#include "Date.h"
#include "DateTime.h"
#include "DayTimeDuration.h"
#include "Integer.h"
#include "Literal.h"
#include "AtomicString.h"
#include "Time.h"

#include "ContextFNs.h"

using namespace Patternist;

Item PositionFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    Q_ASSERT(context);
    return Integer::fromValue(context->contextPosition());
}

Item LastFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO << "returning" << context->contextSize();
    Q_ASSERT(context);
    return Integer::fromValue(context->contextSize());
}

Item ImplicitTimezoneFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return toItem(context->implicitTimezone());
}

Item CurrentDateTimeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return toItem(DateTime::fromDateTime(context->currentDateTime()));
}

Item CurrentDateFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return toItem(Date::fromDateTime(context->currentDateTime()));
}

Item CurrentTimeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return toItem(Time::fromDateTime(context->currentDateTime()));
}

Expression::Ptr StaticBaseURIFN::typeCheck(const StaticContext::Ptr &context,
                                           const SequenceType::Ptr &reqType)
{
    /* Our base URI can never be undefined. */
    return wrapLiteral(toItem(AnyURI::fromValue(context->baseURI())), context, this)->typeCheck(context, reqType);
}

Expression::Ptr DefaultCollationFN::typeCheck(const StaticContext::Ptr &context,
                                              const SequenceType::Ptr &reqType)
{
    return wrapLiteral(AtomicString::fromValue(context->defaultCollation().toString()), context, this)->typeCheck(context, reqType);
}

// vim: et:ts=4:sw=4:sts=4

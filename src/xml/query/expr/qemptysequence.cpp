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

#include "qcommonvalues_p.h"
#include "qcommonsequencetypes_p.h"
#include "qemptysequencetype_p.h"
#include "qlistiterator_p.h"

#include "qemptysequence_p.h"

using namespace Patternist;

Expression::Ptr EmptySequence::create(const Expression *const replacementFor,
                                      const StaticContext::Ptr &context)
{
    Q_ASSERT(replacementFor);
    Q_ASSERT(context);

    EmptySequence *const retval = new EmptySequence();
    context->addLocation(retval, context->locationFor(replacementFor));
    return Expression::Ptr(retval);
}

Item::Iterator::Ptr EmptySequence::evaluateSequence(const DynamicContext::Ptr &) const
{
    return CommonValues::emptyIterator;
}

Item EmptySequence::evaluateSingleton(const DynamicContext::Ptr &) const
{
    return Item();
}

void EmptySequence::evaluateToSequenceReceiver(const DynamicContext::Ptr &) const
{
}

ItemType::Ptr EmptySequence::type() const
{
    return CommonSequenceTypes::Empty;
}

SequenceType::Ptr EmptySequence::staticType() const
{
    return CommonSequenceTypes::Empty;
}

bool EmptySequence::evaluateEBV(const DynamicContext::Ptr &) const
{
    return false;
}

QString EmptySequence::stringValue() const
{
    return QString();
}

ExpressionVisitorResult::Ptr EmptySequence::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID EmptySequence::id() const
{
    return IDEmptySequence;
}

// vim: et:ts=4:sw=4:sts=4

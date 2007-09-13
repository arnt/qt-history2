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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qdebug_p.h"
#include "qgenericsequencetype_p.h"
#include "qlistiterator_p.h"

#include "qliteralsequence_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

LiteralSequence::LiteralSequence(const Item::List &list) : m_list(list)
{
    qDebug() << Q_FUNC_INFO << m_list.count();
    Q_ASSERT(list.size() >= 2);
}

Item::Iterator::Ptr LiteralSequence::evaluateSequence(const DynamicContext::Ptr &) const
{
    return makeListIterator(m_list);
}

SequenceType::Ptr LiteralSequence::staticType() const
{
    const Item::List::const_iterator end(m_list.constEnd());
    Item::List::const_iterator it(m_list.constBegin());

    /* Load the first item. */
    ItemType::Ptr t((*it).type());
    ++it;

    for(; end != it; ++it)
        t |= (*it).type();

    return makeGenericSequenceType(t, Cardinality::fromCount(m_list.size()));
}

ExpressionVisitorResult::Ptr LiteralSequence::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID LiteralSequence::id() const
{
    return IDExpressionSequence;
}

Expression::Properties LiteralSequence::properties() const
{
    return IsEvaluated;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE

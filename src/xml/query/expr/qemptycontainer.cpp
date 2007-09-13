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

#include <QList>

#include "qemptycontainer_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

Expression::List EmptyContainer::operands() const
{
    return Expression::List();
}

SequenceType::List EmptyContainer::expectedOperandTypes() const
{
    return SequenceType::List();
}

void EmptyContainer::setOperands(const Expression::List &)
{
}

bool EmptyContainer::compressOperands(const StaticContext::Ptr &)
{
    return true;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE

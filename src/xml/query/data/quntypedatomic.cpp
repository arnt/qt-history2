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
#include "qdebug_p.h"

#include "quntypedatomic_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

UntypedAtomic::UntypedAtomic(const QString &s) : AtomicString(s)
{
}

UntypedAtomic::Ptr UntypedAtomic::fromValue(const QString &value)
{
    return UntypedAtomic::Ptr(new UntypedAtomic(value));
}

ItemType::Ptr UntypedAtomic::type() const
{
    return BuiltinTypes::xsUntypedAtomic;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE

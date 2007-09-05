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

#include "BuiltinTypes.h"
#include "Debug.h"

#include "UntypedAtomic.h"

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

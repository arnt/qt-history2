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

#include "BuiltinTypes.h"
#include "GenericSequenceType.h"

#include "Aggregator.h"

using namespace Patternist;

SequenceType::Ptr Aggregator::staticType() const
{
    const SequenceType::Ptr t(m_operands.first()->staticType());
    ItemType::Ptr itemType(t->itemType());

    /* Since we have types that are derived from xs:integer, this ensures that
     * the static type is xs:integer even if the argument is for
     * instance xs:unsignedShort. */
    if(BuiltinTypes::xsInteger->xdtTypeMatches(itemType) &&
       !itemType->xdtTypeMatches(BuiltinTypes::xsInteger))
    {
        itemType = BuiltinTypes::xsInteger;
    }

    return makeGenericSequenceType(itemType,
                                   t->cardinality().toWithoutMany());
}

// vim: et:ts=4:sw=4:sts=4

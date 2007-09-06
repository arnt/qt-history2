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

#include "ListIterator.h"
#include "BuiltinTypes.h"

#include "Item.h"

using namespace Patternist;

Item::Iterator::Ptr Item::typedValue() const
{
    if(isAtomicValue())
        return makeSingletonIterator(Item(atomicValue));
    else
        return node.typedValue();
}

ItemType::Ptr NodeModel::typeFromKind(const Node::NodeKind nodeKind)
{
    switch(nodeKind)
    {
        case Node::Element:
            return BuiltinTypes::element;
        case Node::Attribute:
            return BuiltinTypes::attribute;
        case Node::Text:
            return BuiltinTypes::text;
        case Node::ProcessingInstruction:
            return BuiltinTypes::pi;
        case Node::Comment:
            return BuiltinTypes::comment;
        case Node::Document:
            return BuiltinTypes::document;
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "A node type that doesn't exist in the XPath Data Model was encountered.");
            return ItemType::Ptr(); /* Dummy, silence compiler warning. */
        }
    }
}

// vim: et:ts=4:sw=4:sts=4

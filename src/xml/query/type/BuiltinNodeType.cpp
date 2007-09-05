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

/**
 * @file
 * @short This file is included by BuiltintNodeType.h.
 * If you need includes in this file, put them in BuiltintNodeType.h, outside of the namespace.
 */

template <const Node::NodeKind kind>
BuiltinNodeType<kind>::BuiltinNodeType()
{
}

template <const Node::NodeKind kind>
bool BuiltinNodeType<kind>::xdtTypeMatches(const ItemType::Ptr &other) const
{
    if(!other->isNodeType())
        return false;

    return *static_cast<const BuiltinNodeType *>(other.get()) == *this
            ? true
            : xdtTypeMatches(other->xdtSuperType());
}

template <const Node::NodeKind kind>
bool BuiltinNodeType<kind>::itemMatches(const Item &item) const
{
    Q_ASSERT(item);

    return item.isNode() &&
           item.asNode().kind() == kind;
}

template <const Node::NodeKind kind>
ItemType::Ptr BuiltinNodeType<kind>::atomizedType() const
{
    switch(kind)
    {
        /* Fallthrough all these. */
        case Node::Attribute:
        case Node::Document:
        case Node::Element:
        case Node::Text:
            return BuiltinTypes::xsUntypedAtomic;
        case Node::ProcessingInstruction:
        /* Fallthrough. */
        case Node::Comment:
            return BuiltinTypes::xsString;
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "Encountered invalid XPath Data Model node type.");
            return BuiltinTypes::xsUntypedAtomic;
        }
    }
}

template <const Node::NodeKind kind>
QString BuiltinNodeType<kind>::displayName(const NamePool::Ptr &) const
{
    switch(kind)
    {
        case Node::Element:
            return QLatin1String("element()");
        case Node::Document:
            return QLatin1String("document()");
        case Node::Attribute:
            return QLatin1String("attribute()");
        case Node::Text:
            return QLatin1String("text()");
        case Node::ProcessingInstruction:
            return QLatin1String("processing-instruction()");
        case Node::Comment:
            return QLatin1String("comment()");
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "Encountered invalid XPath Data Model node type.");
            return QString();
        }
    }
}

template <const Node::NodeKind kind>
ItemType::Ptr BuiltinNodeType<kind>::xdtSuperType() const
{
    return BuiltinTypes::node;
}

template <const Node::NodeKind kind>
Node::NodeKind BuiltinNodeType<kind>::nodeKind() const
{
    return kind;
}


// vim: et:ts=4:sw=4:sts=4

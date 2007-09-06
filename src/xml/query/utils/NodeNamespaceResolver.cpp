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

#include "Item.h"
#include "NamePool.h"

#include "NodeNamespaceResolver.h"

using namespace Patternist;

NodeNamespaceResolver::NodeNamespaceResolver(const Item &item) : m_node(item.asNode())
{
    Q_ASSERT(m_node);
}

void NodeNamespaceResolver::addBinding(const NamespaceBinding nb)
{
    Q_UNUSED(nb);
    Q_ASSERT_X(false, Q_FUNC_INFO, "Calling this function for this sub-class makes little sense.");
}

QName::NamespaceCode NodeNamespaceResolver::lookupNamespaceURI(const QName::PrefixCode prefix) const
{
    const QName::NamespaceCode ns = m_node.namespaceForPrefix(prefix);

    if(ns == NoBinding)
    {
        if(prefix == StandardPrefixes::empty)
            return StandardNamespaces::empty;
        else
            return NoBinding;
    }
    else
        return ns;
}

NamespaceResolver::Bindings NodeNamespaceResolver::bindings() const
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "Not implemented.");
    return NamespaceResolver::Bindings();
}

// vim: et:ts=4:sw=4:sts=4

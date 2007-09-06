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

#include "NamePool.h"

#include "GenericNamespaceResolver.h"

using namespace Patternist;

GenericNamespaceResolver::GenericNamespaceResolver(const Bindings &list) : m_bindings(list)
{
}

void GenericNamespaceResolver::addBinding(const NamespaceBinding nb)
{
    if(nb.namespaceURI() == StandardNamespaces::UndeclarePrefix)
        m_bindings.remove(nb.prefix());
    else
        m_bindings.insert(nb.prefix(), nb.namespaceURI());
}

QName::NamespaceCode GenericNamespaceResolver::lookupNamespaceURI(const QName::PrefixCode prefix) const
{
    return m_bindings.value(prefix, NoBinding);
}

NamespaceResolver::Ptr GenericNamespaceResolver::defaultXQueryBindings()
{
    Bindings list;

    list.insert(StandardPrefixes::xml,    StandardNamespaces::xml);
    list.insert(StandardPrefixes::xs,     StandardNamespaces::xs);
    list.insert(StandardPrefixes::xsi,    StandardNamespaces::xsi);
    list.insert(StandardPrefixes::fn,     StandardNamespaces::fn);
    list.insert(StandardPrefixes::local,  StandardNamespaces::local);
    list.insert(StandardPrefixes::empty,  StandardNamespaces::empty);

    return NamespaceResolver::Ptr(new GenericNamespaceResolver(list));
}

NamespaceResolver::Bindings GenericNamespaceResolver::bindings() const
{
    return m_bindings;
}

// vim: et:ts=4:sw=4:sts=4

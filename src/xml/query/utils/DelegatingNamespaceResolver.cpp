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

#include "NamePool.h"

#include "DelegatingNamespaceResolver.h"

using namespace Patternist;

DelegatingNamespaceResolver::DelegatingNamespaceResolver(const NamespaceResolver::Ptr &resolver) : m_nsResolver(resolver)
{
    Q_ASSERT(m_nsResolver);
}

QName::NamespaceCode DelegatingNamespaceResolver::lookupNamespaceURI(const QName::PrefixCode prefix) const
{
    const QName::NamespaceCode val(m_bindings.value(prefix, NoBinding));

    if(val == NoBinding)
        return m_nsResolver->lookupNamespaceURI(prefix);
    else
        return val;
}

NamespaceResolver::Bindings DelegatingNamespaceResolver::bindings() const
{
    Bindings bs(m_nsResolver->bindings());
    const Bindings::const_iterator end(m_bindings.constEnd());
    Bindings::const_iterator it(m_bindings.constBegin());

    for(; it != end; ++it)
        bs.insert(it.key(), it.value());

    return bs;
}

void DelegatingNamespaceResolver::addBinding(const NamespaceBinding nb)
{
    if(nb.namespaceURI() == StandardNamespaces::UndeclarePrefix)
        m_bindings.remove(nb.prefix());
    else
        m_bindings.insert(nb.prefix(), nb.namespaceURI());
}

// vim: et:ts=4:sw=4:sts=4

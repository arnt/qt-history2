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

#ifndef Patternist_NodeNamespaceResolver_H
#define Patternist_NodeNamespaceResolver_H

#include <QHash>

#include "qnamespaceresolver_p.h"
#include "qitem_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short A NamespaceResolver that use a Node's in-scope namespace
     * bindings for resolving namespaces.
     *
     * @ingroup Patternist
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NodeNamespaceResolver : public NamespaceResolver
    {
    public:
        NodeNamespaceResolver(const Item &item);

        virtual void addBinding(const NamespaceBinding nb);
        virtual QName::NamespaceCode lookupNamespaceURI(const QName::PrefixCode prefix) const;
        virtual Bindings bindings() const;

    private:
        const Node m_node;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

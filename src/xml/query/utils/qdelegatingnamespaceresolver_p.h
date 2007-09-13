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

#ifndef Patternist_DelegatingNamespaceResolver_H
#define Patternist_DelegatingNamespaceResolver_H

#include <QHash>

#include "qnamespaceresolver_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{

    /**
     * @short Contains a set of bindings, plus a pointer to another resolver
     * which is delegates requests to, in case it can't handle a lookup on its
     * own.
     *
     * @ingroup Patternist
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DelegatingNamespaceResolver : public NamespaceResolver
    {
    public:
        DelegatingNamespaceResolver(const NamespaceResolver::Ptr &ns);

        virtual void addBinding(const NamespaceBinding nb);

        virtual QName::NamespaceCode lookupNamespaceURI(const QName::PrefixCode prefix) const;
        virtual Bindings bindings() const;

    private:
        const NamespaceResolver::Ptr m_nsResolver;
        Bindings m_bindings;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

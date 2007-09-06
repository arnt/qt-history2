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

#ifndef Patternist_NamespaceResolver_H
#define Patternist_NamespaceResolver_H

template<typename A, typename B> class QHash;

#include <QSharedData>

#include "PlainSharedPtr.h"
#include "NamespaceBinding.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Base class for namespace resolvers.
     *
     * @ingroup Patternist
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NamespaceResolver : public QSharedData
    {
    public:
        enum Constants
        {
            NoBinding = -1
        };

        typedef PlainSharedPtr<NamespaceResolver> Ptr;

        /**
         * A list of namespace bindings. The key is the prefix, and the value is
         * the namespace URI.
         */
        typedef QHash<QName::PrefixCode, QName::NamespaceCode> Bindings;

        NamespaceResolver();
        virtual ~NamespaceResolver();

        /**
         * Adds the mapping from @p prefix to @p namespaceURI to
         * this NamespaceResolver. If this NamespaceResolver already contains
         * a binding involving @p prefix, the old binding is replaced.
         */
        virtual void addBinding(const NamespaceBinding nb) = 0;

        /**
         * Resolves the @p prefix to the corresponding namespace URI. If no binding
         * exists for @p prefix, NoBinding is returned.
         *
         * @returns the namespace corresponding to @p prefix.
         */
        virtual QName::NamespaceCode lookupNamespaceURI(const QName::PrefixCode prefix) const = 0;

        /**
         * @returns all bindings this NamespaceResolver handles.
         */
        virtual Bindings bindings() const = 0;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

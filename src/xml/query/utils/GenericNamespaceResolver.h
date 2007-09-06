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

#ifndef Patternist_GenericNamespaceResolver_H
#define Patternist_GenericNamespaceResolver_H

#include <QHash>

#include "NamespaceResolver.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Generic namespace resolver which resolves lookups against entries in a QHash.
     *
     * @ingroup Patternist
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GenericNamespaceResolver : public NamespaceResolver
    {
    public:
        GenericNamespaceResolver(const Bindings &list);
        virtual void addBinding(const NamespaceBinding nb);

        virtual QName::NamespaceCode lookupNamespaceURI(const QName::PrefixCode prefix) const;

        /**
         * Returns a GenericNamespaceResolver containing the following bindings:
         *
         * - <tt>xml</tt> = <tt>http://www.w3.org/XML/1998/namespace</tt>
         * - <tt>xml</tt> = <tt>http://www.w3.org/XML/1998/namespace</tt>
         * - <tt>xs</tt> = <tt>http://www.w3.org/2001/XMLSchema</tt>
         * - <tt>xsi</tt> = <tt>http://www.w3.org/2001/XMLSchema-instance</tt>
         * - <tt>fn</tt> = <tt>http://www.w3.org/2005/xpath-functions</tt>
         * - <tt>xdt</tt> = <tt>http://www.w3.org/2005/xpath-datatypes</tt>
         */
        static NamespaceResolver::Ptr defaultXQueryBindings();

        virtual Bindings bindings() const;

    private:
        /**
         * The key is the prefix, the value the namespace URI.
         */
        Bindings m_bindings;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

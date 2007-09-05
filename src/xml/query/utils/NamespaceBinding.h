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

#ifndef Patternist_NamespaceBinding_H
#define Patternist_NamespaceBinding_H

template<typename T> class QVector;

#include "QName.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Represents a namespace binding: a prefix, and a namespace URI.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NamespaceBinding
    {
    public:
        enum
        {
            InvalidCode = -1
        };

        typedef QVector<NamespaceBinding> Vector;

        inline NamespaceBinding() : m_prefix(InvalidCode),
                                    m_namespace(InvalidCode)
        {
        }

        inline NamespaceBinding(const QName::PrefixCode p,
                                const QName::NamespaceCode n) : m_prefix(p),
                                                                m_namespace(n)
        {
        }

        inline bool operator==(const NamespaceBinding &other) const
        {
            return m_prefix == other.m_prefix &&
                   m_namespace == other.m_namespace;
        }

        inline QName::PrefixCode prefix() const
        {
            return m_prefix;
        }

        inline QName::NamespaceCode namespaceURI() const
        {
            return m_namespace;
        }

        inline bool isNull() const
        {
            return m_prefix == InvalidCode;
        }

        /**
         * @short Constructs a NamespaceBinding whose prefix and namespace is
         * taken from @p qName.
         *
         * The local name in @p qName is ignored. @p qName may not be null.
         */
        static inline NamespaceBinding fromQName(const QName qName)
        {
            Q_ASSERT(!qName.isNull());
            return NamespaceBinding(qName.prefix(), qName.namespaceURI());
        }

    private:
        QName::PrefixCode      m_prefix;
        QName::NamespaceCode   m_namespace;
    };

    /**
     * @relates NamespaceBinding
     */
    static inline uint qHash(const NamespaceBinding nb)
    {
        return (nb.prefix() << 16) + nb.namespaceURI();
    }

}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

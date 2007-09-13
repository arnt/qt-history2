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

#ifndef Patternist_QName_H
#define Patternist_QName_H

#include <QtDebug>

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    class QName;
    static inline uint qHash(const QName name);

    class QName
    {
    private:
        /**
         * @short Various constants used in the NamePool and QName.
         *
         * Setting of the mask enums use essentially this:
         * @code
         * // Fills the bits from beg to end with 1s and leaves the rest as 0.
         *
         * template<typename IntegralT>
         *  inline IntegralT bitmask(IntegralT begin, IntegralT end)
         *  {
         *      IntegralT filled_bits = (1 << (end - begin + 1)) - 1;
         *      return filled_bits << begin;
         *  }
         * @endcode
         *
         * The masks, such as LocalNameMask, are positive. That is, for the
         * area which the name resides, the bits are set.
         */
        enum Constants
        {
            LocalNameOffset     = 0,
            LocalNameLength     = 11,
            NamespaceOffset     = 11,
            NamespaceLength     = 9,
            PrefixLength        = 10,
            InvalidCode         = 1 << 31,
            NamespaceMask       = ((1 << ((NamespaceOffset + NamespaceLength) - NamespaceOffset)) - 1) << NamespaceOffset,
            LocalNameMask       = ((1 << ((LocalNameOffset + LocalNameLength) - LocalNameOffset)) - 1) << LocalNameOffset
        };

    public:

        /**
         * Stores a namespace URI, local name and prefix.
         */
        typedef qint32 Code;

        enum PublicConstants
        {
            PrefixOffset        = 21,
            PrefixMask          = ((1 << ((PrefixOffset + PrefixLength) - PrefixOffset)) - 1) << PrefixOffset,
            MaximumPrefixes     = (PrefixMask >> PrefixOffset) - 1,
            MaximumLocalNames   = (LocalNameMask >> LocalNameOffset) - 1,
            MaximumNamespaces   = (NamespaceMask >> NamespaceOffset) - 1,
            ExpandedNameMask    = LocalNameMask | NamespaceMask,
            /**
             * This value should not be public, but it's too messy rearrange
             * all the enum values it depends on.
             *
             * Think twice before using this value.
             */
            LexicalQNameMask    = LocalNameMask | PrefixMask
        };

        typedef qint16 NamespaceCode;
        typedef NamespaceCode PrefixCode;
        typedef NamespaceCode LocalNameCode;

        /**
         * @short Constructs an invalid QName.
         */
        inline QName() : m_qNameCode(InvalidCode)
        {
        }

        inline QName(const NamespaceCode uri,
                     const LocalNameCode ln,
                     const PrefixCode p = 0) : m_qNameCode((uri << NamespaceOffset) +
                                                           (ln << LocalNameOffset)  +
                                                           (p << PrefixOffset))
        {
            /* We can't use members like prefix() here because if one of the
             * values are to large, they would overflow into the others. */
            Q_ASSERT_X(p <= MaximumPrefixes, "",
                       qPrintable(QString::fromLatin1("NamePool prefix limits: max is %1, therefore %2 exceeds.").arg(MaximumPrefixes).arg(p)));
            Q_ASSERT_X(ln <= MaximumLocalNames, "",
                       qPrintable(QString::fromLatin1("NamePool local name limits: max is %1, therefore %2 exceeds.").arg(MaximumLocalNames).arg(ln)));
            Q_ASSERT_X(uri <= MaximumNamespaces, "",
                       qPrintable(QString::fromLatin1("NamePool namespace limits: max is %1, therefore %2 exceeds.").arg(MaximumNamespaces).arg(uri)));
        }

        inline LocalNameCode localName() const
        {
            return (m_qNameCode & LocalNameMask) >> LocalNameOffset;
        }

        inline PrefixCode prefix() const
        {
            return (m_qNameCode & PrefixMask) >> PrefixOffset;
        }

        /**
         * Returns @c true if this QName has a prefix that isn't
         * the empty string, otherwise false.
         */
        inline bool hasPrefix() const
        {
            return prefix() != 0;
        }

        /**
         * Returns @c true if this QName has a non-empty namespace.
         */
        inline bool hasNamespace() const
        {
            return namespaceURI() != 0;
        }

        inline NamespaceCode namespaceURI() const
        {
            return (m_qNameCode & NamespaceMask) >> NamespaceOffset;
        }

        inline bool isNull() const
        {
            return m_qNameCode == InvalidCode;
        }

        inline bool operator==(const QName other) const
        {
            return (m_qNameCode & ExpandedNameMask) == (other.m_qNameCode & ExpandedNameMask);
        }

        inline bool operator!=(const QName other) const
        {
            return !operator==(other);
        }

        inline bool isLexicallyEqual(const QName other) const
        {
            return (m_qNameCode & LexicalQNameMask) == (other.m_qNameCode & LexicalQNameMask);
        }

        inline void setPrefix(const PrefixCode c)
        {
            m_qNameCode |= (c << PrefixOffset);
        }

        inline void setNamespaceURI(const NamespaceCode c)
        {
            m_qNameCode |= (c << NamespaceOffset);
        }

        inline void setLocalName(const LocalNameCode c)
        {
            m_qNameCode |= (c << LocalNameOffset);
        }

        /**
         * Returns the internal code that contains the local name, prefix and
         * namespace components. It is opaque when used outside QName, but can
         * be useful when one wants to put a QName in a hash, and the prefix is
         * significant.
         */
        inline Code code() const
        {
            return m_qNameCode;
        }

        friend inline uint qHash(const QName);

        static inline QName fromPublicCode(const int c)
        {
            return QName(c);
        }

    private:
        inline QName(const int c) : m_qNameCode(c)
        {
        }

        Code m_qNameCode;
    };

    /**
     * Returns a hash code for @p name. It ignores the prefix.
     *
     * @relates QName
     */
    static inline uint qHash(const QName name)
    {
        return name.m_qNameCode & QName::ExpandedNameMask;
    }
}

Q_DECLARE_TYPEINFO(Patternist::QName, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

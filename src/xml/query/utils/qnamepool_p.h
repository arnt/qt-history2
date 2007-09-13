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

#ifndef Patternist_NamePool_H
#define Patternist_NamePool_H

#include <QHash>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSharedData>
#include <QString>
#include <QtDebug>
#include <QVector>

#include "qnamespacebinding_p.h"
#include "qplainsharedptr_p.h"
#include "qprimitives_p.h"
#include "qqname_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Store names such as namespace bindings and QNames and allows them to
     * be referenced in efficient ways.
     *
     * Once a string have been inserted it stays there and cannot be removed. The
     * only way to deallocate any string in the NamePool is to deallocate the
     * NamePool itself, as a whole.
     *
     * This class is not only reentrant, it is thread-safe in all sense of the
     * word. All functions of this class can be called concurrently. This is
     * achieved by internal locking.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo Use QSubStrings, we can save very many heap allocations by that.
     * @todo Check limits
     */
    class Q_AUTOTEST_EXPORT NamePool : public QSharedData
    {
    public:
        typedef PlainSharedPtr<NamePool> Ptr;

    private:
        friend class StandardNamespaces;
        friend class NamespaceBinding;

        enum
        {
            NoSuchValue         = -1,
            /**
             * This must be identical to the amount of members in
             * StandardNamespaces.
             */
            StandardNamespaceCount = 8,
            StandardPrefixCount = 7,
            StandardLocalNameCount = 128
        };

        QVector<QString> m_prefixes;
        QVector<QString> m_namespaces;
        QVector<QString> m_localNames;

        /**
         * This hash contains no essential data, but speeds up
         * finding a prefix in m_prefixes by mapping a prefix(the key) to
         * the index into m_prefixes(which the value is).
         *
         * In other words, one can skip this variable at the cost of having
         * to linearly loop over prefixes, in order to find the entry.
         */
        QHash<QString, QName::PrefixCode> m_prefixMapping;

        /**
         * Same as m_prefixMapping but applies for URIs, and hence m_namespaces instead
         * of m_prefixes.
         */
        QHash<QString, QName::NamespaceCode> m_namespaceMapping;

        QHash<QString, QName::LocalNameCode> m_localNameMapping;

        enum DefaultCapacities
        {
            DefaultPrefixCapacity = 10,
            DefaultURICapacity = DefaultPrefixCapacity,
            /**
             * It looks like it's quite common with 40-60 different local names per XML
             * vocabulary. For background, see:
             *
             * - http://englich.wordpress.com/2007/01/11/representing-xml/
             * - http://englich.wordpress.com/2007/01/09/xmlstat/
             */
            DefaultLocalNameCapacity = 60
        };

    public:
        NamePool();
        NamespaceBinding allocateBinding(const QString &prefix, const QString &uri);

        QName allocateQName(const QString &uri, const QString &localName, const QString &prefix = QString());

        inline QName allocateQName(const QName::NamespaceCode uri, const QString &ln)
        {
            /* We don't lock here, but we do in allocateLocalName(). */
            return QName(uri, allocateLocalName(ln));
        }

        inline const QString &stringForLocalName(const QName::LocalNameCode code) const
        {
            QReadLocker l(mutableLock());
            return m_localNames.at(code);
        }

        inline const QString &stringForPrefix(const QName::PrefixCode code) const
        {
            QReadLocker l(mutableLock());
            return m_prefixes.at(code);
        }

        inline const QString &stringForNamespace(const QName::NamespaceCode code) const
        {
            QReadLocker l(mutableLock());
            return m_namespaces.at(code);
        }

        QString displayName(const QName qName) const;

        inline QString toLexical(const QName qName) const
        {
            QReadLocker l(mutableLock());
            Q_ASSERT_X(!qName.isNull(), "", "It makes no sense to call toLexical() on a null name.");

            if(qName.hasPrefix())
            {
                const QString &p = m_prefixes.at(qName.prefix());
                return p + QLatin1Char(':') + m_localNames.at(qName.localName());
            }
            else
                return m_localNames.at(qName.localName());
        }

        inline QName::NamespaceCode allocateNamespace(const QString &uri)
        {
            QWriteLocker l(&lock);
            return unlockedAllocateNamespace(uri);
        }

        inline QName::LocalNameCode allocateLocalName(const QString &ln)
        {
            QWriteLocker l(&lock);
            return unlockedAllocateLocalName(ln);
        }

        inline QName::PrefixCode allocatePrefix(const QString &prefix)
        {
            QWriteLocker l(&lock);
            return unlockedAllocatePrefix(prefix);
        }

    private:
        /**
         * @note This function can not be called concurrently.
         */
        QName::NamespaceCode unlockedAllocateNamespace(const QString &uri);

        /**
         * @note This function can not be called concurrently.
         */
        QName::LocalNameCode unlockedAllocateLocalName(const QString &ln);

        /**
         * It's assumed that @p prefix is a valid @c NCName.
         *
         * @note This function can not be called concurrently.
         */
        QName::PrefixCode unlockedAllocatePrefix(const QString &prefix);

        Q_DISABLE_COPY(NamePool)

        /**
         * @note This function can not be called concurrently.
         */
        const QString &displayPrefix(const QName::NamespaceCode nc) const;

        inline QReadWriteLock *mutableLock() const
        {
            return const_cast<QReadWriteLock *>(&lock);
        }

        QReadWriteLock lock;
    };

    /**
     * @short Formats QName.
     *
     * @relates QName
     */
    static inline QString formatKeyword(const NamePool::Ptr &np, const QName name)
    {
        return QLatin1String("<span class='XQuery-keyword'>")   +
               escape(np->displayName(name))                    +
               QLatin1String("</span>");
    }

    class StandardNamespaces
    {
    public:
        enum
        {
            /**
             * @short A special value that when passed as the namespace part
             * to NamespaceResolver::addBinding(), undeclares the prefix.
             *
             * This is used by the namespace prolog declaration.
             *
             * This value is never added to the name pool.
             */
            UndeclarePrefix = -2,

            /**
             * This does not mean empty in the sense of "empty", but
             * in the sense of an empyt string, "".
             *
             * Its value, zero, is significant.
             */
            empty = 0,
            fn,
            local,
            xml,
            xmlns,
            xs,
            xsi,
            xslt
        };
    };

    // const QString * a = &*qset.insert("foo");
    class StandardLocalNames
    {
    public:
        enum
        {
            abs,
            adjust_dateTime_to_timezone,
            adjust_date_to_timezone,
            adjust_time_to_timezone,
            arity,
            avg,
            base,
            base_uri,
            boolean,
            ceiling,
            codepoint_equal,
            codepoints_to_string,
            collection,
            compare,
            concat,
            contains,
            count,
            current_date,
            current_dateTime,
            current_time,
            data,
            dateTime,
            day_from_date,
            day_from_dateTime,
            days_from_duration,
            deep_equal,
            default_collation,
            distinct_values,
            doc,
            doc_available,
            document_uri,
            empty,
            encode_for_uri,
            ends_with,
            error,
            escape_html_uri,
            exactly_one,
            exists,
            False,
            floor,
            function_available,
            function_name,
            hours_from_dateTime,
            hours_from_duration,
            hours_from_time,
            id,
            idref,
            implicit_timezone,
            index_of,
            in_scope_prefixes,
            insert_before,
            iri_to_uri,
            is_schema_aware,
            lang,
            last,
            local_name,
            local_name_from_QName,
            lower_case,
            matches,
            max,
            min,
            minutes_from_dateTime,
            minutes_from_duration,
            minutes_from_time,
            month_from_date,
            month_from_dateTime,
            months_from_duration,
            name,
            namespace_uri,
            namespace_uri_for_prefix,
            namespace_uri_from_QName,
            nilled,
            node_name,
            normalize_space,
            normalize_unicode,
            Not,
            number,
            one_or_more,
            position,
            prefix_from_QName,
            product_name,
            product_version,
            property_name,
            QName,
            remove,
            replace,
            resolve_QName,
            resolve_uri,
            reverse,
            root,
            round,
            round_half_to_even,
            seconds_from_dateTime,
            seconds_from_duration,
            seconds_from_time,
            sourceValue,
            starts_with,
            static_base_uri,
            string,
            string_join,
            string_length,
            string_to_codepoints,
            subsequence,
            substring,
            substring_after,
            substring_before,
            sum,
            supports_backwards_compatibility,
            supports_serialization,
            system_property,
            timezone_from_date,
            timezone_from_dateTime,
            timezone_from_time,
            tokenize,
            trace,
            translate,
            True,
            unordered,
            upper_case,
            vendor,
            vendor_url,
            version,
            xml,
            xmlns,
            year_from_date,
            year_from_dateTime,
            years_from_duration,
            zero_or_one
        };
    };

    class StandardPrefixes
    {
    public:
        enum
        {
            /**
             * This does not mean empty in the sense of "empty", but
             * in the sense of an empyt string, "".
             *
             * Its value, zero, is significant.
             */
            empty = 0,
            fn,
            local,
            xml,
            xmlns,
            xs,
            xsi
        };
    };
}

Q_DECLARE_TYPEINFO(Patternist::NamePool::Ptr, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

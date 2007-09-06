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

#ifndef Patternist_Locale_H
#define Patternist_Locale_H

#include <QString>
#include <QUrl>

#include "Cardinality.h"
#include "NamePool.h"
#include "Primitives.h"

/**
 * @file
 * @short Contains functions used for formatting arguments, such as keywords and paths,
 * in translated strings.
 *
 * This file was originally called PatternistLocale.h. However, it broke build on MS
 * Windows, because it override the locale.h system header.
 */

namespace Patternist
{
    /**
     * @short Translates by calling QObject::tr().
     */
    static inline QString tr(const char *sourceText, const char *comment = 0, int n = -1)
    {
        return QObject::tr(sourceText, comment, n);
    }

    static inline QString formatKeyword(const QString &keyword)
    {
        return QLatin1String("<span class='XQuery-keyword'>")   +
               escape(keyword)                                  +
               QLatin1String("</span>");
    }

    static inline QString formatKeyword(const char *const keyword)
    {
        return formatKeyword(QLatin1String(keyword));
    }

    static inline QString formatKeyword(const QChar keyword)
    {
        return formatKeyword(QString(keyword));
    }

    /**
     * @short Formats ItemType and SequenceType.
     */
    template<typename T>
    static inline QString formatType(const NamePool::Ptr &np, const T &type)
    {
        Q_ASSERT(type);
        return QLatin1String("<span class='XQuery-type'>")  +
               escape(type->displayName(np))                +
               QLatin1String("</span>");
    }

    /**
     * @short Formats Cardinality.
     */
    static inline QString formatType(const Cardinality &type)
    {
        return QLatin1String("<span class='XQuery-type'>")                      +
               escape(type.displayName(Cardinality::IncludeExplanation))        +
               QLatin1String("</span>");
    }

    /**
     * @short Formats @p uri for display.
     *
     * @note It's not guaranteed that URIs being formatted are valid. That can
     * be an arbitrary string.
     */
    static inline QString formatURI(const QUrl &uri)
    {
        return QLatin1String("<span class='XQuery-uri'>")       +
               escape(uri.toString(QUrl::RemovePassword))       +
               QLatin1String("</span>");
    }

    /**
     * @short Formats @p uri, that's considered to be a URI, for display.
     *
     * @p uri does not have to be a valid QUrl or valid instance of @c
     * xs:anyURI.
     */
    static inline QString formatURI(const QString &uri)
    {
        const QUrl realURI(uri);
        return formatURI(realURI);
    }

    static inline QString formatData(const QString &data)
    {
        return QLatin1String("<span class='XQuery-data'>")  +
               escape(data)                                 +
               QLatin1String("</span>");
    }

    /**
     * This is an overload, provided for convenience.
     */
    static inline QString formatData(const xsInteger data)
    {
        return formatData(QString::number(data));
    }

    /**
     * This is an overload, provided for convenience.
     */
    static inline QString formatData(const char *const data)
    {
        return formatData(QLatin1String(data));
    }

    /**
     * This is an overload, provided for convenience.
     */
    static inline QString formatData(const QLatin1Char &data)
    {
        return formatData(QString(data));
    }

    /**
     * Formats an arbitrary expression, such as a regular expression
     * or XQuery query.
     */
    static inline QString formatExpression(const QString &expr)
    {
        return QLatin1String("<span class='XQuery-expression'>")    +
               escape(expr)                                         +
               QLatin1String("</span>");
    }
}

#ifdef Q_NO_TYPESAFE_FLAGS
#error "Patternist does not compile with Q_NO_TYPESAFE_FLAGS set, because the code uses negative enum values. qglobal.h has typedef uint Flags."
#endif

#ifdef QT_NO_EXCEPTIONS
#error "Patternist use exceptions and cannot be built without."
#endif

#endif
// vim: et:ts=4:sw=4:sts=4

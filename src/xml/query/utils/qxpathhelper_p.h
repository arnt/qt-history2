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

#ifndef Patternist_XPathHelper_H
#define Patternist_XPathHelper_H

#include "qcommonnamespaces_p.h"
#include "qitem_p.h"
#include "qpatternistlocale_p.h"
#include "qreportcontext_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Contains helper and utility functions.
     *
     * The common denominator of its functions is that they do not fit in
     * well elsewhere, such as in a particular class. It is preferred if XPathHelper
     * goes away, and that functions are in more specific classes.
     *
     * @ingroup Patternist
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class XPathHelper
    {
    public:
        /**
         * Determines whether @p ncName is a valid NCName. For example, "body" is, but
         * "xhtml:body" or "body "(note the whitespace) is not. In other words, only canonical
         * lexical representations of @c xs:NCName are valid, not arbitrary lexical representations.
         */
        static bool isNCName(const QString &ncName);

        /**
         * Determines whether @p qName is a valid QName. For example, "body" and "xhtml:body"
         * is, but "xhtml::body" or "x:body "(note the whitespace) is not.
         */
        static bool isQName(const QString &qName);

        /**
         * @short Splits @p qName into @p ncName and @p prefix.
         *
         * Here's an example:
         * @include Example-XPathHelper-splitQName.cpp
         *
         * @note @p qName must be a valid QName, and that is not checked.
         */
        static void splitQName(const QString &qName, QString &prefix, QString &ncName);

        /**
         * Determines whether @p ns is a reserved namespace.
         *
         * @see <a href="http://www.w3.org/TR/xslt20/#reserved-namespaces">XSL Transformations
         * (XSLT) Version 2.0, 3.2 Reserved Namespaces</a>
         * @see <a href="http://www.w3.org/TR/xquery/#FunctionDeclns">XQuery 1.0: An XML
         * Query Language, 4.15 Function Declaration</a>
         * @returns @c true if @p ns is a reserved namespace, otherwise @c false.
         */
        static bool isReservedNamespace(const QName::NamespaceCode ns);

        /**
         * Determines whether @p collation is a supported string collation. If it is
         * not, error code @p code is raised via @p context.
         */
        template<const ReportContext::ErrorCode code, typename TReportContext>
        static inline void checkCollationSupport(const QString &collation,
                                                 const TReportContext &context,
                                                 const SourceLocationReflection *const r)
        {
            Q_ASSERT(context);
            Q_ASSERT(r);

            if(collation != QLatin1String(CommonNamespaces::UNICODE_COLLATION))
            {
                context->error(tr("Only the Unicode Codepoint "
                                  "Collation is supported(%1). Subsequently is %2 "
                                  "unsupported")
                                  .arg(formatURI(QLatin1String(CommonNamespaces::UNICODE_COLLATION)))
                                  .arg(formatURI(collation)),
                               code, r);
            }
        }

    private:
        /**
         * @short This default constructor has no definition, in order to avoid
         * instantiation, since it makes no sense to instantiate this class.
         */
        inline XPathHelper();

        Q_DISABLE_COPY(XPathHelper)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

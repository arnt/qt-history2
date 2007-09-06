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

#ifndef Patternist_QNameConstructor_H
#define Patternist_QNameConstructor_H

#include "SingleContainer.h"
#include "BuiltinTypes.h"
#include "PatternistLocale.h"
#include "XPathHelper.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Creates an @c xs:QName value from a lexical QName using
     * statically known namespace bindings.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class QNameConstructor : public SingleContainer
    {
    public:

        QNameConstructor(const Expression::Ptr &source,
                         const NamespaceResolver::Ptr &nsResolver);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;

        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * Expands @p lexicalQName, which is a lexical representation of a QName such as "x:body", into
         * a QName using @p nsResolver to supply the namespace bindings.
         *
         * If @p lexicalQName is lexically invalid @p InvalidQName is raised via @p context, or if
         * no namespace binding does not exists for a prefix(if any) in @p lexicalQName, @p NoBinding
         * is raised via @p context.
         *
         * If @p asForAttribute is @c true, the name is considered to be for an
         * attribute in some way, and @p lexicalQName will not pick up the
         * default namespace if it doesn't have a prefix.
         *
         * @p nsResolver is parameterized meaning the function can be instantiated with either
         * DynamicContext or StaticContext.
         */
        template<typename TReportContext,
                 const ReportContext::ErrorCode InvalidQName,
                 const ReportContext::ErrorCode NoBinding>
        static
        QName expandQName(const QString &lexicalQName,
                          const TReportContext &context,
                          const NamespaceResolver::Ptr &nsResolver,
                          const SourceLocationReflection *const r,
                          const bool asForAttribute = false);

        /**
         * Resolves the namespace prefix @p prefix to its namespace if it exists, or
         * raised ReportContext::XPST0081 otherwise.
         *
         * @returns the namespace URI corresponding to @p prefix
         */
        static QName::NamespaceCode namespaceForPrefix(const QName::PrefixCode prefix,
                                                       const StaticContext::Ptr &context,
                                                       const SourceLocationReflection *const r);

        virtual const SourceLocationReflection *actualReflection() const;

    private:
        const NamespaceResolver::Ptr m_nsResolver;
    };

    template<typename TReportContext,
             const ReportContext::ErrorCode InvalidQName,
             const ReportContext::ErrorCode NoBinding>
    QName QNameConstructor::expandQName(const QString &lexicalQName,
                                        const TReportContext &context,
                                        const NamespaceResolver::Ptr &nsResolver,
                                        const SourceLocationReflection *const r,
                                        const bool asForAttribute)
    {
        Q_ASSERT(nsResolver);
        Q_ASSERT(context);

        if(XPathHelper::isQName(lexicalQName))
        {
            QString prefix;
            QString local;
            XPathHelper::splitQName(lexicalQName, prefix, local);
            const QName::NamespaceCode nsCode = asForAttribute && prefix.isEmpty() ? QName::NamespaceCode(StandardNamespaces::empty)
                                                                                   : (nsResolver->lookupNamespaceURI(context->namePool()->allocatePrefix(prefix)));

            if(nsCode == NamespaceResolver::NoBinding)
            {
                context->error(tr("No namespace binding exist for "
                                  "the prefix %1 in %2").arg(formatKeyword(prefix),
                                                             formatKeyword(lexicalQName)),
                               NoBinding,
                               r);
                return QName(); /* Silence compiler warning. */
            }
            else
                return context->namePool()->allocateQName(context->namePool()->stringForNamespace(nsCode), local, prefix);
        }
        else
        {
            context->error(tr("%1 is an invalid %2")
                              .arg(formatData(lexicalQName))
                              .arg(formatType(context->namePool(), BuiltinTypes::xsQName)),
                           InvalidQName,
                           r);
            return QName(); /* Silence compiler warning. */
        }
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

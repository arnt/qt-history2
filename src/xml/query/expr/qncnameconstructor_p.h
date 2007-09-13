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

#ifndef Patternist_NCNameConstructor_H
#define Patternist_NCNameConstructor_H

#include "qsinglecontainer_p.h"
#include "qpatternistlocale_p.h"
#include "qxpathhelper_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Ensures the lexical space of the string value of the Item returned
     * from its child Expression is an NCName.
     *
     * @note It doesn't actually construct an @c xs:NCName. It only ensures the lexical
     * space is an @c NCName. The atomic value can be of any string type, such as @c xs:untypedAtomic
     * of @c xs:string.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class NCNameConstructor : public SingleContainer
    {
    public:

        NCNameConstructor(const Expression::Ptr &source);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * Expands @p lexicalTarget, which is a lexical representation of a QName such as "x:body", into
         * a SharedQName using @p nsResolver to supply the namespace bindings.
         *
         * If @p lexicalTarget is lexically invalid @p InvalidQName is raised via @p context, or if
         * no namespace binding does not exists for a prefix(if any) in @p lexicalTarget, @p NoBinding
         * is raised via @p context.
         *
         * @p nsResolver is paramterized meaning the function can be instantiated with either
         * DynamicContext or StaticContext.
         */
        template<typename TReportContext,
                 const ReportContext::ErrorCode NameIsXML,
                 const ReportContext::ErrorCode LexicallyInvalid>
        static inline
        QString validateTargetName(const QString &lexicalQName,
                                   const TReportContext &context,
                                   const SourceLocationReflection *const r);
    private:

        /**
         * This translation string is put here in order to avoid duplicate messages and
         * hence reduce work for translators and increase consistency.
         */
        static
        const QString nameIsXML(const QString &lexTarget)
        {
            return tr("The target name in a processing instruction "
                        "cannot equal %1(case ignorant) and therefore "
                        "is %2 invalid").arg(formatKeyword("xml"),
                                             formatKeyword(lexTarget));
        }
    };

    template<typename TReportContext,
             const ReportContext::ErrorCode NameIsXML,
             const ReportContext::ErrorCode LexicallyInvalid>
    inline
    QString NCNameConstructor::validateTargetName(const QString &lexicalTarget,
                                                  const TReportContext &context,
                                                  const SourceLocationReflection *const r)
    {
        Q_ASSERT(context);
        qDebug() << "Validating:" << lexicalTarget;

        if(XPathHelper::isNCName(lexicalTarget))
        {
            if(QString::compare(QLatin1String("xml"), lexicalTarget, Qt::CaseInsensitive) == 0)
            {
                context->error(nameIsXML(lexicalTarget), NameIsXML, r);
                return QString();
            }
            else
                return lexicalTarget;
        }
        else
        {
            context->error(tr("%1 is not a valid target name in a "
                              "processing instruction, it must be a valid %2 "
                              "value such as %3")
                              .arg(formatKeyword(lexicalTarget))
                              .arg(formatType(context->namePool(), BuiltinTypes::xsNCName))
                              .arg(formatKeyword("my-name.123")),
                           LexicallyInvalid,
                           r);
            return QString();
        }
    }
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

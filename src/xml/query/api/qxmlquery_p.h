/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QXMLQUERY_P_H
#define QXMLQUERY_P_H

#include <QDir>
#include <QVariant>
#include <QUrl>

#include "qabstractmessagehandler.h"
#include "qabstracturiresolver.h"
#include "qsourcelocation.h"
#include "qxmlname.h"

#include "qacceltreebuilder_p.h"
#include "qacceltreeresourceloader_p.h"
#include "qcommonsequencetypes_p.h"
#include "ColoringMessageHandler.h"
#include "qexception_p.h"
#include "qexpressionfactory_p.h"
#include "qgenericdynamiccontext_p.h"
#include "qgenericstaticcontext_p.h"
#include "qnamepool_p.h"
#include "qqobjectnodemodel_p.h"
#include "qvariableloader_p.h"

QT_BEGIN_HEADER

class QXmlQueryPrivate
{
private:
    /**
     * This variable isn't an enum, because it is too large for that.
     */
    static const quint64 PrefixMask = ((Q_UINT64_C(1) << 14) - 1) << QXmlName::PrefixOffset;

public:
    enum ComponentForUpdate
    {
        QuerySource = 1,
        VariableBindings = 2
    };

    typedef QFlags<ComponentForUpdate> ComponentsForUpdate;

    inline QXmlQueryPrivate() : componentsForUpdate(QuerySource | VariableBindings)
                              , hasEvaluationError(false)
                              , namePool(new Patternist::NamePool())
                              , messageHandler(0)
                              , m_qobjectNodeModel(new Patternist::QObjectNodeModel(namePool))
    {
    }

    ~QXmlQueryPrivate()
    {
    }

    bool isValid()
    {
        return !querySource.isEmpty() && expression();
    }

    inline Patternist::GenericStaticContext::Ptr staticContext()
    {
        if(!componentsForUpdate.testFlag(VariableBindings) && m_staticContext)
            return m_staticContext;

        if(!messageHandler)
            messageHandler = new ColoringMessageHandler();

        m_staticContext = Patternist::GenericStaticContext::Ptr(new Patternist::GenericStaticContext(namePool, messageHandler, queryURI));
        const Patternist::ResourceLoader::Ptr resourceLoader(new Patternist::AccelTreeResourceLoader(namePool));
        m_staticContext->setResourceLoader(resourceLoader);

        VariableLoader::Ptr loader(new VariableLoader(variableBindings, m_qobjectNodeModel));
        m_staticContext->setExternalVariableLoader(loader);
        componentsForUpdate &= ~VariableBindings; /* Remove the VariableBindings flag. */

        return m_staticContext;
    }

    inline Patternist::GenericDynamicContext::Ptr dynamicContext()
    {
        const Patternist::StaticContext::Ptr context(staticContext());
        Q_ASSERT(context);

        Patternist::GenericDynamicContext::Ptr dynContext(new Patternist::GenericDynamicContext(namePool, context->messageHandler(),
                                                                                                context->sourceLocations()));

        dynContext->setNodeBuilder(Patternist::NodeBuilder::Ptr(new Patternist::AccelTreeBuilder(QUrl(), QUrl(), namePool)));
        dynContext->setResourceLoader(context->resourceLoader());
        dynContext->setExternalVariableLoader(context->externalVariableLoader());

        return dynContext;
    }

    static inline QUrl normalizeQueryURI(const QUrl &uri)
    {
        Q_ASSERT_X(uri.isEmpty() || uri.isValid(), Q_FUNC_INFO,
                   "The URI passed to QXmlQuery::setQuery() must be valid or empty.");
        if(uri.isEmpty())
            return QUrl::fromLocalFile(QDir::currentPath() + QLatin1Char('/'));
        else
            return uri;
    }

    Patternist::Expression::Ptr expression()
    {
        if(!componentsForUpdate)
        {
            Q_ASSERT_X(m_expr, Q_FUNC_INFO,
                       "If we're flagged as !needsCompile, we obviously should have an expression compiled.");
            return m_expr;
        }

        try
        {
            Patternist::ExpressionFactory::Ptr factory(new Patternist::ExpressionFactory());

            m_expr = factory->createExpression(querySource, staticContext(),
                                               Patternist::ExpressionFactory::XQuery10,
                                               Patternist::CommonSequenceTypes::ZeroOrMoreItems,
                                               queryURI);
            componentsForUpdate &= ~QuerySource; /* Remove the QuerySource flag. */
        }
        catch(const Patternist::Exception &)
        {
            m_expr.reset();
        }

        return m_expr;
    }

    static inline Patternist::QName toPoolName(const QXmlName &name)
    {
        return Patternist::QName::fromPublicCode((name.m_code & QXmlName::ExpandedNameMask) |
                                                 ((name.m_code & PrefixMask) >> (QXmlName::PrefixOffset - Patternist::QName::PrefixOffset)));
    }

    static inline QXmlName fromPoolName(const Patternist::QName &poolName)
    {
        const Patternist::QName::Code c(poolName.code());

        QXmlName retval;
        retval.m_code = (c & Patternist::QName::ExpandedNameMask) | (quint64(poolName.prefix()) << 50);
        return retval;
    }

    ComponentsForUpdate                     componentsForUpdate;
    bool                                    hasEvaluationError;
    VariableLoader::BindingHash             variableBindings;
    Patternist::NamePool::Ptr               namePool;
    QAbstractMessageHandler::Ptr            messageHandler;
    QString                                 querySource;
    /**
     * Must be absolute and valid.
     */
    QUrl                                    queryURI;
    QAbstractUriResolver::Ptr               uriResolver;

private:
    Patternist::GenericStaticContext::Ptr   m_staticContext;
    VariableLoader::Ptr                     m_variableLoader;
    /**
     * This is the AST for the query.
     */
    Patternist::Expression::Ptr             m_expr;
    const Patternist::QObjectNodeModel::Ptr m_qobjectNodeModel;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QXmlQueryPrivate::ComponentsForUpdate)

QT_END_HEADER
#endif
// vim: et:ts=4:sw=4:sts=4

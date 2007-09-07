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

#include <QCoreApplication>

/* Patternist */
#include "qbasictypesfactory_p.h"
#include "qcommonnamespaces_p.h"
#include "qgenericdynamiccontext_p.h"
#include "qfunctionfactorycollection_p.h"
#include "qgenericnamespaceresolver_p.h"
#include "qlistiterator_p.h"

#include "qgenericstaticcontext_p.h"

using namespace Patternist;

GenericStaticContext::GenericStaticContext(const NamePool::Ptr &np,
                                           QAbstractMessageHandler *const handler,
                                           const QUrl &aBaseURI)
                  : m_boundarySpacePolicy(BSPStrip),
                    m_constructionMode(CMPreserve),
                    m_defaultFunctionNamespace(CommonNamespaces::XFN),
                    m_orderingEmptySequence(Greatest),
                    m_orderingMode(Ordered),
                    m_defaultCollation(QUrl::fromEncoded(CommonNamespaces::UNICODE_COLLATION)),
                    m_baseURI(aBaseURI),
                    m_messageHandler(handler),
                    m_preserveMode(Preserve),
                    m_inheritMode(Inherit),
                    m_namespaceResolver(GenericNamespaceResolver::defaultXQueryBindings()),
                    m_namePool(np)
{
    /* We'll easily have at least this many AST nodes, that we need
     * to track locations for. */
    m_locations.reserve(30);

    Q_ASSERT(np);
    Q_ASSERT(!m_baseURI.isRelative());
}

NamespaceResolver::Ptr GenericStaticContext::namespaceBindings() const
{
    return m_namespaceResolver;
}

FunctionFactory::Ptr GenericStaticContext::functionSignatures() const
{
    return FunctionFactoryCollection::xpath20Factory(m_namePool);
}

DynamicContext::Ptr GenericStaticContext::dynamicContext() const
{
    qDebug() << Q_FUNC_INFO << sourceLocations().count();
    GenericDynamicContext::Ptr context(new GenericDynamicContext(m_namePool, m_messageHandler, sourceLocations()));
    // TODO we have many bugs here..
    context->setResourceLoader(m_resourceLoader);
    return context;
}

SchemaTypeFactory::Ptr GenericStaticContext::schemaDefinitions() const
{
    return BasicTypesFactory::self(m_namePool);
}

QUrl GenericStaticContext::baseURI() const
{
    Q_ASSERT_X(!m_baseURI.isRelative(), Q_FUNC_INFO,
               "The static base-uri must be absolute. This error is most likely caused by misuing the API.");
    return m_baseURI;
}

void GenericStaticContext::setBaseURI(const QUrl &uri)
{
    Q_ASSERT(!uri.isRelative());
    m_baseURI = uri;
}

bool GenericStaticContext::compatModeEnabled() const
{
    return false; // Currently we're fullblown 2.0
}

QUrl GenericStaticContext::defaultCollation() const
{
    return m_defaultCollation;
}

QAbstractMessageHandler * GenericStaticContext::messageHandler() const
{
    return m_messageHandler;
}

void GenericStaticContext::setDefaultCollation(const QUrl &uri)
{
    m_defaultCollation = uri;
}

void GenericStaticContext::setNamespaceBindings(const NamespaceResolver::Ptr &resolver)
{
    Q_ASSERT(resolver);
    m_namespaceResolver = resolver;
}

StaticContext::BoundarySpacePolicy GenericStaticContext::boundarySpacePolicy() const
{
    return m_boundarySpacePolicy;
}

void GenericStaticContext::setBoundarySpacePolicy(const BoundarySpacePolicy policy)
{
    Q_ASSERT(policy == BSPPreserve || policy == BSPStrip);
    m_boundarySpacePolicy = policy;
}

StaticContext::ConstructionMode GenericStaticContext::constructionMode() const
{
    return m_constructionMode;
}

void GenericStaticContext::setConstructionMode(const ConstructionMode mode)
{
    Q_ASSERT(mode == CMPreserve || mode == CMStrip);
    m_constructionMode = mode;
}

StaticContext::OrderingMode GenericStaticContext::orderingMode() const
{
    return m_orderingMode;
}

void GenericStaticContext::setOrderingMode(const OrderingMode mode)
{
    Q_ASSERT(mode == Ordered || mode == Unordered);
    m_orderingMode = mode;
}

StaticContext::OrderingEmptySequence GenericStaticContext::orderingEmptySequence() const
{
    return m_orderingEmptySequence;
}

void GenericStaticContext::setOrderingEmptySequence(const OrderingEmptySequence ordering)
{
    Q_ASSERT(ordering == Greatest || ordering == Least);
    m_orderingEmptySequence = ordering;
}

QString GenericStaticContext::defaultFunctionNamespace() const
{
    return m_defaultFunctionNamespace;
}

void GenericStaticContext::setDefaultFunctionNamespace(const QString &ns)
{
    m_defaultFunctionNamespace = ns;
}


QString GenericStaticContext::defaultElementNamespace() const
{
    return m_defaultElementNamespace;
}

void GenericStaticContext::setDefaultElementNamespace(const QString &ns)
{
    m_defaultElementNamespace = ns;
}

StaticContext::InheritMode GenericStaticContext::inheritMode() const
{
    return m_inheritMode;
}

void GenericStaticContext::setInheritMode(const InheritMode mode)
{
    Q_ASSERT(mode == Inherit || mode == NoInherit);
    m_inheritMode = mode;
}

StaticContext::PreserveMode GenericStaticContext::preserveMode() const
{
    return m_preserveMode;
}

void GenericStaticContext::setPreserveMode(const PreserveMode mode)
{
    Q_ASSERT(mode == Preserve || mode == NoPreserve);
    m_preserveMode = mode;
}

ItemType::Ptr GenericStaticContext::contextItemType() const
{
    return m_contextItemType;
}

void GenericStaticContext::setContextItemType(const ItemType::Ptr &type)
{
    m_contextItemType = type;
}

StaticContext::Ptr GenericStaticContext::copy() const
{
    GenericStaticContext *const retval = new GenericStaticContext(m_namePool, m_messageHandler, m_baseURI);
    const NamespaceResolver::Ptr newSolver(new GenericNamespaceResolver(m_namespaceResolver->bindings()));

    retval->setNamespaceBindings(newSolver);
    retval->setDefaultCollation(m_defaultCollation);
    retval->setBoundarySpacePolicy(m_boundarySpacePolicy);
    retval->setConstructionMode(m_constructionMode);
    retval->setOrderingMode(m_orderingMode);
    retval->setOrderingEmptySequence(m_orderingEmptySequence);
    retval->setDefaultFunctionNamespace(m_defaultFunctionNamespace);
    retval->setInheritMode(m_inheritMode);
    retval->setPreserveMode(m_preserveMode);
    retval->setExternalVariableLoader(m_externalVariableLoader);
    retval->setResourceLoader(m_resourceLoader);
    retval->setContextItemType(m_contextItemType);
    retval->m_locations = m_locations;

    return StaticContext::Ptr(retval);
}

ResourceLoader::Ptr GenericStaticContext::resourceLoader() const
{
    return m_resourceLoader;
}

void GenericStaticContext::setResourceLoader(const ResourceLoader::Ptr &loader)
{
    m_resourceLoader = loader;
}

ExternalVariableLoader::Ptr GenericStaticContext::externalVariableLoader() const
{
    return m_externalVariableLoader;
}

void GenericStaticContext::setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader)
{
    m_externalVariableLoader = loader;
}

NamePool::Ptr GenericStaticContext::namePool() const
{
    return m_namePool;
}

void GenericStaticContext::addLocation(const SourceLocationReflection *const reflection,
                                       const QSourceLocation &location)
{
    Q_ASSERT(!location.isNull());
    Q_ASSERT_X(reflection, Q_FUNC_INFO,
               "The reflection cannot be zero.");
    m_locations.insert(reflection, location);
}

StaticContext::LocationHash GenericStaticContext::sourceLocations() const
{
    return m_locations;
}

QSourceLocation GenericStaticContext::locationFor(const SourceLocationReflection *const reflection) const
{
    return m_locations.value(reflection->actualReflection());
}

QAbstractUriResolver::Ptr GenericStaticContext::uriResolver() const
{
    return m_uriResolver;
}

// vim: et:ts=4:sw=4:sts=4

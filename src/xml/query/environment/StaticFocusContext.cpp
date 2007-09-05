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

/* Patternist */
#include "BasicTypesFactory.h"
#include "FunctionFactoryCollection.h"
#include "GenericNamespaceResolver.h"
#include "CommonNamespaces.h"
#include "GenericDynamicContext.h"

#include "StaticFocusContext.h"

using namespace Patternist;

StaticFocusContext::StaticFocusContext(const ItemType::Ptr &t,
                                       const StaticContext::Ptr &context) : m_contextItemType(t),
                                                                            m_context(context)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(context);
    Q_ASSERT(t);
}

NamespaceResolver::Ptr StaticFocusContext::namespaceBindings() const
{
    return m_context->namespaceBindings();
}

FunctionFactory::Ptr StaticFocusContext::functionSignatures() const
{
    return m_context->functionSignatures();
}

DynamicContext::Ptr StaticFocusContext::dynamicContext() const
{
    return m_context->dynamicContext();
}

SchemaTypeFactory::Ptr StaticFocusContext::schemaDefinitions() const
{
    return m_context->schemaDefinitions();
}

QUrl StaticFocusContext::baseURI() const
{
    return m_context->baseURI();
}

void StaticFocusContext::setBaseURI(const QUrl &uri)
{
    m_context->setBaseURI(uri);
}

bool StaticFocusContext::compatModeEnabled() const
{
    return m_context->compatModeEnabled();
}

QUrl StaticFocusContext::defaultCollation() const
{
    return m_context->defaultCollation();
}

QAbstractMessageHandler * StaticFocusContext::messageHandler() const
{
    return m_context->messageHandler();
}

void StaticFocusContext::setDefaultCollation(const QUrl &uri)
{
    m_context->setDefaultCollation(uri);
}

void StaticFocusContext::setNamespaceBindings(const NamespaceResolver::Ptr &resolver)
{
    m_context->setNamespaceBindings(resolver);
}

StaticContext::BoundarySpacePolicy StaticFocusContext::boundarySpacePolicy() const
{
    return m_context->boundarySpacePolicy();
}

void StaticFocusContext::setBoundarySpacePolicy(const BoundarySpacePolicy policy)
{
    m_context->setBoundarySpacePolicy(policy);
}

StaticContext::ConstructionMode StaticFocusContext::constructionMode() const
{
    return m_context->constructionMode();
}

void StaticFocusContext::setConstructionMode(const ConstructionMode mode)
{
    m_context->setConstructionMode(mode);
}

StaticContext::OrderingMode StaticFocusContext::orderingMode() const
{
    return m_context->orderingMode();
}

void StaticFocusContext::setOrderingMode(const OrderingMode mode)
{
    m_context->setOrderingMode(mode);
}

StaticContext::OrderingEmptySequence StaticFocusContext::orderingEmptySequence() const
{
    return m_context->orderingEmptySequence();
}

void StaticFocusContext::setOrderingEmptySequence(const OrderingEmptySequence ordering)
{
    m_context->setOrderingEmptySequence(ordering);
}

QString StaticFocusContext::defaultFunctionNamespace() const
{
    return m_context->defaultFunctionNamespace();
}

void StaticFocusContext::setDefaultFunctionNamespace(const QString &ns)
{
    m_context->setDefaultFunctionNamespace(ns);
}

QString StaticFocusContext::defaultElementNamespace() const
{
    return m_context->defaultElementNamespace();
}

void StaticFocusContext::setDefaultElementNamespace(const QString &ns)
{
    m_context->setDefaultElementNamespace(ns);
}

StaticContext::InheritMode StaticFocusContext::inheritMode() const
{
    return m_context->inheritMode();
}

void StaticFocusContext::setInheritMode(const InheritMode mode)
{
    m_context->setInheritMode(mode);
}

StaticContext::PreserveMode StaticFocusContext::preserveMode() const
{
    return m_context->preserveMode();
}

void StaticFocusContext::setPreserveMode(const PreserveMode mode)
{
    m_context->setPreserveMode(mode);
}

ItemType::Ptr StaticFocusContext::contextItemType() const
{
    return m_contextItemType;
}

ExternalVariableLoader::Ptr StaticFocusContext::externalVariableLoader() const
{
    return m_context->externalVariableLoader();
}

StaticContext::Ptr StaticFocusContext::copy() const
{
    return StaticContext::Ptr(new StaticFocusContext(m_contextItemType,
                                                     m_context->copy()));
}

ResourceLoader::Ptr StaticFocusContext::resourceLoader() const
{
    return m_context->resourceLoader();
}

NamePool::Ptr StaticFocusContext::namePool() const
{
    return m_context->namePool();
}

void StaticFocusContext::addLocation(const SourceLocationReflection *const reflection,
                                     const QSourceLocation &location)
{
    m_context->addLocation(reflection, location);
}

StaticContext::LocationHash StaticFocusContext::sourceLocations() const
{
    return m_context->sourceLocations();
}

QSourceLocation StaticFocusContext::locationFor(const SourceLocationReflection *const reflection) const
{
    return m_context->locationFor(reflection);
}

QAbstractUriResolver::Ptr StaticFocusContext::uriResolver() const
{
    return m_context->uriResolver();
}

// vim: et:ts=4:sw=4:sts=4

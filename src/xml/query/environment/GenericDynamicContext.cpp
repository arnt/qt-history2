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

#include "CommonValues.h"
#include "Debug.h"
#include "Focus.h"
#include "ListIterator.h"

#include "GenericDynamicContext.h"

using namespace Patternist;

GenericDynamicContext::GenericDynamicContext(const NamePool::Ptr &np,
                                             QAbstractMessageHandler *const errHandler,
                                             const LocationHash &locations) : m_messageHandler(errHandler)
                                                                            , m_currentDateTime(QDateTime::currentDateTime().toTimeSpec(Qt::UTC))
                                                                            , m_namePool(np)
                                                                            , m_locations(locations)
{
    qDebug() << Q_FUNC_INFO << "locations:" << m_locations.count();
    Q_ASSERT(m_messageHandler);
    Q_ASSERT(m_namePool);
}

PlainSharedPtr<DayTimeDuration> GenericDynamicContext::implicitTimezone() const
{
    /* Or what do you prefer, sir? */
    return CommonValues::DayTimeDurationZero;
}

QAbstractMessageHandler * GenericDynamicContext::messageHandler() const
{
    return m_messageHandler;
}

QDateTime GenericDynamicContext::currentDateTime() const
{
    return m_currentDateTime;
}

xsInteger GenericDynamicContext::contextPosition() const
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "That this function is called makes no sense. A Focus should be used.");
    return 0;
}

Item GenericDynamicContext::contextItem() const
{
    return Item();
}

xsInteger GenericDynamicContext::contextSize()
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "That this function is called makes no sense. A Focus should be used.");
    return 0;
}

void GenericDynamicContext::setFocusIterator(const Item::Iterator::Ptr &)
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "That this function is called makes no sense. A Focus should be used.");
}

Item::Iterator::Ptr GenericDynamicContext::focusIterator() const
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "That this function is called makes no sense. A Focus should be used.");
    return Item::Iterator::Ptr();
}

SequenceReceiver::Ptr GenericDynamicContext::outputReceiver() const
{
    return m_outputReceiver;
}

void GenericDynamicContext::setOutputReceiver(const SequenceReceiver::Ptr &receiver)
{
    m_outputReceiver = receiver;
}

void GenericDynamicContext::setNodeBuilder(const NodeBuilder::Ptr &builder)
{
    m_nodeBuilder = builder;
}

NodeBuilder::Ptr GenericDynamicContext::nodeBuilder(const QUrl &baseURI) const
{
    return m_nodeBuilder->create(baseURI);
}

ResourceLoader::Ptr GenericDynamicContext::resourceLoader() const
{
    return m_resourceLoader;
}

void GenericDynamicContext::setResourceLoader(const ResourceLoader::Ptr &loader)
{
    m_resourceLoader = loader;
}

ExternalVariableLoader::Ptr GenericDynamicContext::externalVariableLoader() const
{
    return m_externalVariableLoader;
}

void GenericDynamicContext::setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader)
{
    m_externalVariableLoader = loader;
}

NamePool::Ptr GenericDynamicContext::namePool() const
{
    return m_namePool;
}

QSourceLocation GenericDynamicContext::locationFor(const SourceLocationReflection *const reflection) const
{
    qDebug() << Q_FUNC_INFO << "count:" << m_locations.count() << "asking for:" << reflection << "has it?" << m_locations.contains(reflection);

    return m_locations.value(reflection->actualReflection());
}

void GenericDynamicContext::addNodeModel(const NodeModel::Ptr &nm)
{
    m_nodeModels.append(nm);
}

QAbstractUriResolver::Ptr GenericDynamicContext::uriResolver() const
{
    return m_uriResolver;
}
// vim: et:ts=4:sw=4:sts=4

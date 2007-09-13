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

#include <QDateTime>

#include "qdaytimeduration_p.h"
#include "qlistiterator_p.h"

#include "qdelegatingdynamiccontext_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

DelegatingDynamicContext::DelegatingDynamicContext(const DynamicContext::Ptr &prevContext)
                                                  : m_prevContext(prevContext)
{
    Q_ASSERT(m_prevContext);
}

ItemCacheCell &DelegatingDynamicContext::itemCacheCell(const VariableSlotID slot)
{
    return m_prevContext->itemCacheCell(slot);
}

ItemSequenceCacheCell::Vector &DelegatingDynamicContext::itemSequenceCacheCells(const VariableSlotID slot)
{
    return m_prevContext->itemSequenceCacheCells(slot);
}

xsInteger DelegatingDynamicContext::contextPosition() const
{
    return m_prevContext->contextPosition();
}

Item DelegatingDynamicContext::contextItem() const
{
    return m_prevContext->contextItem();
}

xsInteger DelegatingDynamicContext::contextSize()
{
    return m_prevContext->contextSize();
}

void DelegatingDynamicContext::setFocusIterator(const Item::Iterator::Ptr &it)
{
    m_prevContext->setFocusIterator(it);
}

Item::Iterator::Ptr DelegatingDynamicContext::positionIterator(const VariableSlotID slot) const
{
    return m_prevContext->positionIterator(slot);
}

void DelegatingDynamicContext::setPositionIterator(const VariableSlotID slot,
                                                   const Item::Iterator::Ptr &newValue)
{
    m_prevContext->setPositionIterator(slot, newValue);
}

void DelegatingDynamicContext::setRangeVariable(const VariableSlotID slotNumber,
                                               const Item &newValue)
{
    m_prevContext->setRangeVariable(slotNumber, newValue);
}

Item::Iterator::Ptr DelegatingDynamicContext::focusIterator() const
{
    return m_prevContext->focusIterator();
}

Item DelegatingDynamicContext::rangeVariable(const VariableSlotID slotNumber) const
{
    return m_prevContext->rangeVariable(slotNumber);
}

void DelegatingDynamicContext::setExpressionVariable(const VariableSlotID slotNumber,
                                                     const Expression::Ptr &newValue)
{
    m_prevContext->setExpressionVariable(slotNumber, newValue);
}

Expression::Ptr DelegatingDynamicContext::expressionVariable(const VariableSlotID slotNumber) const
{
    return m_prevContext->expressionVariable(slotNumber);
}

QAbstractMessageHandler * DelegatingDynamicContext::messageHandler() const
{
    return m_prevContext->messageHandler();
}

PlainSharedPtr<DayTimeDuration> DelegatingDynamicContext::implicitTimezone() const
{
    return m_prevContext->implicitTimezone();
}

QDateTime DelegatingDynamicContext::currentDateTime() const
{
    return m_prevContext->currentDateTime();
}

SequenceReceiver::Ptr DelegatingDynamicContext::outputReceiver() const
{
    return m_prevContext->outputReceiver();
}

NodeBuilder::Ptr DelegatingDynamicContext::nodeBuilder(const QUrl &baseURI) const
{
    return m_prevContext->nodeBuilder(baseURI);
}

ResourceLoader::Ptr DelegatingDynamicContext::resourceLoader() const
{
    return m_prevContext->resourceLoader();
}

ExternalVariableLoader::Ptr DelegatingDynamicContext::externalVariableLoader() const
{
    return m_prevContext->externalVariableLoader();
}

NamePool::Ptr DelegatingDynamicContext::namePool() const
{
    return m_prevContext->namePool();
}

QSourceLocation DelegatingDynamicContext::locationFor(const SourceLocationReflection *const reflection) const
{
    qDebug() << Q_FUNC_INFO << endl;
    return m_prevContext->locationFor(reflection);
}

void DelegatingDynamicContext::addNodeModel(const NodeModel::Ptr &nm)
{
    m_prevContext->addNodeModel(nm);
}

QAbstractUriResolver::Ptr DelegatingDynamicContext::uriResolver() const
{
    return m_prevContext->uriResolver();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE

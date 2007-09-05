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

/**
 * @file
 * @short This file is included by StackContextBase.h.
 * If you need includes in this file, put them in StackContextBase.h, outside of the namespace.
 */

template<typename TSuperClass>
StackContextBase<TSuperClass>::StackContextBase() : m_rangeVariables(10),
                                                    m_expressionVariables(10),
                                                    m_positionIterators(5),
                                                    m_itemCacheCells(5),
                                                    m_itemSequenceCacheCells(5)
{
    qDebug() << Q_FUNC_INFO;
    /* The m_* containers are initialized with default sizes. Estimated guesses on usage patterns. */
}

template<typename TSuperClass>
StackContextBase<TSuperClass>::StackContextBase(const DynamicContext::Ptr &prevContext)
                                                : TSuperClass(prevContext),
                                                  m_rangeVariables(10),
                                                  m_expressionVariables(10),
                                                  m_positionIterators(5),
                                                  m_itemCacheCells(5),
                                                  m_itemSequenceCacheCells(5)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(prevContext);
}

template<typename TSuperClass>
ItemCacheCell &StackContextBase<TSuperClass>::itemCacheCell(const VariableSlotID slot)
{
    qDebug() << Q_FUNC_INFO << "slot:" << slot
                            << "max:" << qMax(slot + 1, m_itemCacheCells.size())
                            << "curr size:" << m_itemCacheCells.size();

    if(slot >= m_itemCacheCells.size())
        m_itemCacheCells.resize(qMax(slot + 1, m_itemCacheCells.size()));

    qDebug() << "AFTER:" << m_itemCacheCells.size();

    return m_itemCacheCells[slot];
}

template<typename TSuperClass>
ItemSequenceCacheCell::Vector &StackContextBase<TSuperClass>::itemSequenceCacheCells(const VariableSlotID slot)
{
    qDebug() << Q_FUNC_INFO << "slot:" << slot
                            << "max:" << qMax(slot + 1, m_itemSequenceCacheCells.size())
                            << "curr size:" << m_itemSequenceCacheCells.size();

    if(slot >= m_itemSequenceCacheCells.size())
        m_itemSequenceCacheCells.resize(qMax(slot + 1, m_itemSequenceCacheCells.size()));

    qDebug() << "AFTER:" << m_itemSequenceCacheCells.size();

    return m_itemSequenceCacheCells;
}

template<typename TSuperClass>
Item StackContextBase<TSuperClass>::rangeVariable(const VariableSlotID slot) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot;
    Q_ASSERT(slot < m_rangeVariables.size());
    Q_ASSERT(m_rangeVariables.at(slot));
    return m_rangeVariables.at(slot);
}

template<typename TSuperClass>
Expression::Ptr StackContextBase<TSuperClass>::expressionVariable(const VariableSlotID slot) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot;
    Q_ASSERT(slot < m_expressionVariables.size());
    return m_expressionVariables.at(slot);
}

template<typename TSuperClass>
Item::Iterator::Ptr StackContextBase<TSuperClass>::positionIterator(const VariableSlotID slot) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot;
    Q_ASSERT(slot < m_positionIterators.size());
    return m_positionIterators.at(slot);
}

template<typename TSuperClass>
template<typename VectorType, typename UnitType>
inline
void StackContextBase<TSuperClass>::setSlotVariable(const VariableSlotID slot,
                                                    const UnitType &newValue,
                                                    VectorType &container) const
{
    if(slot < container.size())
        container.replace(slot, newValue);
    else
    {
        container.resize(slot + 1);
        container.replace(slot, newValue);
    }
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setRangeVariable(const VariableSlotID slot,
                                                     const Item &newValue)
{
    setSlotVariable(slot, newValue, m_rangeVariables);
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setExpressionVariable(const VariableSlotID slot,
                                                          const Expression::Ptr &newValue)
{
    setSlotVariable(slot, newValue, m_expressionVariables);
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setPositionIterator(const VariableSlotID slot,
                                                        const Item::Iterator::Ptr &newValue)
{
    setSlotVariable(slot, newValue, m_positionIterators);
}

// vim: et:ts=4:sw=4:sts=4

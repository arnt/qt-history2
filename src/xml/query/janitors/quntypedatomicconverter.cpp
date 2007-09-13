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

#include "qitem_p.h"
#include "qcommonsequencetypes_p.h"
#include "qdebug_p.h"
#include "qgenericsequencetype_p.h"
#include "qitemmappingiterator_p.h"
#include "qlistiterator_p.h"

#include "quntypedatomicconverter_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

UntypedAtomicConverter::UntypedAtomicConverter(const Expression::Ptr &operand,
                                               const ItemType::Ptr &reqType)
                                              : SingleContainer(operand),
                                                m_reqType(reqType)
{
    Q_ASSERT(reqType);
    qDebug() << Q_FUNC_INFO;
}

Item UntypedAtomicConverter::mapToItem(const Item &item, const DynamicContext::Ptr &context) const
{
    return cast(item, context);
}

Item::Iterator::Ptr UntypedAtomicConverter::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return makeItemMappingIterator<Item>
                    (UntypedAtomicConverter::Ptr(const_cast<UntypedAtomicConverter *>(this)),
                    m_operand->evaluateSequence(context),
                    context);
}

Item UntypedAtomicConverter::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operand->evaluateSingleton(context));

    if(item)
        return cast(item, context);
    else /* Empty is allowed. UntypedAtomicConverter doesn't care about cardinality. */
        return Item();
}

Expression::Ptr UntypedAtomicConverter::typeCheck(const StaticContext::Ptr &context,
                                                  const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(SingleContainer::typeCheck(context, reqType));

    /* Let the CastingPlatform look up its AtomicCaster. */
    prepareCasting(context, m_operand->staticType()->itemType());

    return me;
}

SequenceType::List UntypedAtomicConverter::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreAtomicTypes);
    return result;
}

SequenceType::Ptr UntypedAtomicConverter::staticType() const
{
    return makeGenericSequenceType(m_reqType,
                                   m_operand->staticType()->cardinality());
}

ExpressionVisitorResult::Ptr UntypedAtomicConverter::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

const SourceLocationReflection *UntypedAtomicConverter::actualReflection() const
{
    return m_operand.get();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE

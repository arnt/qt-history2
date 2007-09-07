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

#ifndef Patternist_OrderBy_H
#define Patternist_OrderBy_H

#include "qatomiccomparator_p.h"
#include "qsinglecontainer_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class OrderBy : public SingleContainer
    {
    public:
        enum Stability
        {
            StableOrder,
            UnstableOrder
        };

        class OrderSpec
        {
        public:
            typedef QVector<OrderSpec> Vector;

            enum Direction
            {
                Ascending,
                Descending
            };

            /**
             * @short Default constructor, which is needed by QVector.
             */
            inline OrderSpec()
            {
            }

            inline OrderSpec(const Direction dir,
                             const StaticContext::OrderingEmptySequence orderingEmpty) : direction(dir),
                                                                                         orderingEmptySequence(orderingEmpty)
            {
            }

            AtomicComparator::Ptr comparator;
            Direction direction : 1;
            StaticContext::OrderingEmptySequence orderingEmptySequence : 1;
        };

        OrderBy(const Stability stability,
                const OrderSpec::Vector &orderSpecs, const Expression::Ptr &operand);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual SequenceType::List expectedOperandTypes() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

    private:
        const Stability m_stability;
        const OrderSpec::Vector m_orderSpecs;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

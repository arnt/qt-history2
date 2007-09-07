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

#ifndef Patternist_ReturnOrderBy_H
#define Patternist_ReturnOrderBy_H

#include "qorderby_p.h"
#include "qunlimitedcontainer_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements XQuery 1.0's <tt>order by</tt> expression.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ReturnOrderBy : public UnlimitedContainer
    {
    public:
        ReturnOrderBy(const OrderBy::Stability stability,
                      const OrderBy::OrderSpec::Vector &oSpecs, const Expression::List &operands);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;
        virtual SequenceType::List expectedOperandTypes() const;
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual ID id() const;

        inline OrderBy::OrderSpec::Vector orderSpecs() const
        {
            return m_orderSpecs;
        }

        inline OrderBy::Stability stability() const
        {
            return m_stability;
        }

    private:
        /**
         * This variable is unfortunately only used at compile time. However,
         * it's tricky to get rid of it due to how QueryTransformParser would
         * have to be adapted.
         */
        const OrderBy::Stability m_stability;
        OrderBy::OrderSpec::Vector m_orderSpecs;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

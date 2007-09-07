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

#ifndef Patternist_IfThenClause_H
#define Patternist_IfThenClause_H

#include "qtriplecontainer_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements XPath 2.0's conditional expression <tt>if([expr]) then [expr] else [expr]</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-conditionals">XML Path Language (XPath) 2.0,
     * 3.8 Conditional Expressions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class IfThenClause : public TripleContainer
    {
    public:
        IfThenClause(const Expression::Ptr &test,
                     const Expression::Ptr &then,
                     const Expression::Ptr &el);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        virtual SequenceType::Ptr staticType() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        /**
         * @returns IDIfThenClause
         */
        virtual ID id() const;
        virtual QList<PlainSharedPtr<OptimizationPass> > optimizationPasses() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

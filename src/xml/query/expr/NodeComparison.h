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
***************************************************************************
*/

#ifndef Patternist_NodeComparison_H
#define Patternist_NodeComparison_H

#include "PairContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the node comparison operators <tt>\>\></tt>, <tt>\<\<</tt>, and @c is.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-node-comparisons">XML Path Language
     * (XPath) 2.0, 3.5.3 Node Comparisons</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class NodeComparison : public PairContainer
    {
    public:
        NodeComparison(const Expression::Ptr &operand1,
                       const Node::DocumentOrder op,
                       const Expression::Ptr &operand2);

        virtual bool evaluateEBV(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;

        virtual Node::DocumentOrder operatorID() const;
        /**
         * If any operator is the empty sequence, the NodeComparison rewrites
         * into that, since the empty sequence is always the result in that case.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * @returns either CommonSequenceTypes::ZeroOrOneBoolean or
         * CommonSequenceTypes::ExactlyOneBoolean depending on the static
         * cardinality of its operands.
         */
        virtual SequenceType::Ptr staticType() const;

        /**
         * Determines the string representation for a node comparison operator.
         *
         * @returns
         * - "<<" if @p op is Precedes
         * - ">>" if @p op is Follows
         * - "is" if @p op is Is
         */
        static QString displayName(const Node::DocumentOrder op);

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
    private:
        const Node::DocumentOrder m_op;

    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

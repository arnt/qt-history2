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

#ifndef Patternist_ExpressionDispatch_H
#define Patternist_ExpressionDispatch_H

#include <QSharedData>

#include "qplainsharedptr_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    class AndExpression;
    class ArgumentReference;
    class ArithmeticExpression;
    class Atomizer;
    class AttributeConstructor;
    class AttributeNameValidator;
    class CardinalityVerifier;
    class CastAs;
    class CastableAs;
    class CollationChecker;
    class CombineNodes;
    class CommentConstructor;
    class ContextItem;
    class DocumentConstructor;
    class DynamicContextStore;
    class ElementConstructor;
    class EmptySequence;
    class ExpressionSequence;
    class EvaluationCache;
    class ExpressionVariableReference;
    class ExternalVariableReference;
    class FirstItemPredicate;
    class ForClause;
    class FunctionCall;
    class GeneralComparison;
    class GenericPredicate;
    class IfThenClause;
    class InstanceOf;
    class ItemVerifier;
    class LetClause;
    class Literal;
    class LiteralSequence;
    class NamespaceConstructor;
    class NCNameConstructor;
    class NodeComparison;
    class OrderBy;
    class OrExpression;
    class ParentNodeAxis;
    class Path;
    class PositionalVariableReference;
    class ProcessingInstructionConstructor;
    class QNameConstructor;
    class QuantifiedExpression;
    class RangeExpression;
    class ReturnOrderBy;
    class RangeVariableReference;
    class SimpleContentConstructor;
    class AxisStep;
    class TextNodeConstructor;
    class TreatAs;
    class TruthPredicate;
    class UntypedAtomicConverter;
    class UserFunctionCallsite;
    class ValidationError;
    class ValueComparison;

    /**
     * @todo Documentation's missing
     *
     * @defgroup Patternist_expr_dispatch Expression Dispatching
     */

    /**
     * @ingroup Patternist_expr_dispatch
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ExpressionVisitorResult : public QSharedData
    {
    public:
        typedef PlainSharedPtr<ExpressionVisitorResult> Ptr;
        ExpressionVisitorResult() {}
        virtual ~ExpressionVisitorResult() {}
    };

    /**
     * @ingroup Patternist_expr_dispatch
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ExpressionVisitor : public QSharedData
    {
    public:
        typedef PlainSharedPtr<ExpressionVisitor> Ptr;
        virtual ~ExpressionVisitor() {}

        virtual ExpressionVisitorResult::Ptr visit(const AndExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ArithmeticExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ArgumentReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const Atomizer *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const AttributeConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const AttributeNameValidator *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CardinalityVerifier *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CastAs *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CastableAs *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CollationChecker *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CombineNodes *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CommentConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ContextItem *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const DocumentConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const DynamicContextStore *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ElementConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const EmptySequence *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const EvaluationCache *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ExpressionSequence *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ExpressionVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ExternalVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const FirstItemPredicate *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ForClause *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const FunctionCall *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const GeneralComparison *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const GenericPredicate *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const IfThenClause *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const AxisStep *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const InstanceOf *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ItemVerifier *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const LetClause *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const Literal *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const LiteralSequence *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const NamespaceConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const NCNameConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const NodeComparison *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const OrderBy *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const OrExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ParentNodeAxis *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const Path *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const PositionalVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ProcessingInstructionConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const QNameConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const QuantifiedExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const RangeExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const RangeVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ReturnOrderBy *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const SimpleContentConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const TextNodeConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const TreatAs *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const TruthPredicate *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const UntypedAtomicConverter *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const UserFunctionCallsite *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ValidationError *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ValueComparison *) const = 0;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

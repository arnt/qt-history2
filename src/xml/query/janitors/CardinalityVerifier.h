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

#ifndef Patternist_CardinalityVerifier_H
#define Patternist_CardinalityVerifier_H

#include "SingleContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Verifies that the sequence an Expression evaluates to conforms to a Cardinality.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#cardinality-funcs">XQuery 1.0 and
     * XPath 2.0 Functions and Operators, 15.2 Functions That Test the Cardinality of Sequences</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class CardinalityVerifier : public SingleContainer
    {
    public:
        CardinalityVerifier(const Expression::Ptr &operand,
                            const Cardinality &card,
                            const ReportContext::ErrorCode code);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        /**
         * If the static cardinality of the operand is within the required cardinality,
         * the operand is returned as is, since results will always be valid and hence
         * is not a CardinalityVerifier necessary.
         *
         * This is an optimization, and hence could be done at the compression() stage, but
         * the earlier the better.
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * A utility function for determining whether the static type of an Expression matches
         * a cardinality. More specifically, this function performs the cardinality verification
         * part of the Function Conversion Rules.
         *
         * @todo Mention the rewrite and when exactly an error is issued via @p context
         */
        static Expression::Ptr
        verifyCardinality(const Expression::Ptr &operand,
                          const Cardinality &card,
                          const ReportContext::Ptr &context,
                          const ReportContext::ErrorCode code = ReportContext::XPTY0004);

        virtual const SourceLocationReflection *actualReflection() const;

    private:
        /**
         * Centralizes a message string in order to increase consistency and
         * reduce work for translators.
         */
        static inline QString wrongCardinality(const Cardinality &req,
                                               const Cardinality &got = Cardinality::empty());

        const Cardinality m_reqCard;
        const bool m_allowsMany;
        const ReportContext::ErrorCode m_errorCode;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

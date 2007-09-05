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

#ifndef Patternist_EvaluationCache_H
#define Patternist_EvaluationCache_H

#include "SingleContainer.h"
#include "VariableDeclaration.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Evaluates to the same result as its operand, but ensures the
     * operand is evaluated once even if this Expression is evaluated several
     * times during a query run.
     *
     * EvaluationCache does this in a pipelined way, by delivering items from
     * its cache, which is stored in the DynamicContext. If the cache has less
     * items than what the caller requests, EvaluationCache continues to
     * deliver but this time from the source, which it also populates into the
     * cache.
     *
     * EvaluationCache is used as an optimization in order to avoid running expensive code
     * paths multiple times, but also is sometimes a necessity: for instance,
     * when objects must be unique, such as potentially in the case of node identity.
     *
     * EvaluationCache is in particular used for variables, whose sole purpose
     * is to store it once(at least conceptually) and then use it in multiple
     * places.
     *
     * In some cases an EvaluationCache isn't necessary. For instance, when a
     * variable is only referenced once. In those cases EvaluationCache removes
     * itself as an optimization; implemented in EvaluationCache::compress().
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class EvaluationCache : public SingleContainer
    {
    public:
        EvaluationCache(const Expression::Ptr &operand,
                        const VariableDeclaration::Ptr &varDecl,
                        const VariableSlotID slot);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        virtual SequenceType::Ptr staticType() const;

        /**
         * The first operand must be exactly one @c xs:string.
         */
        virtual SequenceType::List expectedOperandTypes() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual Properties properties() const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual const SourceLocationReflection *actualReflection() const;

        inline VariableSlotID slot() const
        {
            return m_slot;
        }

    private:
        const VariableDeclaration::Ptr  m_declaration;
        const VariableSlotID            m_slot;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_CastAs_H
#define Patternist_CastAs_H

#include "qsinglecontainer_p.h"
#include "qcastingplatform_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements XPath 2.0's <tt>cast as</tt> expression.
     *
     * Implements the casting expression, such as <tt>'3' cast as xs:integer</tt>. This class also
     * implements constructor functions, which are created in the ConstructorFunctionsFactory.
     *
     * CastAs uses CastingPlatform for carrying out the actual casting.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#casting">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 7 Casting</a>
     * @see <a href="http://www.w3.org/TR/xpath20/#id-cast">XML Path Language
     * (XPath) 2.0, 3.10.2 Cast</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class CastAs : public SingleContainer,
                   public CastingPlatform<CastAs, true /* issueError */>
    {
    public:

        /**
         * @todo Wrong/old documentation
         *
         * Creates a cast expression for the type @p name via the schema type
         * factory @p factory. This function is used by parser when creating
         * 'cast to' expressions, and the ConstructorFunctionsFactory, when creating
         * constructor functions.
         *
         * @param targetType the type which the the CastAs should cast to
         * @param source the operand to evaluate and then cast from
         */
        CastAs(const Expression::Ptr &source,
               const SequenceType::Ptr &targetType);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;

        /**
         * @returns a SequenceType where the ItemType is this CastAs's
         * target type, as per targetType(), and the Cardinality is inferred from the
         * source operand to reflect whether this CastAs always will evaluate to
         * exactly-one or zero-or-one values.
         */
        virtual SequenceType::Ptr staticType() const;

        /**
         * Overriden in order to check that casting to an abstract type
         * is not attempted.
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        /**
         * If the target type is the same as the source type, it is rewritten
         * to the operand.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        inline ItemType::Ptr targetType() const
        {
            return m_targetType->itemType();
        }

    private:
        /**
         * Performs casting to @c xs:QName. This case is special, and is always done at compile time.
         */
        Expression::Ptr castToQName(const StaticContext::Ptr &context) const;

        const SequenceType::Ptr m_targetType;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

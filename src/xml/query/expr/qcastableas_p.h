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

#ifndef Patternist_CastableAs_H
#define Patternist_CastableAs_H

#include "qsinglecontainer_p.h"
#include "qcastingplatform_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements XPath 2.0's <tt>castable as</tt> expression.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-castable">XML Path Language
     * (XPath) 2.0, 3.10.3 Castable</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class CastableAs : public SingleContainer,
                       protected CastingPlatform<CastableAs, false>
    {
    public:
        CastableAs(const Expression::Ptr &operand,
                             const SequenceType::Ptr &targetType);

        virtual bool evaluateEBV(const DynamicContext::Ptr &) const;

        /**
         * Overriden to const fold to @c true when the target type
         * is a type which casting to always succeeds. This is
         * the type identical to the target type, <tt>xs:string</tt>,
         * and <tt>xs:untypedAtomic</tt>.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        inline ItemType::Ptr targetType() const
        {
            return m_targetType->itemType();
        }

    private:
        const SequenceType::Ptr m_targetType;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

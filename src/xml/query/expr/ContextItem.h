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

#ifndef Patternist_ContextItem_H
#define Patternist_ContextItem_H

#include "EmptyContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the context item, the dot: <tt>.</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-context-item-expression">XML Path Language
     * (XPath) 2.0, 3.1.4 Context Item Expression</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ContextItem : public EmptyContainer
    {
    public:
        /**
         * @p expr is possibly used for error reporting. If this context item has been
         * created implicitly, such as for the expression <tt>fn:string()</tt>, @p expr
         * should be passed a valid pointer to the Expression that this context
         * item is generated for.
         */
        inline ContextItem(const Expression::Ptr &expr = Expression::Ptr()) : m_expr(expr)
        {
        }

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;

        /**
         * @returns always DisableElimination and RequiresContextItem
         */
        virtual Expression::Properties properties() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * Overriden to store a pointer to StaticContext::contextItemType().
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * Overriden to store a pointer to StaticContext::contextItemType().
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        /**
         * @returns always IDContextItem
         */
        virtual ID id() const;

        /**
         * @returns always BuiltinTypes::item;
         */
        virtual ItemType::Ptr expectedContextItemType() const;

        virtual const SourceLocationReflection *actualReflection() const;
    private:
        ItemType::Ptr           m_itemType;
        const Expression::Ptr   m_expr;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

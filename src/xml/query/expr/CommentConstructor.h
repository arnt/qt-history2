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

#ifndef Patternist_CommentConstructor_H
#define Patternist_CommentConstructor_H

#include "SingleContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Constructs a comment node. This covers both computed and directly constructed
     * text nodes.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#id-constructors">XQuery
     * 1.0: An XML Query Language, 3.7 Constructors</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class CommentConstructor : public SingleContainer
    {
    public:
        CommentConstructor(const Expression::Ptr &operand);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        virtual SequenceType::Ptr staticType() const;

        /**
         * The first operand must be exactly one @c xs:string.
         */
        virtual SequenceType::List expectedOperandTypes() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        virtual Properties properties() const;

    private:
        inline QString evaluateContent(const DynamicContext::Ptr &context) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

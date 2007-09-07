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

#ifndef Patternist_Path_H
#define Patternist_Path_H

#include "qpaircontainer_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the path expression, containing two steps, such as in <tt>html/body</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#id-path-expressions">XQuery 1.0: An
     * XML Query Language, 3.2 Path Expressions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class Path : public PairContainer
    {
    public:
        Path(const Expression::Ptr &operand1,
             const Expression::Ptr &operand2);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        virtual SequenceType::List expectedOperandTypes() const;

        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        inline Item::Iterator::Ptr mapToSequence(const Item &item,
                                                 const DynamicContext::Ptr &context) const;

        /**
         * @returns the static type of the last step where the cardinality is multiplied with
         * the cardinality of the first step's cardinality.
         */
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        virtual Properties properties() const;

        /**
         * @returns the item type of the last step's static type.
         */
        virtual ItemType::Ptr newContextItemType() const;

    private:
        typedef PlainSharedPtr<Path> Ptr;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

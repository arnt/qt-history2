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

#ifndef Patternist_LiteralSequence_H
#define Patternist_LiteralSequence_H

#include "EmptyContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Houses a sequence of atomic values, making it available as an Expression.
     *
     * This is not only literals that can be created via the XQuery syntax(strings and numbers), but
     * all other atomic values, such as <tt>xs:date</tt> or <tt>xs:time</tt>. It is not guaranteed
     * that consequtive atomic values are represented in a LiteralSequence.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#id-literals">XQuery 1.0: An XML Query Language,
     * 3.1.1 Literals</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class LiteralSequence : public EmptyContainer
    {
    public:
        /**
         * Creates a LiteralSequence that represents @p item.
         *
         * @param list the list of item. No entry may be @c null. The list
         * must at least be two entries large.
         */
        LiteralSequence(const Item::List &list);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * @returns IDExpressionSequence
         */
        virtual ID id() const;

        virtual Properties properties() const;

    private:
        const Item::List m_list;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @todo docs
     *
     * Patternist's family of iterators in one of the most central parts of Patternist's API,
     * and are responsible for carrying, and typically also creating, data.
     *
     * An iterator, which always is an Iterator sub-class, is similar to a Java-style
     * iterator. What signifies Patternist's iterators is that they almost always contains
     * business logic(which is the cause to their efficiency).
     *
     * An example which illustrates this principle is the RangeIterator. When the
     * RangeExpression is told to create a sequence of integers between 1 and 1000, it
     * doesn't enter a loop that allocates 1000 Integer instances, but instead return
     * an RangeIterator that incrementally creates the numbers when asked to do so via its
     * RangeIterator::next() function. If it turns out that the expression that
     * has the range expression as operand only needs three items from it, that is what gets
     * created, not 1000.
     *
     * All iterators operates by that principle, perhaps suitably labeled as "pull-based",
     * "lazy loaded" or "serialized". Central for the XPath language is that it filters and
     * selects data, and the iterators supports this well by letting the demand of the filter
     * expressions(the callees) decide how "much" source that gets computed. In this way the
     * evaluation of an expression tree can lead to a chain of pipelined iterators, where the
     * first asks the second for data and then performs its specific operations, the second
     * subsequently asks the third, and so forth.
     *
     * However, the iterators are not limited to be used for representing sequences of items
     * in the XPath Data Model. The Iterator is parameterized on one argument, meaning any
     * type of "units" can be iterated, be it Item or any other. One use of this is in the
     * ExpressionSequence(which implements the comma operator) where it creates Iterator instances
     * over Expression instances -- its operands. The parameterization is often used in
     * combination with the MappingIterator and the MappingCallback.
     *
     * @defgroup Patternist_iterators Iterators
     * @author Frans Englich <fenglich@trolltech.com>
     */
}

// vim: et:ts=4:sw=4:sts=4

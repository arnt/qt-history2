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

#ifndef Patternist_OperandsIterator_H
#define Patternist_OperandsIterator_H

#include <QPair>
#include <QStack>

#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A helper class that iterates a tree of Expression instances. It
     * is not a sub-class of Iterator.
     *
     * The Iterator delivers all Expression instances that are children at any
     * depth of the Expression passed in the constructor. The Expression passed
     * in the constructor is excluded.
     *
     * The order is delivered in a defined way, from left to right and depth
     * first.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class OperandsIterator
    {
        typedef QPair<Expression::List, int> Level;
    public:
        inline OperandsIterator(const Expression::Ptr &start)
        {
            Q_ASSERT(start);
            m_exprs.push(qMakePair(start->operands(), -1));
        }

        /**
         * @short Returns the current Expression and advances the iterator.
         *
         * If the end has been reached, a default constructed pointer is
         * returned.
         *
         * We intentionally return by reference.
         */
        inline Expression::Ptr next()
        {
            if(m_exprs.isEmpty())
                return Expression::Ptr();

            Level &lvl = m_exprs.top();
            ++lvl.second;

            if(lvl.second == lvl.first.size())
            {
                /* Resume iteration above us. */
                m_exprs.pop();

                if(m_exprs.isEmpty())
                    return Expression::Ptr();

                while(true)
                {
                    Level &previous = m_exprs.top();
                    ++previous.second;

                    if(previous.second < previous.first.count())
                    {
                        const Expression::Ptr &op = previous.first.at(previous.second);
                        m_exprs.push(qMakePair(op->operands(), -1));
                        return previous.first.at(previous.second);
                    }
                    else
                    {
                        // We have already reached the end of this level.
                        m_exprs.pop();
                        if(m_exprs.isEmpty())
                            return Expression::Ptr();
                    }
                }
            }
            else
            {
                const Expression::Ptr &op = lvl.first.at(lvl.second);
                m_exprs.push(qMakePair(op->operands(), -1));
                return op;
            }
        }

    private:
        Q_DISABLE_COPY(OperandsIterator)

        QStack<Level> m_exprs;
    };
}

// vim: et:ts=4:sw=4:sts=4
QT_END_HEADER 

#endif

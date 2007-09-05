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

#ifndef Patternist_EmptyContainer_H
#define Patternist_EmptyContainer_H

#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for expressions that has no operands.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class EmptyContainer : public Expression
    {
    public:
        /**
         * @returns always an empty list.
         */
        virtual Expression::List operands() const;

        /**
         * Does nothing, since sub-classes has no operands. Calling
         * it makes hence no sense, and it also results in an assert crash.
         */
        virtual void setOperands(const Expression::List &);

    protected:
        /**
         * @returns always @c true
         */
        virtual bool compressOperands(const StaticContext::Ptr &context);

        /**
         * @returns always an empty list since it has no operands.
         */
        virtual SequenceType::List expectedOperandTypes() const;

    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

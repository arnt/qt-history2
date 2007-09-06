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

#ifndef Patternist_AtomicMathematician_H
#define Patternist_AtomicMathematician_H

#include <QFlags>

#include "DynamicContext.h"
#include "Item.h"
#include "AtomicTypeDispatch.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for classes that performs arithmetic operations between atomic values.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicMathematician : public AtomicTypeVisitorResult
    {
    public:
        virtual ~AtomicMathematician();

        typedef PlainSharedPtr<AtomicMathematician> Ptr;

        enum Operator
        {
            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-divide">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.4 op:numeric-divide</a>
             */
            Div         = 1,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-integer-divide">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.5 op:numeric-integer-divide</a>
             */
            IDiv        = 2,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-subtract">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.2 op:numeric-subtract</a>
             */
            Substract   = 4,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-mod">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.6 op:numeric-mod</a>
             */
            Mod         = 8,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-multiply">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.3 op:numeric-multiply</a>
             */
            Multiply    = 16,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-add">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.1 op:numeric-add</a>
             */
            Add         = 32
        };

        typedef QFlags<Operator> Operators;

        virtual Item calculate(const Item &operand1,
                                    const Operator op,
                                    const Item &operand2,
                                    const PlainSharedPtr<DynamicContext> &context) const = 0;

        static QString displayName(const AtomicMathematician::Operator op);

    };
    Q_DECLARE_OPERATORS_FOR_FLAGS(AtomicMathematician::Operators)
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

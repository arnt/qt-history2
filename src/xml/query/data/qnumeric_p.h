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

#ifndef Patternist_Numeric_H
#define Patternist_Numeric_H

#include "Item.h"
#include "Primitives.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for all numeric values.
     *
     * @section creation Creating Instances
     *
     * @todo
     * - Depending on what type of val
     * - Numeric::createFromString
     * - Various classes has ::Zero(), ::PosINF(), ::NaN(), NegINF()
     * - Never use constructor, use createFromNative, or createFromString.
     *
     * @see <a href="http://www.w3.org/TR/xquery-operators/#numeric-functions">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 6 Functions and Operators on Numerics</a>
     * @see <a href="http://www.w3.org/TR/xquery-operators/#func-overloading">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 1.2 Function Overloading</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo discuss data hierarchy the non existatnt number data type
     */
    class Numeric : public AtomicValue
    {
    public:

        typedef PlainSharedPtr<Numeric> Ptr;

        /**
         * Creates a Numeric sub-class that is appropriate for @p number.
         *
         * @note usages of e/E is not handled; Double::fromLexical should
         * be used in that case. There is no function similar to fromLexical that also
         * takes double values into account(because that distinction is done in the scanner).
         *
         * Currently used in the parser to create appropriate expressions.
         */
        static AtomicValue::Ptr fromLexical(const QString &number);

        /**
         * @returns the particular number's value as a native representation of
         * the type xs:double. This can be considered that the value is cast to
         * xs:double.
         */
        virtual xsDouble toDouble() const = 0;

        /**
         * @returns the particular number's value as a native representation of
         * the type xs:integer. This can be considered that the value is cast to
         * xs:integer.
         */
        virtual xsInteger toInteger() const = 0;

        /**
         * @returns the particular number's value as a native representation of
         * the type xs:float. This can be considered that the value is cast to
         * xs:float.
         */
        virtual xsFloat toFloat() const = 0;

        /**
         * @returns the particular number's value as a native representation of
         * the type xs:decimal. This can be considered that the value is cast to
         * xs:decimal.
         */
        virtual xsFloat toDecimal() const = 0;

        /**
         * Performs the algorithm specified for the function fn:round on this Numeric,
         * and whose result is returned.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-round">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 6.4.4 fn:round</a>
         */
        virtual Numeric::Ptr round() const = 0;

        /**
         * Performs rounding as defined for the fn:round-half-to-even  on this Numeric,
         * and whose result is returned.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-round-half-to-even">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 6.4.5 fn:round-half-to-even</a>
         */
        virtual Numeric::Ptr roundHalfToEven(const xsInteger scale) const = 0;

        /**
         * Performs the algorithm specified for the function fn:floor on this Numeric,
         * and whose result is returned.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-floor">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 6.4.3 fn:floor</a>
         */
        virtual Numeric::Ptr floor() const = 0;

        /**
         * Performs the algorithm specified for the function fn:ceiling on this Numeric,
         * and whose result is returned.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-ceiling">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 6.4.2 fn:ceiling</a>
         */
        virtual Numeric::Ptr ceiling() const = 0;

        /**
         * Performs the algorithm specified for the function fn:abs on this Numeric,
         * and whose result is returned.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-ceiling">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 6.4.1 fn:abs</a>
         */
        virtual Numeric::Ptr abs() const = 0;

        /**
         * Determines whether this Numeric is not-a-number, @c NaN. For numeric types
         * that cannot represent @c NaN, this function should return @c false.
         *
         * @returns @c true if this Numeric is @c NaN
         */
        virtual bool isNaN() const = 0;

        /**
         * Determines whether this Numeric is an infinite number. Signedness
         * is irrelevant, -INF as well as INF is considered infinity.
         *
         * For numeric types that cannot represent infinity, such as xs:integer
         * , this function should return @c false.
         *
         * @returns @c true if this Numeric is an infinite number
         */
        virtual bool isInf() const = 0;

    protected:
        /**
         * MS Windows does not have C99's nearbyint() function(see the man
         * page), so we do our best at coping with it.
         */
        static xsDouble nearByInt(const xsDouble val);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

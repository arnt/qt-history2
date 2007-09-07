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

#ifndef Patternist_Decimal_H
#define Patternist_Decimal_H

#include "Numeric.h"

/**
 * Defined in QtCore's qlocale.cpp.
 */
extern char *qdtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **resultp);

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the value instance of the @c xs:decimal type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo Documentation is missing/incomplete
     */
    class Decimal : public Numeric
    {
    public:

        static Decimal::Ptr fromValue(const xsDecimal num);

        /**
         * Creates a Decimal from the lexical representation of @c xs:decimal stored in
         * @p strNumeric.
         *
         * A possible optimization is to create an Integer if the string ends
         * with ".0". But this is not conformant. For example, the user writes N.0
         * which according to the specification is an xs:decimal, but where the
         * expression is, is an xs:integer is required. That would pass with
         * such an optimization.
         */
        static AtomicValue::Ptr fromLexical(const QString &strNumeric);

        /**
         * Gets the Effective %Boolean Value of this number.
         *
         * @returns @c false if the number is 0 or @c NaN, otherwise @c true.
         */
        bool evaluateEBV(const PlainSharedPtr<DynamicContext> &) const;

        virtual QString stringValue() const;

        /**
         * @returns always BuiltinTypes::xsDecimal
         */
        virtual ItemType::Ptr type() const;

        virtual xsDouble toDouble() const;
        virtual xsInteger toInteger() const;
        virtual xsFloat toFloat() const;
        virtual xsDecimal toDecimal() const;

        virtual Numeric::Ptr round() const;
        virtual Numeric::Ptr roundHalfToEven(const xsInteger scale) const;
        virtual Numeric::Ptr floor() const;
        virtual Numeric::Ptr ceiling() const;
        virtual Numeric::Ptr abs() const;

        /**
         * @returns always @c false, xs:decimal doesn't have
         * not-a-number in its value space.
         */
        virtual bool isNaN() const;

        /**
         * @returns always @c false, xs:decimal doesn't have
         * infinity in its value space.
         */
        virtual bool isInf() const;

        /**
         * Converts @p value into a canonical string representation for @c xs:decimal. This
         * function is used internally by various classes. Users probably wants to call
         * stringValue() which in turn calls this function.
         */
        static QString toString(const xsDecimal value);

    protected:

        Decimal(const xsDecimal num);

    private:
        const xsDecimal m_value;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

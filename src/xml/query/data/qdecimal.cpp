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

#include <math.h>

#include "qabstractfloat_p.h"
#include "qatomictype_p.h"
#include "qbuiltintypes_p.h"
#include "qvalidationerror_p.h"

#include "qdecimal_p.h"

using namespace Patternist;

Decimal::Decimal(const xsDecimal num) : m_value(num)
{
}

Decimal::Ptr Decimal::fromValue(const xsDecimal num)
{
    return Decimal::Ptr(new Decimal(num));
}

AtomicValue::Ptr Decimal::fromLexical(const QString &strNumeric)
{
    /* QString::toDouble() handles the whitespace facet. */

    /* Block these out, as QString::toDouble() supports them. */
    if(strNumeric == QLatin1String("-Inf")  ||
       strNumeric == QLatin1String("Inf")   ||
       strNumeric == QLatin1String("+Inf")  ||
       strNumeric == QLatin1String("+INF")  ||
       strNumeric == QLatin1String("INF")   ||
       strNumeric == QLatin1String("NaN")   ||
       strNumeric == QLatin1String("nan"))
    {
        return ValidationError::createError();
    }

    bool conversionOk = false;
    const xsDecimal num = strNumeric.toDouble(&conversionOk);

    if(conversionOk)
        return AtomicValue::Ptr(new Decimal(num));
    else
        return ValidationError::createError();
}

bool Decimal::evaluateEBV(const PlainSharedPtr<DynamicContext> &) const
{
    return !Double::isEqual(m_value, 0.0);
}

QString Decimal::stringValue() const
{
    return toString(m_value);
}

QString Decimal::toString(const xsDecimal value)
{
    /*
     * If SV is in the value space of xs:integer, that is, if there are no
     * significant digits after the decimal point, then the value is converted
     * from an xs:decimal to an xs:integer and the resulting xs:integer is
     * converted to an xs:string using the rule above.
     */
    if(Double::isEqual(::floor(value), value))
    {
        /* The static_cast is identical to Integer::toInteger(). */
        return QString::number(static_cast<xsInteger>(value));
    }
    else
    {
        int sign;
        int decimalPoint;
        char *result = 0;
        static_cast<void>(qdtoa(value, 0, 0, &decimalPoint, &sign, 0, &result));
        /* If the copy constructor is used instead of QString::operator=(),
         * it doesn't compile. I have no idea why. */
        const QString qret(QString::fromLatin1(result));
        delete result;

        QString valueAsString;

        if(sign)
            valueAsString += QLatin1Char('-');

        if(0 < decimalPoint)
        {
            valueAsString += qret.left(decimalPoint);
            valueAsString += QLatin1Char('.');
            if (qret.size() <= decimalPoint)
                valueAsString += QLatin1Char('0');
            else
                valueAsString += qret.mid(decimalPoint);
        }
        else
        {
            valueAsString += QLatin1Char('0');
            valueAsString += QLatin1Char('.');

            for(int d = decimalPoint; d < 0; d++)
                valueAsString += QLatin1Char('0');

            valueAsString += qret;
        }

        return valueAsString;
    }
}

ItemType::Ptr Decimal::type() const
{
    return BuiltinTypes::xsDecimal;
}

xsDouble Decimal::toDouble() const
{
    return static_cast<xsDouble>(m_value);
}

xsInteger Decimal::toInteger() const
{
    return static_cast<xsInteger>(m_value);
}

xsFloat Decimal::toFloat() const
{
    return static_cast<xsFloat>(m_value);
}

xsDecimal Decimal::toDecimal() const
{
    return m_value;
}

Numeric::Ptr Decimal::round() const
{
    return Numeric::Ptr(new Decimal(nearByInt(m_value)));
}

Numeric::Ptr Decimal::roundHalfToEven(const xsInteger /*scale*/) const
{
    return Numeric::Ptr();
}

Numeric::Ptr Decimal::floor() const
{
    return Numeric::Ptr(new Decimal(static_cast<xsDecimal>(::floor(m_value))));
}

Numeric::Ptr Decimal::ceiling() const
{
    return Numeric::Ptr(new Decimal(static_cast<xsDecimal>(ceil(m_value))));
}

Numeric::Ptr Decimal::abs() const
{
    return Numeric::Ptr(new Decimal(static_cast<xsDecimal>(fabs(m_value))));
}

bool Decimal::isNaN() const
{
    return false;
}

bool Decimal::isInf() const
{
    return false;
}

// vim: et:ts=4:sw=4:sts=4

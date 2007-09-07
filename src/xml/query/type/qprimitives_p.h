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

#ifndef Patternist_Primitives_H
#define Patternist_Primitives_H

#include <QtGlobal>

class QString;
/**
 * @file
 * @short Contains enumerators and typedefs applying
 * for Patternist on a global scale, as well as central documentation.
 */

/**
 * @short Contains Patternist, an XPath 2.0, XQuery 1.0 and XSL-T 2.0 implementation.
 *
 * @author Frans Englich <fenglich@trolltech.com>
 */
QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @defgroup Patternist_cppWXSTypes C++ Primitives for W3C XML Schema Number Types
     *
     * The implementations of W3C XML Schema's(WXS) number types, more specifically
     * their value spaces, must in the end be represented by primitive C++ types.
     * In addition, there is an extensive range of functions and classes that in
     * different ways deals with data that will end up as instances of the WXS
     * types. For this reason, a set of typedefs for these primitives exists, that
     * are used throughout the API. This ensures consistency, reduces the amount
     * of conversions, and potentially precision loss in conversions.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:double. Taking this type, xsDouble,
     * as parameter, is the most efficient way to integrate with xs:double.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef qreal xsDouble;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:float. Taking this type, xsFloat,
     * as parameter, is the most efficient way to integrate with xs:float.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef xsDouble xsFloat;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:decimal. Taking this type, xsDecimal,
     * as parameter, is the most efficient way to integrate with xs:decimal.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef xsDouble xsDecimal;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:integer. Taking this type, xsInteger,
     * as parameter, is the most efficient way to integrate with xs:integer.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef qint64 xsInteger;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:integer. Taking this type, xsInteger,
     * as parameter, is the most efficient way to integrate with xs:integer.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef qint32 VariableSlotID;

    typedef qint32  DayCountProperty;
    typedef qint32  HourCountProperty;
    typedef qint32  MinuteCountProperty;
    typedef qint32  MonthCountProperty;
    typedef qint32  SecondCountProperty;
    typedef qint64  MSecondCountProperty;
    typedef qint32  SecondProperty;
    typedef qint32  YearProperty;
    typedef qint8   DayProperty;
    typedef qint8   HourProperty;
    typedef qint8   MinuteProperty;
    typedef qint8   MonthProperty;

    /**
     * Milliseconds. 1 equals 0.001 SecondProperty.
     */
    typedef qint16  MSecondProperty;

    /**
     * The hour property of a zone offset. For example, -13 in the
     * zone offset "-13:08".
     */
    typedef qint8   ZOHourProperty;

    /**
     * The minute property of a zone offset. For example, -08 in the
     * zone offset "-13:08".
     */
    typedef qint8   ZOMinuteProperty;

    /**
     * The full zone offset in minutes.
     */
    typedef qint32  ZOTotal;

    /**
     * @short Identical to Qt::escape() but since Qt::escape() is in QtGui, using
     * it creates a dependency on that library. This function does not.
     *
     * @see Qt::escape()
     */
    QString escape(const QString &input);
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

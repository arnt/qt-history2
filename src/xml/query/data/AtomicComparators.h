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

#ifndef Patternist_AtomicComparators_H
#define Patternist_AtomicComparators_H

#include "AtomicComparator.h"
#include "Primitives.h"

/**
 * @file
 * @short Contains all the classes implementing comparisons between atomic values.
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Performs case @em sensitive string comparison
     * between @c xs:anyUri, @c xs:string, and @c xs:untypedAtomic.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringComparator : public AtomicComparator
    {
    public:
        /**
         * Compares two strings, and returns the appropriate AtomicComparator::ComparisonResult enum. This
         * is a bit simplified version of string comparison as defined in the XPath specifications,
         * since this does not take any string collations into account(which an implementation neither
         * is required to do).
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#string-compare">XQuery 1.0 and XPath
         * 2.0 Functions and Operators, 7.3 ValueComparison::Equality and Comparison of Strings</a>
         */
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;

        /**
         * Compares two strings, and returns @c true if they are considered equal as per
         * an ordinary string comparison. In other words, this is an implementation with
         * the Unicode code point collation.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#string-compare">XQuery 1.0 and XPath
         * 2.0 Functions and Operators, 7.3 ValueComparison::Equality and Comparison of Strings</a>
         */
        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Performs case @em insensitive string comparison
     * between @c xs:anyUri, @c xs:string, and @c xs:untypedAtomic.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CaseInsensitiveStringComparator : public AtomicComparator
    {
    public:
        /**
         * Converts both string values to upper case and afterwards compare them.
         */
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;

        /**
         * Converts both string values case insensitively.
         */
        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares @c xs:base64Binary and @c xs:hexBinary values.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BinaryDataComparator : public AtomicComparator
    {
    public:
        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares @c xs:boolean values.
     *
     * This is done via the object's Boolean::evaluteEBV() function.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BooleanComparator : public AtomicComparator
    {
    public:
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;

        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares @c xs:double values.
     *
     * @todo Add docs about numeric promotion
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractFloatComparator : public AtomicComparator
    {
    public:
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;

        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares @c xs:decimal values.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DecimalComparator : public AtomicComparator
    {
    public:
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;

        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares @c xs:integer values.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class IntegerComparator : public AtomicComparator
    {
    public:
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;

        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares @c xs:QName values.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class QNameComparator : public AtomicComparator
    {
    public:
        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares sub-classes of AbstractDateTime.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeComparator : public AtomicComparator
    {
    public:
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;
        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };

    /**
     * @short Compares sub-classes of AbstractDuration.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDurationComparator : public AtomicComparator
    {
    public:
        virtual ComparisonResult compare(const Item &op1,
                                         const AtomicComparator::Operator op,
                                         const Item &op2) const;
        virtual bool equals(const Item &op1,
                            const Item &op2) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

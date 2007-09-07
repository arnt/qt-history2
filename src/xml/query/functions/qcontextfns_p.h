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

#ifndef Patternist_ContextFNs_H
#define Patternist_ContextFNs_H

#include "FunctionCall.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#context">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 16 Context Functions</a>.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the function <tt>fn:position()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-position">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.1 fn:position</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class PositionFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:last()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-last">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.2 fn:last</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class LastFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:implicit-timezone()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-implicit-timezone">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.6 fn:implicit-timezone</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ImplicitTimezoneFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:current-dateTime()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-current-dateTime">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.3 fn:current-dateTime</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CurrentDateTimeFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:current-date()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-current-date">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.4 fn:current-date</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CurrentDateFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:current-time()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-current-time">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.5 fn:current-date</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CurrentTimeFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:default-collation()</tt>.
     *
     * This is done by rewriting to StaticContext::defaultCollation() at the typeCheck() stage.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-default-collation">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.7 fn:default-collation</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DefaultCollationFN : public FunctionCall
    {
    public:
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
    };

    /**
     * @short Implements the function <tt>fn:static-base-uri()</tt>.
     *
     * This is done by rewriting to StaticContext::baseURI() at the typeCheck() stage.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-static-base-uri">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 16.8 fn:static-base-uri</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StaticBaseURIFN : public FunctionCall
    {
    public:
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_BooleanFNs_H
#define Patternist_BooleanFNs_H

#include "FunctionCall.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#boolean-functions">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 9 Functions and Operators on Boolean Values</a>.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the function <tt>fn:true()</tt>.
     *
     * The implementation always rewrites itself to a boolean value at compile time.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class TrueFN : public FunctionCall
    {
    public:
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:false()</tt>.
     *
     * The implementation always rewrites itself to a boolean value at compile time.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FalseFN : public FunctionCall
    {
    public:
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:not()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NotFN : public FunctionCall
    {
    public:
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual QList<PlainSharedPtr<OptimizationPass> > optimizationPasses() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

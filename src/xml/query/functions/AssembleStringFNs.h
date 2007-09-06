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

#ifndef Patternist_AssembleStringFNs_H
#define Patternist_AssembleStringFNs_H

#include "FunctionCall.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#func-assemble-disassemble-string">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 7.2 Functions to Assemble and Disassemble Strings</a>.
 * <a href="http://www.azillionmonkeys.com/qed/unicode.html">Quick Guide to understanding
 * Unicode Data Transfer Formats</a>
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the function <tt>fn:codepoints-to-string()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CodepointsToStringFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:string-to-codepoints()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StringToCodepointsFN : public FunctionCall
    {
    public:
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

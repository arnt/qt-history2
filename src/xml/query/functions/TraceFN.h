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

#ifndef Patternist_TraceFN_H
#define Patternist_TraceFN_H

#include "FunctionCall.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the function <tt>fn:trace()</tt>.
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class TraceFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

        /**
         * Formally speaking, the type inference is:
         *
@verbatim
statEnv |-  (FN-URI,"trace")(Type) : prime(Type) · quantifier(Type)
@endverbatim
         *
         * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_fn_reverse">XQuery 1.0
         * and XPath 2.0 Formal Semantics, 7.2.12 The fn:reverse function</a>, for
         * an example of where the type inference is used
         * @returns the static type of the function's first argument.
         */
        virtual SequenceType::Ptr staticType() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_ErrorFN_H
#define Patternist_ErrorFN_H

#include "FunctionCall.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the function <tt>fn:error()</tt>.
     *
     * <tt>fn:error()</tt> is a bit special in that its first argument varies between
     * the different signatures. This is implemented by changing the function
     * signature if the amount of arguments is one.
     *
     * <tt>fn:error()</tt> has as return type the peculiar "none" type, which is handled by NoneType.
     *
     * @ingroup Patternist_functions
     * @see CommonSequenceTypes::none
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-error">XQuery 1.0 and
     * XPath 2.0 Functions and Operators, 3 The Error Function</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ErrorFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual FunctionSignature::Ptr signature() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

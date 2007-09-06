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

#ifndef Patternist_XPath20CoreFunctions_H
#define Patternist_XPath20CoreFunctions_H

#include "AbstractFunctionFactory.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Handles the functions defines in XQuery 1.0 and XPath 2.0
     * Function and Operators, except those also available in XPath 1.0.
     *
     * All XPath 2.0 functions is the union of the functions available in XPath20CoreFunctions
     * and XPath10CoreFunctions. One could therefore say that the name XPath20CoreFunctions is a
     * bit misleading.
     *
     * @note XPath20CoreFunctions inherits from XPath10CoreFunctions only for implementation
     * reasons, it does not supply the functions in the XPath10CoreFunctions factory.
     *
     * @see XPath10CoreFunctions
     * @see <a href ="http://www.w3.org/TR/xpath-functions/">XQuery 1.0
     * and XPath 2.0 Functions and Operators</a>
     * @see <a href="http://www.w3.org/TR/xpath.html#corelib">XML Path Language (XPath)
     * Version 1.0, 4 Core Function Library</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_functions
     */
    class XPath20CoreFunctions : public AbstractFunctionFactory
    {
    protected:
        virtual Expression::Ptr retrieveExpression(const QName name,
                                                   const Expression::List &args,
                                                   const FunctionSignature::Ptr &sign) const;

        virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np,
                                                                 const QName name);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

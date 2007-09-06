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

#ifndef Patternist_XPath10CoreFunctions_H
#define Patternist_XPath10CoreFunctions_H

#include "AbstractFunctionFactory.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Supplies the functions available in XPath 1.0.
     *
     * @ingroup Patternist_functions
     * @see <a href="http://www.w3.org/TR/xpath.html#corelib">XML Path Language (XPath)
     * Version 1.0, 4 Core Function Library</a>
     * @see XPath20CoreFunctions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class XPath10CoreFunctions : public AbstractFunctionFactory
    {
    protected:
        /**
         * This function is responsible for creating the actual Expression, corresponding
         * to @p localName and the function signature @p sign. It is called by
         * createFunctionCall(), once it have been determined the function actually
         * exists and have the correct arity.
         */
        virtual Expression::Ptr retrieveExpression(const QName name,
                                                   const Expression::List &args,
                                                   const FunctionSignature::Ptr &sign) const;
        virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QName name);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_XSLT10CoreFunctions_H
#define Patternist_XSLT10CoreFunctions_H

#include "qabstractfunctionfactory_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Handles the functions defines in XSL-T 1.0.
     *
     * @see XPath10CoreFunctions
     * @see XPath20CoreFunctions
     * @see <a href="http://www.w3.org/TR/xslt20/#add-func">XSL Transformations
     * (XSLT) Version 2.0, 16 Additional Functions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_functions
     */
    class XSLT10CoreFunctions : public AbstractFunctionFactory
    {
    protected:
        virtual Expression::Ptr retrieveExpression(const QName ln,
                                                   const Expression::List &args,
                                                   const FunctionSignature::Ptr &sign) const;

        virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QName name);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

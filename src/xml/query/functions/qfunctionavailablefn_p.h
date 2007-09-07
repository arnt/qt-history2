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

#ifndef Patternist_FunctionAvailableFN_H
#define Patternist_FunctionAvailableFN_H

#include "qfunctioncall_p.h"
#include "qfunctionfactory_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements XSL-T 2.0's XPath function <tt>fn:function-available()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xslt20/#function-function-available">XSL Transformations
     * (XSLT) Version 2.0, 18.1.1 Testing Availability of Functions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_functions
     */
    class FunctionAvailableFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        /**
         * Reimplemented to store data from the @p context which is needed at runtime.
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

    private:
        NamespaceResolver::Ptr m_resolver;
        FunctionFactory::Ptr m_functionFactory;
        QString m_defFuncNS;
    };
}
QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

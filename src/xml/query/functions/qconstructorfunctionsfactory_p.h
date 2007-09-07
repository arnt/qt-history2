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

#ifndef Patternist_ConstructorFunctionsFactory_H
#define Patternist_ConstructorFunctionsFactory_H

#include "AbstractFunctionFactory.h"
#include "SchemaTypeFactory.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short A function factory that handles the builtin constructor functions, such
     * as <tt>xs:time()</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-constructor-functions">XML Path
     * Language (XPath) 2.0, 3.10.4 Constructor Functions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_functions
     */
    class ConstructorFunctionsFactory : public AbstractFunctionFactory
    {
    public:
        ConstructorFunctionsFactory(const NamePool::Ptr &np, const SchemaTypeFactory::Ptr &);

        virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QName name);

    protected:
         virtual Expression::Ptr retrieveExpression(const QName name,
                                                    const Expression::List &args,
                                                    const FunctionSignature::Ptr &sign) const;

    private:
        const SchemaTypeFactory::Ptr m_typeFactory;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

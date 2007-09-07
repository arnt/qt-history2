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


#ifndef Patternist_FunctionFactoryCollection_H
#define Patternist_FunctionFactoryCollection_H

#include "qfunctionfactory_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * A FunctionFactoryCollection instance is a FunctionFactory in its own right,
     * but looks in its contained collection of factories for requested functions.
     *
     * @note the order of adding function libraries is significant.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FunctionFactoryCollection: public FunctionFactory,
                                     public FunctionFactory::List
    {
    public:

        typedef PlainSharedPtr<FunctionFactoryCollection> Ptr;

        /**
         * Creates a function call node.
         */
        virtual Expression::Ptr createFunctionCall(const QName,
                                                   const Expression::List &arguments,
                                                   const StaticContext::Ptr &context,
                                                   const SourceLocationReflection *const r);
        virtual bool isAvailable(const NamePool::Ptr &np, const QName name, const xsInteger arity);

        virtual FunctionSignature::Hash functionSignatures() const;

        virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QName name);

        /**
         * @return a FunctionFactory containing all core functions and constructor
         * functions required for XPath 2.0, plus Patternist's
         * implementation defined functions. The functions specified for XQuery 1.0
         * are the same as for XPath 2.0 so this FunctionFactory work well for XQuery
         * as well.
         */
        static FunctionFactory::Ptr xpath20Factory(const NamePool::Ptr &np);

        /**
         * @return a FunctionFactory containing all core functions required for XPath 1.0
         * plus Patternist's implementation defined functions.
         */
        static FunctionFactory::Ptr xpath10Factory();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

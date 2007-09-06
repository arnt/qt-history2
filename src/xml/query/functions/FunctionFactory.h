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


#ifndef Patternist_FunctionFactory_H
#define Patternist_FunctionFactory_H

#include <QHash>
#include <QSharedData>

#include "Expression.h"
#include "FunctionSignature.h"
#include "Primitives.h"
#include "QName.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short An entry point for looking up and creating FunctionCall instances.
     *
     * @ingroup Patternist_functions
     * @see <a href ="http://www.w3.org/TR/xpath-functions/">XQuery 1.0
     * and XPath 2.0 Functions and Operators</a>
     * @see <a href="http://www.w3.org/TR/xpath20/#dt-function-signature">XML Path
     * Language (XPath) 2.0, Definition: Function signatures</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FunctionFactory : public QSharedData
    {
    public:

        typedef PlainSharedPtr<FunctionFactory> Ptr;
        typedef QList<FunctionFactory::Ptr> List;

        virtual ~FunctionFactory();

        /**
         * Creates a function call implementation.
         *
         * A FunctionFactory represents a set of functions, which it
         * is able to instantiate and to serve FunctionSignatures for. Conventionally,
         * a FunctionFactory per namespace exists.
         *
         * @note This function should not issue any error unless it is absolutely
         * confident that the error cannot be fixed in another way. For example, in
         * some cases it might be that a function is available in another FunctionFactory
         * and it would therefore be wrong to issue an error signalling that no function
         * by that @p name exists, but leave that to the callee.
         * @param name the name of the function to create. In Clark syntax, this could
         * for example be {http://www.w3.org/2005/04/xpath-functions}lower-case
         * @param arguments the function's operands
         * @param context the usual StaticContext which supplies compile time data
         * and reporting functionality.
         * @returns an instance of Expression which is the function implementation
         * for @p name. Or, a static error was raised.
         */
        virtual Expression::Ptr createFunctionCall(const QName name,
                                                   const Expression::List &arguments,
                                                   const StaticContext::Ptr &context,
                                                   const SourceLocationReflection *const r) = 0;

        /**
         * Determines whether a function with the name @p name and arity @p arity
         * is available. The implementation operates on the result of
         * retrieveFunctionSignature() to determine the result.
         *
         * @param np the NamePool.
         * @param name the name of the function. For example fn:string-join.
         * @param arity the number of arguments the function must have.
         */
        virtual bool isAvailable(const NamePool::Ptr &np,
                                 const QName name,
                                 const xsInteger arity);

        virtual FunctionSignature::Hash functionSignatures() const = 0;

        /**
         * Determines whether this FunctionFactory contains the function signature
         * @p signature.
         *
         * The implementation uses functionSignatures().
         */
        bool hasSignature(const FunctionSignature::Ptr &signature) const;

    protected:
        /**
         * @short This constructor cannot be removed, because it can't be synthesized, for
         * some reason.
         */
        inline FunctionFactory()
        {
        }

        /**
         * This is a convenience function for sub-classes. It retrieves the
         * function signature for function with name @p name.
         *
         * According to the specifications are function signatures identified by their
         * name and arity, but currently is the arity not part of the signature.
         *
         * If no function could be found for the given name, @c null is returned.
         */
        virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QName name) = 0;

    private:
        Q_DISABLE_COPY(FunctionFactory)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

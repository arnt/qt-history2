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

#ifndef Patternist_AbstractFunctionFactory_H
#define Patternist_AbstractFunctionFactory_H

#include "CommonNamespaces.h"
#include "FunctionFactory.h"
#include "FunctionSignature.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Supplies convenience code for the function factories.
     *
     * @ingroup Patternist_functions
     * @see XPath10CoreFunctions
     * @see XPath20CoreFunctions
     * @see XSLT10CoreFunctions
     * @author Vincent Ricard <magic@magicninja.org>
     */
    class AbstractFunctionFactory : public FunctionFactory
    {
    public:
        virtual Expression::Ptr createFunctionCall(const QName name,
                                                   const Expression::List &arguments,
                                                   const StaticContext::Ptr &context,
                                                   const SourceLocationReflection *const r);

        virtual FunctionSignature::Hash functionSignatures() const;

    protected:
        /**
         * This function is responsible for creating the actual Expression, corresponding
         * to @p localName and the function signature @p sign. It is called by
         * createFunctionCall(), once it have been determined the function actually
         * exists and have the correct arity.
         *
         * This function will only be called for names in the @c fn namespace.
         */
        virtual Expression::Ptr retrieveExpression(const QName name,
                                                   const Expression::List &args,
                                                   const FunctionSignature::Ptr &sign) const = 0;

        inline
        FunctionSignature::Ptr addFunction(const QName::LocalNameCode localName,
                                           const FunctionSignature::Arity minArgs,
                                           const FunctionSignature::Arity maxArgs,
                                           const SequenceType::Ptr &returnType,
                                           const Expression::Properties props)
        {
            return addFunction(localName,
                               minArgs,
                               maxArgs,
                               returnType,
                               Expression::IDIgnorableExpression,
                               props);
        }

        FunctionSignature::Ptr addFunction(const QName::LocalNameCode &localName,
                                           const FunctionSignature::Arity minArgs,
                                           const FunctionSignature::Arity maxArgs,
                                           const SequenceType::Ptr &returnType,
                                           const Expression::ID id = Expression::IDIgnorableExpression,
                                           const Expression::Properties props = Expression::Properties())
        {
            const QName name(StandardNamespaces::fn, localName);

            const FunctionSignature::Ptr s(new FunctionSignature(name, minArgs, maxArgs,
                                                                 returnType, props, id));

            m_signatures.insert(name, s);
            return s;
        }

        static inline QName::LocalNameCode argument(const NamePool::Ptr &np, const char *const name)
        {
            return np->allocateLocalName(QLatin1String(name));
        }

        FunctionSignature::Hash m_signatures;

    private:
        /**
         * @short Determines whether @p arity is a valid number of
         * arguments for the function with signature @p sign.
         *
         * If it is not, a static error with error code ReportContext::XPST0017
         * is issued via @p context.
         */
        void verifyArity(const FunctionSignature::Ptr &sign,
                         const StaticContext::Ptr &context,
                         const xsInteger arity,
                         const SourceLocationReflection *const r) const;

    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

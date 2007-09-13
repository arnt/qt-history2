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

#ifndef Patternist_UserFunctionCallsite_H
#define Patternist_UserFunctionCallsite_H

#include "qunlimitedcontainer_p.h"
#include "qfunctionsignature_p.h"
#include "qvariabledeclaration_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Performs a call to a UserFunction.
     *
     * UserFunctionCallsite is the call site to a function that has been
     * declared in the query using <tt>declare function</tt>. That is, it is
     * never used for builtin functions such as <tt>fn:count()</tt>.
     *
     * @see UserFunction
     * @see ArgumentReference
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class UserFunctionCallsite : public UnlimitedContainer
    {
    public:
        typedef PlainSharedPtr<UserFunctionCallsite> Ptr;
        typedef QList<UserFunctionCallsite::Ptr> List;

        UserFunctionCallsite(const QName name,
                             const FunctionSignature::Arity arity);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual Expression::Properties properties() const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * @returns always IDUserFunctionCallsite.
         */
        virtual ID id() const;

        /**
         * If @p slotOffset is -1, it means this function has no arguments.
         */
        void setSource(const Expression::Ptr &body,
                       const FunctionSignature::Ptr &sign,
                       const VariableSlotID slotOffset,
                       const VariableDeclaration::List &varDecls);

        /**
         * @returns @c true, if a function definition with signature @p sign
         * would be valid to call from this callsite, otherwise @c false.
         */
        bool isSignatureValid(const FunctionSignature::Ptr &sign) const;

        FunctionSignature::Arity arity() const;
        Expression::Ptr body() const;
        QName name() const;
        FunctionSignature::Ptr signature() const;

        /**
         * Called in the earliest stages of the compilation process. @p sign can
         * be any function signature for a user declared function. If @p sign
         * matches this UserFunctionCallsite, it means the UserFunction represented
         * by @p sign is recursive and that this UserFunctionCallsite should take
         * appropriate measures.
         *
         * @returns @c true if is recursive, otherwise @c false
         */
        bool configureRecursion(const FunctionSignature::Ptr &sign);

    private:
        /**
         * Creates a new context sets the arguments, and returns it.
         */
        DynamicContext::Ptr bindVariables(const DynamicContext::Ptr &context) const;

        const QName                     m_name;
        const FunctionSignature::Arity  m_arity;
        /**
         * The reason this variable, as well as others, aren't const, is that
         * the binding to the actual function, is resolved after this
         * UserFunctionCallsite has been created.
         */
        VariableSlotID                  m_slotOffset;
        Expression::Ptr                 m_body;
        FunctionSignature::Ptr          m_signature;
        bool                            m_isRecursive;
    };

    /**
     * @short Formats UserFunctionCallsite.
     *
     * @relates UserFunctionCallsite
     */
    static inline QString formatFunction(const UserFunctionCallsite::Ptr &func)
    {
        Q_UNUSED(func);
        // TODO TODO TODO
        // TODO Make UserFunctionCallsite always use a FunctionSignature
        return QLatin1String("<span class='XQuery-function'>")  +
               QString() +
               //escape(func->name()->toString())                 +
               QLatin1String("</span>");
    }
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

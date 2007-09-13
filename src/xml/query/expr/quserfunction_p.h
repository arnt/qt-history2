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

#ifndef Patternist_UserFunction_H
#define Patternist_UserFunction_H

template<typename T> class QList;

#include <QSharedData>

#include "qexpression_p.h"
#include "qfunctionsignature_p.h"
#include "qvariabledeclaration_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short A function created with XQuery's <tt>declare function</tt> declaration.
     *
     * @see UserFunctionCall
     * @see ArgumentReference
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class UserFunction : public QSharedData
    {
    public:
        typedef PlainSharedPtr<UserFunction> Ptr;
        typedef QList<UserFunction::Ptr> List;

        /**
         * If @p slotOffset is -1, it means this function has no arguments.
         */
        UserFunction(const FunctionSignature::Ptr &signature,
                     const Expression::Ptr &body,
                     const VariableSlotID slotOffset,
                     const VariableDeclaration::List &varDecls);

        inline Expression::Ptr body() const;
        inline FunctionSignature::Ptr signature() const;
        inline VariableSlotID expressionSlotOffset() const;
        inline VariableDeclaration::List argumentDeclarations() const;

    private:
        const FunctionSignature::Ptr    m_signature;
        const Expression::Ptr           m_body;
        const VariableSlotID            m_slotOffset;
        const VariableDeclaration::List m_argumentDeclarations;
    };

    inline Expression::Ptr UserFunction::body() const
    {
        return m_body;
    }

    inline FunctionSignature::Ptr UserFunction::signature() const
    {
        return m_signature;
    }

    inline VariableSlotID UserFunction::expressionSlotOffset() const
    {
        return m_slotOffset;
    }

    inline VariableDeclaration::List UserFunction::argumentDeclarations() const
    {
        return m_argumentDeclarations;
    }
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

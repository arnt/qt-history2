/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_FunctionCall_H
#define Patternist_FunctionCall_H

#include "UnlimitedContainer.h"
#include "FunctionSignature.h"
#include "XPathHelper.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for implementations of builtin functions.
     *
     * However, it doesn't handle user declared functions.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FunctionCall : public UnlimitedContainer
    {
    public:
        typedef PlainSharedPtr<FunctionCall> Ptr;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        virtual void setSignature(const FunctionSignature::Ptr &sign);
        virtual FunctionSignature::Ptr signature() const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        virtual Expression::Properties properties() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        virtual ID id() const;

    private:
        FunctionSignature::Ptr m_signature;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

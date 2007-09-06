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

#ifndef Patternist_TripleContainer_H
#define Patternist_TripleContainer_H

#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for expressions that has exactly three operands.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class TripleContainer : public Expression
    {
    public:
        virtual Expression::List operands() const;
        virtual void setOperands(const Expression::List &operands);

        virtual bool compressOperands(const StaticContext::Ptr &);

    protected:
        TripleContainer(const Expression::Ptr &operand1,
                        const Expression::Ptr &operand2,
                        const Expression::Ptr &operand3);

        Expression::Ptr m_operand1;
        Expression::Ptr m_operand2;
        Expression::Ptr m_operand3;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

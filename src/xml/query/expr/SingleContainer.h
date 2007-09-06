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
***************************************************************************
*/

#ifndef Patternist_SingleContainer_H
#define Patternist_SingleContainer_H

#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for expressions that has exactly one operand.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class SingleContainer : public Expression
    {
    public:
        virtual Expression::List operands() const;

        virtual void setOperands(const Expression::List &operands);
        virtual bool compressOperands(const StaticContext::Ptr &);

    protected:
        SingleContainer(const Expression::Ptr &operand);

        Expression::Ptr m_operand;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_SimpleContentConstructor_H
#define Patternist_SimpleContentConstructor_H

#include "SingleContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Constructs simple content found in attributes.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#id-attributes">XQuery 1.0:
     * An XML Query Language, 3.7.1.1 Attributes</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class SimpleContentConstructor : public SingleContainer
    {
    public:
        SimpleContentConstructor(const Expression::Ptr &operand);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

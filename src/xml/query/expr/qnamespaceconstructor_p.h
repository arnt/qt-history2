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

#ifndef Patternist_NamespaceConstructor_H
#define Patternist_NamespaceConstructor_H

#include "qemptycontainer_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Constructs a namespace on an element, and naturally only appears
     * as a child of ElementConstructor.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class NamespaceConstructor : public EmptyContainer
    {
    public:
        NamespaceConstructor(const NamespaceBinding nb);

        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        /**
         * @returns a list containing one CommonSequenceTypes::ExactlyOneString instance.
         */
        virtual SequenceType::List expectedOperandTypes() const;

        /**
         * The static type is exactly one attribute node. It's unclear what
         * affects the static type has, but specifying anything else could lead
         * to complications wrt. node order, XQTY0024. Of course, it's not
         * conceptually correct, since a namespace node isn't an attribute
         * node.
         */
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual Expression::Properties properties() const;

    private:
        const NamespaceBinding m_binding;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

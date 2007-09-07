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

#ifndef Patternist_ExternalVariableReference_H
#define Patternist_ExternalVariableReference_H

#include "EmptyContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A reference to an external variable.
     *
     * ExternalVariableReference does not sub-class VariableReference, because it
     * works differently from how sub-classes of VariableReference do. This class
     * uses DynamicContext::externalVariableLoader() for retrieving its value, while
     * a VariableReference sub-class uses slots in the DynamicContext.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ExternalVariableReference : public EmptyContainer
    {
    public:
        ExternalVariableReference(const QName name,
                                  const SequenceType::Ptr &type);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

        virtual SequenceType::Ptr staticType() const;

        /**
         * @returns always DisableElimination
         */
        virtual Expression::Properties properties() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

    private:
        const QName             m_name;
        const SequenceType::Ptr m_seqType;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

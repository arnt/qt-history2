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

#ifndef Patternist_ItemVerifier_H
#define Patternist_ItemVerifier_H

#include "qsinglecontainer_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{

    /**
     * @short Verifies that the items in a sequence an Expression evaluates
     * is of a certain ItemType.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ItemVerifier : public SingleContainer
    {
    public:

        ItemVerifier(const Expression::Ptr &operand,
                     const ItemType::Ptr &reqType,
                     const ReportContext::ErrorCode errorCode);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        inline Item mapToItem(const Item &, const DynamicContext::Ptr &) const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual const SourceLocationReflection *actualReflection() const;

    private:
        typedef PlainSharedPtr<ItemVerifier> Ptr;
        inline void verifyItem(const Item &item,
                               const DynamicContext::Ptr &context) const;

        const ItemType::Ptr m_reqType;
        const ReportContext::ErrorCode m_errorCode;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_UntypedAtomicConverter_H
#define Patternist_UntypedAtomicConverter_H

#include "Item.h"
#include "SingleContainer.h"
#include "CastingPlatform.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Casts every item in a sequence obtained from
     * evaluating an Expression, to a requested atomic type.
     *
     * The atomic values it casts from are instances of xs:untypedAtomic(hence
     * the name). Typically, the items are from an Atomizer. UntypedAtomicConverter
     * implements the automatic conversion which typically is activated when XPath
     * is handling untyped data.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-function-calls">XML Path
     * Language (XPath) 2.0, 3.1.5 Function Calls, in particular the
     * Function Conversion Rules</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class UntypedAtomicConverter : public SingleContainer,
                                   protected CastingPlatform<UntypedAtomicConverter, true>
    {
    public:
        UntypedAtomicConverter(const Expression::Ptr &operand,
                               const ItemType::Ptr &reqType);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

        virtual SequenceType::Ptr staticType() const;
        virtual SequenceType::List expectedOperandTypes() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * Overriden to call CastingPlatform::typeCheck()
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        inline Item mapToItem(const Item &item,
                                   const DynamicContext::Ptr &context) const;

        inline ItemType::Ptr targetType() const
        {
            return m_reqType;
        }

        virtual const SourceLocationReflection *actualReflection() const;

    private:
        typedef PlainSharedPtr<UntypedAtomicConverter> Ptr;
        const ItemType::Ptr m_reqType;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

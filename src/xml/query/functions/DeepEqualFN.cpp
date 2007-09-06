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

#include "BuiltinTypes.h"
#include "CommonSequenceTypes.h"
#include "CommonValues.h"
#include "ListIterator.h"
#include "Literal.h"
#include "Numeric.h"

#include "DeepEqualFN.h"

using namespace Patternist;

bool DeepEqualFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it1(m_operands.first()->evaluateSequence(context));
    const Item::Iterator::Ptr it2(m_operands.at(1)->evaluateSequence(context));

    while(true)
    {
        const Item item1(it1->next());
        const Item item2(it2->next());

        if(!item1)
        {
            if(item2)
                return false;
            else
                return true;
        }
        else if(!item2)
        {
            if(item1)
                return false;
            else
                return true;
        }
        else if(item1.isNode())
        {
            if(item2.isNode())
            {
                if(item1.asNode().isDeepEqual(item2.asNode()))
                    continue;
                else
                    return false;
            }
            else
                return false;
        }
        else if(item2.isNode())
        {
            /* We know that item1 is not a node due to the check above. */
            return false;
        }
        else if(flexibleCompare(item1, item2, context))
            continue;
        else if(BuiltinTypes::numeric->itemMatches(item1) &&
                item1.as<Numeric>()->isNaN() &&
                item2.as<Numeric>()->isNaN())
        {
            // TODO
            /* Handle the specific NaN circumstances. item2 isn't checked whether it's of
             * type numeric, since the AtomicComparator lookup would have failed if both weren't
             * numeric. */
            continue;
        }
        else
            return false;
    };
}

Expression::Ptr DeepEqualFN::typeCheck(const StaticContext::Ptr &context,
                                       const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(FunctionCall::typeCheck(context, reqType));
    const ItemType::Ptr t1(m_operands.first()->staticType()->itemType());
    const ItemType::Ptr t2(m_operands.at(1)->staticType()->itemType());
    /* TODO This can be much more improved, and the optimizations should be moved
     * to compress(). */

    if(*CommonSequenceTypes::Empty == *t1)
    {
        if(*CommonSequenceTypes::Empty == *t2)
            return wrapLiteral(CommonValues::BooleanTrue, context, this);
        else
            return me;
    }
    else if(*CommonSequenceTypes::Empty == *t2)
    {
        if(*CommonSequenceTypes::Empty == *t1)
            return wrapLiteral(CommonValues::BooleanTrue, context, this);
        else
            return me;
    }
    else if(BuiltinTypes::node->xdtTypeMatches(t1) &&
            BuiltinTypes::node->xdtTypeMatches(t2))
        return me; /* We're comparing nodes. */
    else if(BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t1) &&
            BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t2))
    {
        prepareComparison(fetchComparator(t1, t2, context));
        return me;
    }
    else
    {
        if(BuiltinTypes::node->xdtTypeMatches(t1) && BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t2) ||
           BuiltinTypes::node->xdtTypeMatches(t2) && BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t1))
        {
            /* One operand contains nodes and the other atomic values, or vice versa. They can never
             * be identical. */
        // TODO warn?
            return wrapLiteral(CommonValues::BooleanFalse, context, this);
        }
        else
        {
            // TODO Warn?
            return me;
        }
    }
}

// vim: et:ts=4:sw=4:sts=4

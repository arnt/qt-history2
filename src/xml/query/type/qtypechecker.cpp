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

#include "AtomicType.h"
#include "Atomizer.h"
#include "BuiltinTypes.h"
#include "CardinalityVerifier.h"
#include "CommonSequenceTypes.h"
#include "Debug.h"
#include "ItemVerifier.h"
#include "PatternistLocale.h"
#include "UntypedAtomicConverter.h"

#include "TypeChecker.h"

using namespace Patternist;

QString TypeChecker::wrongType(const NamePool::Ptr &np,
                               const ItemType::Ptr &reqType,
                               const ItemType::Ptr &opType)
{
    return tr("Required type is %1 but %2 was supplied")
              .arg(formatType(np, reqType), formatType(np, opType));
}

Expression::Ptr TypeChecker::applyFunctionConversion(const Expression::Ptr &operand,
                                                     const SequenceType::Ptr &reqType,
                                                     const StaticContext::Ptr &context,
                                                     const ReportContext::ErrorCode code,
                                                     const Options options)
{
    /*
    qDebug() << Q_FUNC_INFO << "reqType:" << reqType->displayName()
             << "opType:" << operand->staticType()->displayName();
             */
    Q_ASSERT_X(!ReportContext::codeToString(code).isEmpty(), Q_FUNC_INFO,
               "This test ensures 'code' exists, otherwise codeToString() would assert.");
    Q_ASSERT(operand);
    Q_ASSERT(reqType);
    Q_ASSERT(context);

    if(context->compatModeEnabled())
    {
        Q_ASSERT_X(false, Q_FUNC_INFO,
                   "Backwards compatible mode is not implemented yet.");
    }

    /* Do it in two steps: verify type, and then cardinality. */
    const Expression::Ptr vop(verifyType(operand, reqType, context, code, options));
    return CardinalityVerifier::verifyCardinality(vop, reqType->cardinality(), context, code);
}

bool TypeChecker::promotionPossible(const ItemType::Ptr &fromType,
                                    const ItemType::Ptr &toType,
                                    const StaticContext::Ptr &context)
{
    /* These types can be promoted to xs:string. xs:untypedAtomic should be
     * cast when interpreting it formally, but implementing it as a promotion
     * gives the same result(and is faster). */
    if(*toType == *BuiltinTypes::xsString &&
       (BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(fromType) ||
       BuiltinTypes::xsAnyURI->xdtTypeMatches(fromType)))
        return true;

    if(*toType == *BuiltinTypes::xsDouble &&
       BuiltinTypes::numeric->xdtTypeMatches(fromType))
    {
        /* Any numeric can be promoted to xs:double. */
        return true;
    }

    /* xs:decimal/xs:integer can be promoted to xs:float. */
    if(*toType == *BuiltinTypes::xsFloat && BuiltinTypes::xsDecimal->xdtTypeMatches(fromType))

    {
        context->warning(tr("Promoting %1 to %2 may cause loss in precision.")
                                   .arg(formatType(context->namePool(), toType))
                                   .arg(formatType(context->namePool(), BuiltinTypes::xsFloat)));
        return true;
    }

    return false;
}

Expression::Ptr TypeChecker::typeCheck(Expression *const op,
                                       const StaticContext::Ptr &context,
                                       const SequenceType::Ptr &reqType)
{
    return Expression::Ptr(op->typeCheck(context, reqType));
}

Expression::Ptr TypeChecker::verifyType(const Expression::Ptr &operand,
                                        const SequenceType::Ptr &reqSeqType,
                                        const StaticContext::Ptr &context,
                                        const ReportContext::ErrorCode code,
                                        const Options options)
{
    const ItemType::Ptr reqType(reqSeqType->itemType());
    const Expression::Properties props(operand->properties());

    /* If operand requires a focus, do the necessary type checking for that. */
    if((props & Expression::RequiresFocus) && (options & CheckFocus))
    {
        const ItemType::Ptr contextType(context->contextItemType());
        if(contextType)
        {
            if((props & Expression::RequiresContextItem) == Expression::RequiresContextItem)
            {
                Q_ASSERT_X(operand->expectedContextItemType(), Q_FUNC_INFO,
                           "When the Expression sets the RequiresContextItem property, it must "
                           "return a type in expectedContextItemType()");

                if(!operand->expectedContextItemType()->xdtTypeMatches(contextType))
                {
                    context->error(wrongType(context->namePool(), operand->expectedContextItemType(), contextType),
                                            ReportContext::XPTY0020, operand.get());
                    return operand;
                }
            }
        }
        else
        {
            context->error(tr("The focus is undefined."), ReportContext::XPDY0002, operand.get());
            return operand;
        }
    }

    ItemType::Ptr operandType(operand->staticType()->itemType());

    /* This returns the operand if the types are identical or if operandType
     * is a subtype of reqType. */
    if(reqType->xdtTypeMatches(operandType))
        return operand;

    /* Since we haven't exited yet, it means that the operandType is a super type
     * of reqType, and that there hence is a path down to it through the
     * type hierachy -- but that doesn't neccessarily mean that a up-cast(down the
     * hierarchy) would succeed. */

    Expression::Ptr result(operand);

    if(reqType->isAtomicType())
    {
        if(!operandType->isAtomicType())
        {
            result = Expression::Ptr(new Atomizer(result));
            /* The atomizer might know more about the type. */
            operandType = result->staticType()->itemType();
        }

        if(reqType->xdtTypeMatches(operandType))
        {
            /* Atomization was sufficient. Either the expected type is xs:anyAtomicType
             * or the type the Atomizer knows it returns, matches the required type. */
            return result;
        }

        if((options & AutomaticallyConvert) && BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(operandType))
        {
            if(*reqType == *BuiltinTypes::numeric)
            {
                result = typeCheck(new UntypedAtomicConverter(result, BuiltinTypes::xsDouble),
                                   context, reqSeqType);
            }
            else
                result = typeCheck(new UntypedAtomicConverter(result, reqType), context, reqSeqType);

            /* The UntypedAtomicConverter might know more about the type, so reload. */
            operandType = result->staticType()->itemType();
        }

        if(reqType->xdtTypeMatches(operandType))
            return result;

        /* Test if promotion will solve it; the xdtTypeMatches didn't
         * do that. */
        if((options & AutomaticallyConvert) && promotionPossible(operandType, reqType, context))
            return result;

        if(operandType->xdtTypeMatches(reqType))
        {
            /* For example, operandType is numeric, and reqType is xs:integer. */
            return Expression::Ptr(new ItemVerifier(result, reqType, code));
        }
        else
        {
            context->error(wrongType(context->namePool(), reqType, operandType), code, operand.get());
            return result;
        }
    }
    else if(reqType->isNodeType())
    {
        qDebug() << "Checking for a node type..";

        ReportContext::ErrorCode myCode;

        if(*reqType == *CommonSequenceTypes::EBV->itemType())
            myCode = ReportContext::FORG0006;
        else
            myCode = code;

        /* empty-sequence() is considered valid because it's ok to do
         * for example nilled( () ). That is, to pass an empty sequence to a
         * function requiring for example node()?. */
        if(*operandType == *CommonSequenceTypes::Empty)
            return result;
        else if(!operandType->xdtTypeMatches(reqType))
        {
            context->error(wrongType(context->namePool(), reqType, operandType), myCode, operand.get());
            return result;
        }

        /* Operand must be an item. Thus, the sequence can contain both
         * nodes and atomic values: we have to verify. */
        return Expression::Ptr(new ItemVerifier(result, reqType, myCode));
    }
    else
    {
        if(!reqType->xdtTypeMatches(operandType))
        {
            context->error(wrongType(context->namePool(), reqType, operandType),
                                    code, operand.get());
            return result;
        }
    }

    /* This line should be reached if required type is
     * EBVType, and the operand is compatible. */
    return result;
}

// vim: et:ts=4:sw=4:sts=4

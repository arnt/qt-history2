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

#include "qoptimizerblocks_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

ExpressionIdentifier::~ExpressionIdentifier()
{
}

ExpressionCreator::~ExpressionCreator()
{
}

OptimizationPass::OptimizationPass(const ExpressionIdentifier::Ptr &startID,
                                   const ExpressionIdentifier::List &opIDs,
                                   const ExpressionMarker &sourceExpr,
                                   const ExpressionCreator::Ptr &resultCtor,
                                   const OperandsMatchMethod mMethod) : startIdentifier(startID),
                                                                        operandIdentifiers(opIDs),
                                                                        sourceExpression(sourceExpr),
                                                                        resultCreator(resultCtor),
                                                                        operandsMatchMethod(mMethod)
{
    Q_ASSERT_X(resultCtor || !sourceExpr.isEmpty(), Q_FUNC_INFO,
               "Either resultCreator or sourceExpression must be set, otherwise there's "
               "nothing to rewrite to.");
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE

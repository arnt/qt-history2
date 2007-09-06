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

#include "OptimizationPasses.h"

#include "BooleanFNs.h"

using namespace Patternist;

bool TrueFN::evaluateEBV(const DynamicContext::Ptr &) const
{
    return true;
}

bool FalseFN::evaluateEBV(const DynamicContext::Ptr &) const
{
    return false;
}

bool NotFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
    /* That little '!' is quite important in this function -- I forgot it ;-) */
    return !m_operands.first()->evaluateEBV(context);
}

OptimizationPass::List NotFN::optimizationPasses() const
{
    return OptimizationPasses::notFN;
}

// vim: et:ts=4:sw=4:sts=4

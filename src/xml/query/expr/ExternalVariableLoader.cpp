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

#include "Boolean.h"
#include "CommonValues.h"
#include "DynamicContext.h"
#include "ListIterator.h"

#include "ExternalVariableLoader.h"

using namespace Patternist;

ExternalVariableLoader::~ExternalVariableLoader()
{
}


SequenceType::Ptr ExternalVariableLoader::announceExternalVariable(const QName name,
                                                                   const SequenceType::Ptr &declaredType)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(!name.isNull());
    Q_ASSERT(declaredType);
    Q_UNUSED(name); /* Needed when compiling in release mode. */
    Q_UNUSED(declaredType); /* Needed when compiling in release mode. */

    return SequenceType::Ptr();
}

Item::Iterator::Ptr ExternalVariableLoader::evaluateSequence(const QName name,
                                                             const DynamicContext::Ptr &context)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(!name.isNull());
    const Item item(evaluateSingleton(name, context));

    if(item)
        return makeSingletonIterator(item);
    else
        return CommonValues::emptyIterator;
}

Item ExternalVariableLoader::evaluateSingleton(const QName name,
                                                    const DynamicContext::Ptr &context)
{
    Q_ASSERT(!name.isNull());
    return Boolean::fromValue(evaluateEBV(name, context));
}

bool ExternalVariableLoader::evaluateEBV(const QName name,
                                         const DynamicContext::Ptr &context)
{
    Q_ASSERT(!name.isNull());
    return Boolean::evaluateEBV(evaluateSequence(name, context), context);
}

// vim: et:ts=4:sw=4:sts=3

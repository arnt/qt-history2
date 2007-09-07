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

#include "qfocus_p.h"
#include "qlistiterator_p.h"
#include "qreceiverdynamiccontext_p.h"
#include "qstackcontextbase_p.h"

#include "qdynamiccontext_p.h"

using namespace Patternist;

DynamicContext::Ptr DynamicContext::createFocus() const
{
    return DynamicContext::Ptr(new Focus(DynamicContext::Ptr(const_cast<DynamicContext *>(this))));
}

DynamicContext::Ptr DynamicContext::createStack() const
{
    return DynamicContext::Ptr(new StackContext(DynamicContext::Ptr(const_cast<DynamicContext *>(this))));
}

DynamicContext::Ptr DynamicContext::createReceiverContext(const SequenceReceiver::Ptr &receiver) const
{
    Q_ASSERT(receiver);
    return DynamicContext::Ptr
        (new ReceiverDynamicContext(DynamicContext::Ptr(const_cast<DynamicContext *>(this)),
                                    receiver));
}

// vim: et:ts=4:sw=4:sts=4

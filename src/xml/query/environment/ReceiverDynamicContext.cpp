/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include "ReceiverDynamicContext.h"

using namespace Patternist;

ReceiverDynamicContext::
ReceiverDynamicContext(const DynamicContext::Ptr &prevContext,
                       const SequenceReceiver::Ptr &receiver) : DelegatingDynamicContext(prevContext),
                                                                m_receiver(receiver)
{
    Q_ASSERT(receiver);
}

SequenceReceiver::Ptr ReceiverDynamicContext::outputReceiver() const
{
    return m_receiver;
}

// vim: et:ts=4:sw=4:sts=4

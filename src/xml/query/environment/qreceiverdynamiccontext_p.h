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

#ifndef Patternist_ReceiverDynamicContext_H
#define Patternist_ReceiverDynamicContext_H

#include "qdelegatingdynamiccontext_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short A DynamicContext that has a specialized SequenceReceiver.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ReceiverDynamicContext : public DelegatingDynamicContext
    {
    public:
        /**
         * Construct a ReceiverDynamicContext and passes @p prevContext to its super class. This
         * constructor is typically used when the super class is DelegatingDynamicContext.
         */
        ReceiverDynamicContext(const DynamicContext::Ptr &prevContext,
                               const SequenceReceiver::Ptr &receiver);

        virtual SequenceReceiver::Ptr outputReceiver() const;

    private:
        const SequenceReceiver::Ptr m_receiver;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

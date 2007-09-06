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

#ifndef Patternist_OutputValidator_H
#define Patternist_OutputValidator_H

#include <QSet>

#include "DynamicContext.h"
#include "SequenceReceiver.h"
#include "SourceLocationReflection.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Receives SequenceReceiver events and validates that they are correct,
     * before sending them on to a second SequenceReceiver.
     *
     * Currently, this is only checking that attributes appear before other
     * nodes.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo Escape data
     */
    class OutputValidator : public SequenceReceiver
                          , public DelegatingSourceLocationReflection
    {
    public:
        OutputValidator(const SequenceReceiver::Ptr &receiver,
                        const DynamicContext::Ptr &context,
                        const SourceLocationReflection *const r);

        virtual void namespaceBinding(const NamespaceBinding nb);

        virtual void characters(const QString &value);
        virtual void comment(const QString &value);

        virtual void startElement(const QName name);

        virtual void endElement();

        virtual void attribute(const QName name,
                               const QString &value);

        virtual void processingInstruction(const QName name,
                                           const QString &value);

        virtual void item(const Item &item);

        virtual void startDocument();
        virtual void endDocument();

    private:
        bool                        m_hasReceivedChildren;
        const SequenceReceiver::Ptr m_receiver;
        const DynamicContext::Ptr   m_context;

        /**
         * Keeps the current received attributes, in order to check uniqueness.
         */
        QSet<QName>                 m_attributes;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

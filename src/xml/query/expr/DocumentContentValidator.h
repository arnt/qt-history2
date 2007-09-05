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

#ifndef Patternist_DocumentContentValidator_H
#define Patternist_DocumentContentValidator_H

#include "DynamicContext.h"
#include "Expression.h"
#include "SequenceReceiver.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Receives SequenceReceiver events and validates that they are correct,
     * before sending them on to a second SequenceReceiver.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     * @todo Escape data
     */
    class DocumentContentValidator : public SequenceReceiver
    {
    public:
        DocumentContentValidator(const SequenceReceiver::Ptr &receiver,
                                 const DynamicContext::Ptr &context,
                                 const Expression::Ptr &expr);

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
        const SequenceReceiver::Ptr m_receiver;
        const DynamicContext::Ptr   m_context;
        const Expression::Ptr       m_expr;
        xsInteger                   m_elementDepth;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

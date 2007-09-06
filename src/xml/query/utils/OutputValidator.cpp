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

#include "Debug.h"
#include "PatternistLocale.h"

#include "OutputValidator.h"

using namespace Patternist;

OutputValidator::OutputValidator(const SequenceReceiver::Ptr &receiver,
                                 const DynamicContext::Ptr &context,
                                 const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
                                                                          , m_hasReceivedChildren(false)
                                                                          , m_receiver(receiver)
                                                                          , m_context(context)
{
    Q_ASSERT(receiver);
    Q_ASSERT(context);
}

void OutputValidator::namespaceBinding(const NamespaceBinding nb)
{
    m_receiver->namespaceBinding(nb);
}

void OutputValidator::startElement(const QName name)
{
    m_hasReceivedChildren = false;
    m_receiver->startElement(name);
    m_attributes.clear();
}

void OutputValidator::endElement()
{
    m_hasReceivedChildren = true;
    m_receiver->endElement();
}

void OutputValidator::attribute(const QName name,
                                const QString &value)
{
    if(m_hasReceivedChildren)
    {
        m_context->error(tr("It's not possible to add attributes after any other kind of node"),
                            ReportContext::XQTY0024, this);
    }
    else
    {
        if(m_attributes.contains(name))
        {
            m_context->error(tr("An attribute by name %1 has already been created.").arg(formatKeyword(m_context->namePool(), name)),
                                ReportContext::XQDY0025, this);
        }
        else
        {
            m_attributes.insert(name);
            m_receiver->attribute(name, value);
        }
    }
}

void OutputValidator::comment(const QString &value)
{
    m_hasReceivedChildren = true;
    m_receiver->comment(value);
}

void OutputValidator::characters(const QString &value)
{
    m_hasReceivedChildren = true;
    m_receiver->characters(value);
}

void OutputValidator::processingInstruction(const QName name,
                                            const QString &value)
{
    m_hasReceivedChildren = true;
    m_receiver->processingInstruction(name, value);
}

void OutputValidator::item(const Item &outputItem)
{
    /* We can't send outputItem directly to m_receiver since its item() function
     * won't dispatch to this OutputValidator, but to itself. We're not sub-classing here,
     * we're delegating. */

    if(outputItem.isNode())
        sendAsNode(outputItem);
    else
    {
        m_hasReceivedChildren = true;
        m_receiver->item(outputItem);
    }
}

void OutputValidator::startDocument()
{
    m_receiver->startDocument();
}

void OutputValidator::endDocument()
{
    m_receiver->endDocument();
}

// vim: et:ts=4:sw=4:sts=4

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

#include "DocumentContentValidator.h"

using namespace Patternist;

DocumentContentValidator::
DocumentContentValidator(const SequenceReceiver::Ptr &receiver,
                         const DynamicContext::Ptr &context,
                         const Expression::Ptr &expr) : m_receiver(receiver)
                                                      , m_context(context)
                                                      , m_expr(expr)
                                                      , m_elementDepth(0)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(receiver);
    Q_ASSERT(m_expr);
    Q_ASSERT(context);
}

void DocumentContentValidator::namespaceBinding(const NamespaceBinding nb)
{
    m_receiver->namespaceBinding(nb);
}

void DocumentContentValidator::startElement(const QName name)
{
    ++m_elementDepth;
    m_receiver->startElement(name);
}

void DocumentContentValidator::endElement()
{
    Q_ASSERT(m_elementDepth > 0);
    --m_elementDepth;
    m_receiver->endElement();
}

void DocumentContentValidator::attribute(const QName name,
                                         const QString &value)
{
    if(m_elementDepth == 0)
    {
        m_context->error(tr("An attribute node can't be a child of a document node. "
                            "An attribute node by name %1 was received.")
                            .arg(formatKeyword(m_context->namePool(), name)),
                         ReportContext::XPTY0004, m_expr.get());
    }
    else
        m_receiver->attribute(name, value);
}

void DocumentContentValidator::comment(const QString &value)
{
    m_receiver->comment(value);
}

void DocumentContentValidator::characters(const QString &value)
{
    m_receiver->characters(value);
}

void DocumentContentValidator::processingInstruction(const QName name,
                                                     const QString &value)
{
    m_receiver->processingInstruction(name, value);
}

void DocumentContentValidator::item(const Item &outputItem)
{
    /* We can't send outputItem directly to m_receiver since its item() function
     * won't dispatch to this DocumentContentValidator, but to itself. We're not sub-classing here,
     * we're delegating. */

    if(outputItem.isNode())
        sendAsNode(outputItem);
    else
        m_receiver->item(outputItem);
}

void DocumentContentValidator::startDocument()
{
    m_receiver->startDocument();
}

void DocumentContentValidator::endDocument()
{
    m_receiver->endDocument();
}

// vim: et:ts=4:sw=4:sts=4

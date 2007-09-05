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

#include "Item.h"
#include "Item.h"
#include "ListIterator.h"

#include "SequenceReceiver.h"

using namespace Patternist;

SequenceReceiver::~SequenceReceiver()
{
}

template<const Node::Axis axis>
void SequenceReceiver::sendFromAxis(const Node node)
{
    Q_ASSERT(node);
    const Item::Iterator::Ptr it(node.iterate(axis));
    Item next(it->next());

    while(next)
    {
        sendAsNode(next);
        next = it->next();
    }
}

void SequenceReceiver::sendAsNode(const Item &outputItem)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(outputItem);
    Q_ASSERT(outputItem.isNode());
    const Node asNode = outputItem.asNode();

    /* The order of the case branches is according to typical
     * frequency of appearance in XML documents. */
    switch(asNode.kind())
    {
        case Node::Attribute:
        {
            attribute(asNode.name(), outputItem.stringValue());
            break;
        }
        case Node::Element:
        {
            qDebug() << "Node::Element";
            startElement(asNode.name());

            /* First the namespaces, then attributes, then the children. */
            asNode.sendNamespaces(Ptr(const_cast<SequenceReceiver *>(this)));
            sendFromAxis<Node::AttributeAxis>(asNode);
            sendFromAxis<Node::Child>(asNode);

            endElement();

            break;
        }
        case Node::Text:
        {
            qDebug() << "Node::Text" << outputItem.stringValue();
            characters(outputItem.stringValue());
            break;
        }
        case Node::ProcessingInstruction:
        {
            processingInstruction(asNode.name(), outputItem.stringValue());
            break;
        }
        case Node::Comment:
        {
            comment(outputItem.stringValue());
            break;
        }
        case Node::Document:
        {
            qDebug() << "Node::Document";
            sendFromAxis<Node::Child>(asNode);
            break;
        }
        case Node::Namespace:
            Q_ASSERT_X(false, Q_FUNC_INFO, "Not implemented");
    }
}

void SequenceReceiver::whitespaceOnly(const QStringRef &value)
{
    Q_ASSERT_X(value.toString().trimmed().isEmpty(), Q_FUNC_INFO,
               "The caller must guarantee only whitespace is passed. Use characters() in other cases.");
    characters(value.toString());
}

// vim: et:ts=4:sw=4:sts=4

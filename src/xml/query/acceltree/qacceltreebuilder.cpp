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

#include "qcompressedwhitespace_p.h"
#include "qlistiterator_p.h"

#include "qacceltreebuilder_p.h"

using namespace Patternist;

AccelTreeBuilder::AccelTreeBuilder(const QUrl &docURI,
                                   const QUrl &baseURI,
                                   const NamePool::Ptr &np) : m_preNumber(0)
                                                            , m_isPreviousAtomic(false)
                                                            , m_hasCharacters(false)
                                                            , m_isCharactersCompressed(false)
                                                            , m_namePool(np)
                                                            , m_document(new AccelTree(docURI, baseURI))
                                                            , m_skippedDocumentNodes(0)
                                                            , m_documentURI(docURI)
{
    Q_ASSERT(m_namePool);

    /* TODO Perhaps we can merge m_ancestors and m_size
     * into one, and store a struct for the two instead? */
    m_ancestors.reserve(DefaultNodeStackSize);
    m_ancestors.push(-1);

    m_size.reserve(DefaultNodeStackSize);
    m_size.push(0);
}

void AccelTreeBuilder::startStructure()
{
    qDebug() << Q_FUNC_INFO;
    if(m_hasCharacters)
    {
        /* We create a node even if m_characters is empty.
         * Remember that `text {""}' creates one text node
         * with string value "". */

        qDebug() << "Appending data";
        m_document->basicData.append(AccelTree::BasicNodeData(currentDepth(),
                                                              currentParent(),
                                                              Node::Text,
                                                              m_isCharactersCompressed ? AccelTree::IsCompressed : 0));
        m_document->data.insert(m_preNumber, m_characters);
        ++m_preNumber;
        ++m_size.top();

        m_characters.clear(); /* We don't want it added twice. */
        m_hasCharacters = false;

        if(m_isCharactersCompressed)
            m_isCharactersCompressed = false;
    }
}

void AccelTreeBuilder::item(const Item &it)
{
    Q_ASSERT(it);

    if(it.isAtomicValue())
    {
        if(m_isPreviousAtomic)
        {
            m_characters.inline_append(QLatin1Char(' '));
            m_characters += it.stringValue();
        }
        else
        {
            m_isPreviousAtomic = true;
            const QString sv(it.stringValue());

            if(!sv.isEmpty())
            {
                m_characters += sv;
                m_hasCharacters = true;
            }
        }
    }
    else
        sendAsNode(it);
}

void AccelTreeBuilder::startElement(const QName name)
{
    qDebug() << Q_FUNC_INFO;
    startStructure();

    qDebug() << "Appending node";
    m_document->basicData.append(AccelTree::BasicNodeData(currentDepth(), currentParent(), Node::Element, -1, name));
    m_ancestors.push(m_preNumber);

    ++m_size.top();
    m_size.push(0);

    ++m_preNumber;
    m_isPreviousAtomic = false;
}

void AccelTreeBuilder::endElement()
{
    qDebug() << Q_FUNC_INFO;
    startStructure();
    const AccelTree::PreNumber index = m_ancestors.pop();
    AccelTree::BasicNodeData &data = m_document->basicData[index];

    /* Sub trees needs to be included in upper trees, so we add the count of this element
     * to our parent. */
    m_size[m_size.count() - 2] += m_size.top();

    data.setSize(m_size.pop());
    m_isPreviousAtomic = false;
}

void AccelTreeBuilder::attribute(const QName name, const QString &value)
{
    m_document->basicData.append(AccelTree::BasicNodeData(currentDepth(), currentParent(), Node::Attribute, 0, name));
    m_document->data.insert(m_preNumber, *m_attributeCompress.insert(value));
    ++m_preNumber;
    ++m_size.top();
    m_isPreviousAtomic = false;
}

void AccelTreeBuilder::characters(const QString &ch)
{
    qDebug() << Q_FUNC_INFO;

    /* If a text node constructor appears by itself, a node needs to
     * be created. Therefore, we set m_hasCharacters
     * if we're the only node.
     * However, if the text node appears as a child of a document or element
     * node it is discarded if it's empty.
     */
    if(m_hasCharacters && m_isCharactersCompressed)
    {
        m_characters = CompressedWhitespace::decompress(m_characters);
        qDebug() << "Decompressed into:" << m_characters;
        m_isCharactersCompressed = false;
    }

    m_characters += ch;
    qDebug() << "Added:" << ch << "now has:" << m_characters;

    m_isPreviousAtomic = false;
    m_hasCharacters = !m_characters.isEmpty() || m_preNumber == 0;
}

void AccelTreeBuilder::whitespaceOnly(const QStringRef &ch)
{
    qDebug() << Q_FUNC_INFO << ch.toString();
    Q_ASSERT(!ch.isEmpty());
    Q_ASSERT(ch.toString().trimmed().isEmpty());

    /* This gets problematic due to how QXmlStreamReader works(which
     * is the only one we get whitespaceOnly() events from). Namely, text intermingled
     * with CDATA gets reported as individual Characters events, and
     * QXmlStreamReader::isWhitespace() can return differently for each of those. However,
     * it will occur very rarely, so this workaround of 1) mistakenly compressing 2) decompressing 3)
     * appending, will happen infrequently.
     */
    if(m_hasCharacters)
    {
        if(m_isCharactersCompressed)
        {
            m_characters = CompressedWhitespace::decompress(m_characters);
            m_isCharactersCompressed = false;
        }

        m_characters.append(ch.toString());
    }
    else
    {
        qDebug() << "m_characters is compressed";
        /* We haven't received a text node previously. */
        m_characters = CompressedWhitespace::compress(ch);
        m_isCharactersCompressed = true;
        m_isPreviousAtomic = false;
        m_hasCharacters = true;
    }
}

void AccelTreeBuilder::processingInstruction(const QName target,
                                             const QString &data)
{
    startStructure();
    m_document->data.insert(m_preNumber, data);

    m_document->basicData.append(AccelTree::BasicNodeData(currentDepth(),
                                                          currentParent(),
                                                          Node::ProcessingInstruction,
                                                          0,
                                                          target));
    ++m_preNumber;
    ++m_size.top();
    m_isPreviousAtomic = false;
}

void AccelTreeBuilder::comment(const QString &content)
{
    startStructure();
    m_document->basicData.append(AccelTree::BasicNodeData(currentDepth(), currentParent(), Node::Comment, 0));
    m_document->data.insert(m_preNumber, content);
    ++m_preNumber;
    ++m_size.top();
}

void AccelTreeBuilder::namespaceBinding(const NamespaceBinding nb)
{
    Q_ASSERT_X(m_document->kind(m_preNumber - 1) == Node::Element, Q_FUNC_INFO,
               "We're only supposed to receive namespaceBinding events after an element start event.");

    m_document->namespaces[m_preNumber - 1].append(nb);
}

void AccelTreeBuilder::startDocument()
{
    /* If we have already received nodes, we can't add a document node. */
    if(m_preNumber == 0)
    {
        m_size.push(0);
        m_document->basicData.append(AccelTree::BasicNodeData(0, -1, Node::Document, -1));
        m_ancestors.push(m_preNumber);
        ++m_preNumber;
    }
    else
        ++m_skippedDocumentNodes;

    m_isPreviousAtomic = false;
}

void AccelTreeBuilder::endDocument()
{
    if(m_skippedDocumentNodes == 0)
    {
        /* Create text nodes, if we've received any. We do this only if we're the
         * top node because if we're getting this event as being a child of an element,
         * text nodes or atomic values can appear after us, and which must get
         * merged with the previous text.
         *
         * We call startStructure() before we pop the ancestor, such that the text node becomes
         * a child of this document node. */
        startStructure();

        m_document->basicData.first().setSize(m_size.pop());
        m_ancestors.pop();
    }
    else
        --m_skippedDocumentNodes;

    m_isPreviousAtomic = false;
    //m_document->printStats(m_namePool);
}

NodeModel::Ptr AccelTreeBuilder::builtDocument()
{
    /* Create a text node, if we have received text in some way. */
    startStructure();

    return m_document;
}

NodeBuilder::Ptr AccelTreeBuilder::create(const QUrl &baseURI) const
{
    Q_UNUSED(baseURI);
    return NodeBuilder::Ptr(new AccelTreeBuilder(QUrl(), baseURI, m_namePool));
}

// vim: et:ts=4:sw=4:sts=4

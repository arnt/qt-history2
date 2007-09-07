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

#include <QFile>
#include <QXmlStreamReader>

#include "AccelTreeBuilder.h"
#include "CommonSequenceTypes.h"
#include "ListIterator.h"

#include "AccelTreeResourceLoader.h"

using namespace Patternist;

static inline uint qHash(const QUrl &uri)
{
        return qHash(uri.toString());
}

AccelTreeResourceLoader::AccelTreeResourceLoader(const NamePool::Ptr &np) : m_namePool(np)
{
    Q_ASSERT(m_namePool);
}

bool AccelTreeResourceLoader::retrieveDocument(const QUrl &uri)
{
    const AccelTreeBuilder::Ptr builder(new AccelTreeBuilder(uri, uri, m_namePool));

    QFile file(uri.toLocalFile());

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning("AccelTreeResourceLoader couldn't open file %s. It probably doesn't exist.",
                 qPrintable(uri.toString()));
        return false;
    }

    const bool success = streamToReceiver(&file, builder, m_namePool);

    if(success)
        m_loadedDocuments.insert(uri, builder->builtDocument());
    else
        qWarning("Couldn't parse %s.", qPrintable(uri.toString()));

    return success;
}

QString AccelTreeResourceLoader::prefixFromQName(const QString &qName)
{
    const int offset = qName.indexOf(QLatin1Char(':'));

    if(offset == -1)
        return QString();
    else
        return qName.left(offset);
}

bool AccelTreeResourceLoader::streamToReceiver(QIODevice *const dev,
                                               const SequenceReceiver::Ptr &receiver,
                                               const NamePool::Ptr &np)
{
    Q_ASSERT(dev);
    Q_ASSERT(receiver);
    Q_ASSERT(np);

    QXmlStreamReader reader(dev);

    while(!reader.atEnd())
    {
        reader.readNext();

        switch(reader.tokenType())
        {
            case QXmlStreamReader::StartElement:
            {
                /* Send the name. */
                receiver->startElement(np->allocateQName(reader.namespaceUri().toString(), reader.name().toString(),
                                                         prefixFromQName(reader.qualifiedName().toString())));

                /* Send namespace declarations. */
                const QXmlStreamNamespaceDeclarations nss(reader.namespaceDeclarations());

                /* The far most common case, is for it to be empty. */
                if(!nss.isEmpty())
                {
                    const int len = nss.size();

                    for(int i = 0; i < len; ++i)
                    {
                        const QXmlStreamNamespaceDeclaration &ns = nss.at(i);
                        receiver->namespaceBinding(np->allocateBinding(ns.prefix().toString(), ns.namespaceUri().toString()));
                    }
                }

                /* Send attributes. */
                const QXmlStreamAttributes attrs(reader.attributes());
                const int len = attrs.size();

                for(int i = 0; i < len; ++i)
                {
                    const QXmlStreamAttribute &attr = attrs.at(i);

                    receiver->attribute(np->allocateQName(attr.namespaceUri().toString(), attr.name().toString(),
                                                          prefixFromQName(attr.qualifiedName().toString())),
                                        attr.value().toString());
                }

                continue;
            }
            case QXmlStreamReader::EndElement:
            {
                receiver->endElement();
                continue;
            }
            case QXmlStreamReader::Characters:
            {
                if(reader.isWhitespace())
                    receiver->whitespaceOnly(reader.text());
                else
                    receiver->characters(reader.text().toString());

                continue;
            }
            case QXmlStreamReader::Comment:
            {
                receiver->comment(reader.text().toString());
                continue;
            }
            case QXmlStreamReader::ProcessingInstruction:
            {
                receiver->processingInstruction(np->allocateQName(QString(), reader.processingInstructionTarget().toString()),
                                                reader.processingInstructionData().toString());
                continue;
            }
            case QXmlStreamReader::StartDocument:
            {
                receiver->startDocument();
                continue;
            }
            case QXmlStreamReader::EndDocument:
            {
                receiver->endDocument();
                continue;
            }
            case QXmlStreamReader::EntityReference:
            /* Fallthrough. */
            case QXmlStreamReader::DTD:
            {
                /* We just ignore any DTD and entity references. */
                continue;
            }
            case QXmlStreamReader::Invalid:
                return false;
            case QXmlStreamReader::NoToken:
            {
                Q_ASSERT_X(false, Q_FUNC_INFO,
                           "This token is never expected to be received.");
                return false;
            }
        }
    }

    return true;
}

Item AccelTreeResourceLoader::openDocument(const QUrl &uri)
{
    const AccelTree::Ptr doc(m_loadedDocuments.value(uri));

    if(doc)
        return doc->root(Node()); /* Pass in dummy object. We know AccelTree doesn't use it. */
    else
    {
        if(retrieveDocument(uri))
            return m_loadedDocuments.value(uri)->root(Node()); /* Pass in dummy object. We know AccelTree doesn't use it. */
        else
            return Item();
    }
}

SequenceType::Ptr AccelTreeResourceLoader::announceDocument(const QUrl &uri, const Usage)
{
    // TODO deal with the usage thingy
    Q_ASSERT(uri.isValid());
    Q_ASSERT(!uri.isRelative());
    Q_UNUSED(uri); /* Needed when compiling in release mode. */

    return CommonSequenceTypes::ZeroOrOneDocumentNode;
}

bool AccelTreeResourceLoader::isDocumentAvailable(const QUrl &uri)
{
    return retrieveDocument(uri);
}

// vim: et:ts=4:sw=4:sts=4

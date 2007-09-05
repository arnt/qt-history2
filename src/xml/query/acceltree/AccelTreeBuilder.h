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

#ifndef Patternist_AccelTreeBuilder_H
#define Patternist_AccelTreeBuilder_H

#include <QSet>
#include <QStack>

#include "AccelTree.h"
#include "NamePool.h"
#include "NodeBuilder.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Builds an AccelTree from a stream of XML/Item events
     * received through the NodeBuilder interface.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AccelTreeBuilder : public NodeBuilder
    {
    public:
        typedef PlainSharedPtr<AccelTreeBuilder> Ptr;

        AccelTreeBuilder(const QUrl &docURI, const QUrl &baseURI, const NamePool::Ptr &np);
        virtual void startDocument();
        virtual void endDocument();
        virtual void startElement(const QName name);
        virtual void endElement();
        virtual void attribute(const QName name, const QString &value);
        virtual void characters(const QString &ch);
        virtual void whitespaceOnly(const QStringRef &ch);
        virtual void processingInstruction(const QName target,
                                           const QString &data);
        virtual void namespaceBinding(const NamespaceBinding nb);
        virtual void comment(const QString &content);
        virtual void item(const Item &it);

        virtual NodeModel::Ptr builtDocument();
        virtual NodeBuilder::Ptr create(const QUrl &baseURI) const;

        inline AccelTree::Ptr builtDocument() const
        {
            return m_document;
        }

    private:
        inline void startStructure();

        inline AccelTree::PreNumber currentDepth() const
        {
            return m_ancestors.count() -1;
        }

        inline AccelTree::PreNumber currentParent() const
        {
            return m_ancestors.isEmpty() ? -1 : m_ancestors.top();
        }

        enum Constants
        {
            DefaultNodeStackSize = 10,
            SizeIsEmpty = 0
        };

        AccelTree::PreNumber            m_preNumber;
        bool                            m_isPreviousAtomic;
        bool                            m_hasCharacters;
        /**
         * Whether m_characters has been run through
         * CompressedWhitespace::compress().
         */
        bool                            m_isCharactersCompressed;
        QString                         m_characters;
        NamePool::Ptr                   m_namePool;
        AccelTree::Ptr                  m_document;
        QStack<AccelTree::PreNumber>    m_ancestors;
        QStack<AccelTree::PreNumber>    m_size;

        /** If we have already commenced a document, we don't want to
         * add more document nodes. We keep track of them with this
         * counter, which ensures that startDocument() and endDocument()
         * are skipped consistently. */
        AccelTree::PreNumber            m_skippedDocumentNodes;

        /**
         * All attribute values goes through this set such that we store only
         * one QString for identical attribute values.
         */
        QSet<QString>                   m_attributeCompress;
        const QUrl                      m_documentURI;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_AccelTree_H
#define Patternist_AccelTree_H

#include <QHash>
#include <QUrl>
#include <QVector>

#include "qitem_p.h"
#include "qnamepool_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Stores an XML document using the XPath Accelerator scheme, also
     * known as pre/post numbering.
     *
     * Working on this code will be destructive without a proper understanding of
     * the Accelerator scheme, so do check out the links. We don't implement any form
     * of staircase join, although that is only due to time constraints.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @see <a href="http://www.pathfinder-xquery.org/?q=research/xpath-accel">XPath
     * Accelerator</a>
     * @see <a href="http://www.pathfinder-xquery.org/files/xpath-accel.pdf">Accelerating
     * XPath Location Steps, Torsten Grust</a>
     * @see <a href="http://citeseer.ist.psu.edu/cache/papers/cs/29367/http:zSzzSzwww.informatik.uni-konstanz.dezSz~grustzSzfileszSzstaircase-join.pdf/grust03staircase.pdf">Staircase Join:
     * Teach a Relational DBMS to Watch its (Axis) Steps<a/>
     * @see <a href="http://ftp.cwi.nl/CWIreports/INS/INS-E0510.pdf">Loop-lifted
     * staircase join: from XPath to XQuery, Torsten Grust</a>
     * @see <a href="http://englich.wordpress.com/2007/01/09/xmlstat/">xmlstat, Frans Englich</a>
     * @see <a href"http://www.inf.uni-konstanz.de/dbis/publications/download/accelerating-locsteps.pdf">Accelerating
     * XPath Evaluation in Any RDBMS, Torsten Grust</a>
     */
    class AccelTree : public NodeModel
    {
    public:
        using NodeModel::createNode;

        typedef PlainSharedPtr<AccelTree> Ptr;
        typedef qint32 PreNumber;
        typedef PreNumber PostNumber;
        typedef qint8 Depth;

        inline AccelTree(const QUrl &docURI,
                         const QUrl &bURI) : m_documentURI(docURI),
                                             m_baseURI(bURI)
        {
            /* Pre-allocate at least a little bit. */
            // TODO. Do it according to what an average 4 KB doc contains.
            basicData.reserve(100);
            data.reserve(30);
        }

        /**
         * @short Houses data for a node, and that all node kinds have.
         *
         * BasicNodeData is internal to the Accel tree implementation, and is
         * only used by those classes.
         *
         * @author Frans Englich <fenglich@trolltech.com>
         * @todo Can't m_kind be coded somewhere else? If m_name is invalid,
         * its bits can be used to distinguish the node types that doesn't have
         * names, and for elements, attributes and processing instructions, we need
         * two bits, somewhere. Attributes and processing instructions can't have a
         * size, is that of help? There's also certain rules for the names. For instance,
         * a processing instruction will never have a prefix nor namespace. Neither
         * will an attribute node have a default, non-empty namespace, right?
         * @todo Compress text nodes, add general support for it in Patternist.
         */
        class BasicNodeData
        {
        public:
            inline BasicNodeData()
            {
            }

            inline BasicNodeData(const PreNumber aDepth,
                                 const PreNumber aParent,
                                 const Node::NodeKind k,
                                 const PreNumber s,
                                 const QName n = QName()) : m_parent(aParent)
                                                          , m_size(s)
                                                          , m_name(n)
                                                          , m_depth(aDepth)
                                                          , m_kind(k)
            {
            }

            inline Depth depth() const
            {
                return m_depth;
            }

            inline PreNumber parent() const
            {
                return m_parent;
            }

            /**
             * @see AccelTree::size()
             */
            inline PreNumber size() const
            {
                /* Remember that we use the m_size to signal compression if
                 * we're a text node. */
                if(m_kind == Node::Text)
                    return 0;
                else
                    return m_size;
            }

            inline void setSize(const PreNumber aSize)
            {
                m_size = aSize;
            }

            inline Node::NodeKind kind() const
            {
                return m_kind;
            }

            inline QName name() const
            {
                return m_name;
            }

            inline bool isCompressed() const
            {
                Q_ASSERT_X(m_kind == Node::Text, Q_FUNC_INFO,
                           "Currently, only text nodes are compressed.");
                /* Note, we don't call size() here, since it has logic for text
                 * nodes. */
                return m_size == IsCompressed;
            }

        private:
            /**
             * This is the pre number of the parent.
             */
            PreNumber       m_parent;

            /**
             * This is the count of children this node has.
             *
             * In the case of a text node, which cannot have children,
             * it is set to IsCompressed, if the content has been the result
             * of CompressedWhitespace::compress(). If it's not compressed,
             * it is zero.
             */
            PreNumber       m_size;

            /**
             * For text nodes, and less importantly, comments,
             * this variable is not used.
             */
            QName           m_name;

            Depth           m_depth;
            Node::NodeKind  m_kind : 7;
        };

        virtual QUrl baseURI(const Node ni) const;
        virtual QUrl documentURI(const Node ni) const;
        virtual Node::NodeKind kind(const Node ni) const;
        virtual Node::DocumentOrder compareOrderTo(const Node ni1, const Node ni2) const;
        virtual Node root(const Node n) const;
        virtual Node parent(const Node ni) const;
        virtual Item::Iterator::Ptr iterate(const Node ni, const Node::Axis axis) const;
        virtual QName name(const Node ni) const;
        virtual NamespaceBinding::Vector namespaceBindings(const Node n) const;
        virtual void sendNamespaces(const Node n,
                                    const PlainSharedPtr<SequenceReceiver> &receiver) const;
        virtual QString stringValue(const Node n) const;
        virtual Item::Iterator::Ptr typedValue(const Node n) const;
        virtual ItemType::Ptr type(const Node ni) const;

        friend class AccelTreeBuilder;

        enum Constants
        {
            IsCompressed = 1
        };

        /**
         * The key is the pre number of an element, and the value is a vector
         * containing the namespace declarations being declared on that
         * element. Therefore, it does not reflect the namespaces being in
         * scope for that element.
         */
        QHash<PreNumber, NamespaceBinding::Vector> namespaces;

        /**
         * Stores data for nodes. The QHash's value is the data of the processing instruction, and the
         * content of a text node or comment.
         */
        QHash<PreNumber, QString> data;

        QVector<BasicNodeData> basicData;

        inline QUrl documentURI() const
        {
            return m_documentURI;
        }

        inline QUrl baseURI() const
        {
            return m_baseURI;
        }

        inline bool hasChildren(const PreNumber pre) const
        {
            return basicData.at(pre).size() > 0;
        }

        inline PreNumber parent(const PreNumber pre) const
        {
            return basicData.at(pre).parent();
        }

        inline bool hasParent(const PreNumber pre) const
        {
            qDebug() << "hasParent(), asked for:" << pre << "whose depth is:" << basicData.at(pre).depth();
            return basicData.at(pre).depth() > 0;
        }

        inline bool hasFollowingSibling(const PreNumber pre) const
        {
            return pre < maximumPreNumber();
        }

        inline PostNumber postNumber(const PreNumber pre) const
        {
            const BasicNodeData &b = basicData.at(pre);
            return pre + b.size() - b.depth();
        }

        inline Node::NodeKind kind(const PreNumber pre) const
        {
            return basicData.at(pre).kind();
        }

        inline PreNumber maximumPreNumber() const
        {
            return basicData.count() - 1;
        }

        inline PreNumber toPreNumber(const Node n) const
        {
            return n.data();
        }

        inline PreNumber size(const PreNumber pre) const
        {
            Q_ASSERT_X(basicData.at(pre).size() != -1, Q_FUNC_INFO,
                       "The size cannot be -1. That means an unitialized value is attempted to be used.");
            return basicData.at(pre).size();
        }

        inline Depth depth(const PreNumber pre) const
        {
            return basicData.at(pre).depth();
        }

        void printStats(const NamePool::Ptr &np) const;

        inline QName name(const PreNumber pre) const
        {
            return basicData.at(pre).name();
        }

        inline bool isCompressed(const PreNumber pre) const
        {
            return basicData.at(pre).isCompressed();
        }

        static inline bool hasPrefix(const NamespaceBinding::Vector &nbs, const QName::PrefixCode prefix);

        QUrl m_documentURI;
        QUrl m_baseURI;
    };
}

Q_DECLARE_TYPEINFO(Patternist::AccelTree::BasicNodeData, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

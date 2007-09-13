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

#ifndef Patternist_QObjectNodeModel_H
#define Patternist_QObjectNodeModel_H

#include "qnamepool_p.h"
#include "qitem_p.h"
#include "qdynamiccontext_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

class QObject;

namespace Patternist
{
    class PropertyToAtomicValue;
    /**
     * @short Delegates QtCore's QObject into Patternist's NodeModel.
     * known as pre/post numbering.
     *
     * QObjectNodeModel sets the toggle on Node to @c true, if it
     * represents a property of the QObject. That is, if the Node is
     * an attribute.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class QObjectNodeModel : public NodeModel
    {
    public:
        typedef PlainSharedPtr<QObjectNodeModel> Ptr;
        using NodeModel::createNode;
        QObjectNodeModel(const NamePool::Ptr &np);

        /**
         * A QObject tree doesn't have a natural base URI.  For that
         * reason we return QCoreApplication::applicationFilePath().
         */
        virtual QUrl baseURI(const Node ni) const;

        /**
         * @short Returns the same as baseURI().
         */
        virtual QUrl documentURI(const Node ni) const;

        virtual Node::NodeKind kind(const Node ni) const;
        virtual Node::DocumentOrder compareOrderTo(const Node ni1, const Node ni2) const;
        virtual Node root(const Node n) const;
        virtual Node parent(const Node ni) const;
        virtual Item::Iterator::Ptr iterate(const Node ni, const Node::Axis axis) const;
        virtual QName name(const Node ni) const;
        virtual NamespaceBinding::Vector namespaceBindings(const Node n) const;

        /**
         * @short We don't have any namespaces, so we do nothing.
         */
        virtual void sendNamespaces(const Node n,
                                    const PlainSharedPtr<SequenceReceiver> &receiver) const;
        virtual QString stringValue(const Node n) const;
        virtual Item::Iterator::Ptr typedValue(const Node n) const;
        virtual ItemType::Ptr type(const Node ni) const;

        inline Item mapToItem(const QObject *const,
                              const DynamicContext::Ptr &context) const;

        inline Iterator<QObject *>::Ptr mapToSequence(const QObject *const,
                                                      const DynamicContext::Ptr &context) const;
    private:
        friend class QObjectPropertyToAttributeIterator;
        enum
        {
            /**
             * The highest bit set.
             */
            IsAttribute = 1 << 31
        };

        static inline const QObject *asQObject(const Node n);
        static inline bool isProperty(const Node n);
        static inline QMetaProperty toMetaProperty(const Node n);
        /**
         * Returns the ancestors of @p n. Does therefore not include
         * @p n.
         */
        inline Item::List ancestors(const Node n) const;
        const NamePool::Ptr m_namePool;
        const QUrl m_baseURI;
        inline Item propertyValue(const Node n) const;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_SequenceReceiver_H
#define Patternist_SequenceReceiver_H

#include <QSharedData>

#include "NamespaceBinding.h"
#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A push interface for the XPath Data Model. Similar to SAX's
     * ContentHandler.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SequenceReceiver : public QSharedData
    {
    public:
        typedef PlainSharedPtr<SequenceReceiver> Ptr;

        inline SequenceReceiver()
        {
        }

        virtual ~SequenceReceiver();

        /**
         * @short Signals the start of an element by name @p name.
         */
        virtual void startElement(const QName name) = 0;

        /**
         * @short Signals the presence of the namespace declaration @p nb.
         *
         * This event is received @c after startElement(), as opposed to
         * SAX, and before any attribute() events.
         */
        virtual void namespaceBinding(const NamespaceBinding nb) = 0;

        /**
         * @short Signals the end of the current element.
         */
        virtual void endElement() = 0;

        /**
         * @short Signals the presence of an attribute node.
         *
         * This function is guaranteed by the caller to always be
         * called after a call to startElement() or attribute().
         *
         * @param name the name of the attribute. Guaranteed to always be
         * non-null.
         * @param value the value of the attribute. Guaranteed to always be
         * non-null.
         */
        virtual void attribute(const QName name,
                               const QString &value) = 0;

        virtual void processingInstruction(const QName name,
                                           const QString &value) = 0;
        virtual void comment(const QString &value) = 0;

        /**
         * @short Sends an Item to this SequenceReceiver that may be a Node or an
         * AtomicValue.
         */
        virtual void item(const Item &item) = 0;

        /**
         * Sends a text node with value @p value. Adjascent text nodes
         * may be sent. There's no restrictions on @p value, beyond that it
         * must be valid XML characters. For instance, @p value may contain
         * only whitespace.
         *
         * @see whitespaceOnly()
         */
        virtual void characters(const QString &value) = 0;

        /**
         * This function may be called instead of characters() if, and only if,
         * @p value consists only of whitespace.
         *
         * The caller gurantees that @p value, is not empty.
         *
         * By whitespace is meant a sequence of characters that are either
         * spaces, tabs, or the two new line characters, in any order. In
         * other words, Unicode's whitespace category is not considered
         * whitespace.
         *
         * However, there's no guarantee or requirement that whitespaceOnly()
         * is called for text nodes containing whitespace only, characters()
         * may be called just as well. This is why the default implementation
         * for whitespaceOnly() calls characters().
         *
         * @see characters()
         */
        virtual void whitespaceOnly(const QStringRef &value);

        /**
         * Start of a document node.
         */
        virtual void startDocument() = 0;

        /**
         * End of a document node.
         */
        virtual void endDocument() = 0;

    protected:
        /**
         * Treats @p outputItem as an node and calls the appropriate function,
         * such as attribute() or comment(), depending on its Node::NodeKind.
         *
         * This a helper function sub-classes can use to multi-plex Nodes received
         * via item().
         *
         * @param outputItem must be a Node.
         */
        void sendAsNode(const Item &outputItem);

    private:
        /**
         * Call sendAsNode() for each child of @p node. As consistent with the
         * XPath Data Model, this does not include attribute nodes.
         */
        template<const Node::Axis axis>
        inline void sendFromAxis(const Node node);
        Q_DISABLE_COPY(SequenceReceiver)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

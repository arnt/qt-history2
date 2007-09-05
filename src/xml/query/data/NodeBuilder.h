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

#ifndef Patternist_NodeBuilder_H
#define Patternist_NodeBuilder_H

#include "Item.h"
#include "SequenceReceiver.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Receives SequenceReceiver events and builds a node tree
     * in memory that afterwards can be retrieved via builtNode()
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class NodeBuilder : public SequenceReceiver
    {
    public:
        typedef PlainSharedPtr<NodeBuilder> Ptr;

        inline NodeBuilder()
        {
        }

        /**
         * @short Returns the document that has been built.
         *
         * If this function is called before any events have been received, the result is undefined.
         *
         * The top node that was constructed can be retrieved by calling
         * NodeModel::root() on the returned NodeModel.
         *
         * This function is not @c const, because some implementations delay
         * the node construction until the node is needed. Also, text nodes are
         * difficult, at best, to construct until one knows that all text content
         * has been received(which a call to builtNode() in a natural way
         * signals).
         */
        virtual NodeModel::Ptr builtDocument() = 0;

        /**
         * @short Creates a copy of this NodeBuilder, that operates independently of
         * this NodeBuilder.
         */
        virtual NodeBuilder::Ptr create(const QUrl &baseURI) const = 0;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

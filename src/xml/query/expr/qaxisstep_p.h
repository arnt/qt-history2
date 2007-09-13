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

#ifndef Patternist_AxisStep_H
#define Patternist_AxisStep_H

#include "qitem_p.h"
#include "qemptycontainer_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short A step in a path expression that with an axis and a node test evaluates
     * to a sequence of nodes from the context item.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class Q_AUTOTEST_EXPORT AxisStep : public EmptyContainer
    {
    public:
        typedef PlainSharedPtr<AxisStep> Ptr;

        AxisStep(const Node::Axis axis,
                 const ItemType::Ptr &nodeTest);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        /**
         * Returns @p node if it matches the node test this step is using, otherwise @c null.
         */
        inline Item mapToItem(const Item &node,
                                   const DynamicContext::Ptr &context) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        /**
         * Rewrites to ParentNodeAxis, if possible.
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        /**
         * @returns always BuiltinTypes::node;
         */
        virtual ItemType::Ptr expectedContextItemType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        virtual Properties properties() const;

        /**
         * @returns the axis this step is using.
         */
        Node::Axis axis() const;

        /**
         * @returns the node test this step is using.
         */
        inline ItemType::Ptr nodeTest() const
        {
            return m_nodeTest;
        }

    private:
        static const Node::NodeKind s_whenAxisNodeKindEmpty[];

        /**
         * @returns @c true when the axis @p axis and a node test testing node of
         * type @p nodeKind always produces an empty sequence. One such example
         * is <tt>attribute::comment()</tt>.
         */
        static bool isAlwaysEmpty(const Node::Axis axis, const Node::NodeKind nodeKind);

        const Node::Axis    m_axis;
        const ItemType::Ptr m_nodeTest;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

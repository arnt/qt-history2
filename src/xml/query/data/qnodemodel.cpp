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

#include <QUrl>

#include "NamespaceResolver.h"
#include "ListIterator.h"

#include "Item.h"

using namespace Patternist;

NodeModel::~NodeModel()
{
}

/**
 * @file
 * @short Contains the implementation of NodeModel. The definition is in Item.h.
 */

QName::NamespaceCode NodeModel::namespaceForPrefix(const Node ni,
                                                   const QName::PrefixCode prefix) const
{
    Q_ASSERT(kind(ni) == Node::Element);

    const NamespaceBinding::Vector nbs(namespaceBindings(ni));
    const int len = nbs.size();

    for(int i = 0; i < len; ++i)
    {
        if(nbs.at(i).prefix() == prefix)
            return nbs.at(i).namespaceURI();
    }

    return NamespaceResolver::NoBinding;
}

bool NodeModel::isDeepEqual(const Node n1,
                            const Node n2) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(n1);
    Q_ASSERT(n2);

    const Node::NodeKind nk = n1.kind();

    if(nk != n2.kind())
        return false;

    if(n1.name() != n2.name())
        return false;

    switch(nk)
    {
        case Node::Element:
        {
            Item::Iterator::Ptr atts1(n1.iterate(Node::AttributeAxis));
            Item node(atts1->next());

            const Item::List atts2(n2.iterate(Node::AttributeAxis)->toList());
            const Item::List::const_iterator end(atts2.constEnd());

            while(node)
            {
                bool equal = false;
                for(Item::List::const_iterator it = atts2.constBegin(); it != end; ++it)
                {
                    if(isDeepEqual(node.asNode(), (*it).asNode()))
                        equal = true;
                }

                if(!equal)
                    return false;

                node = atts1->next();
            }

            /* Fallthrough, so we check the children. */
        }
        case Node::Document:
        {
            Item::Iterator::Ptr itn1(n1.iterate(Node::Child));
            Item::Iterator::Ptr itn2(n2.iterate(Node::Child));

            while(true)
            {
                Item no1(itn1->next());
                Item no2(itn2->next());

                while(no1 && isIgnorable(no1.asNode()))
                    no1 = itn1->next();

                while(no2 && isIgnorable(no2.asNode()))
                    no2 = itn2->next();

                if(no1 && no2)
                {
                   if(!isDeepEqual(no1.asNode(), no2.asNode()))
                       return false;
                }
                else
                    return !no1 && !no2;
            }

            return true;
        }
        case Node::Attribute:
        /* Fallthrough */
        case Node::ProcessingInstruction:
        /* Fallthrough. */
        case Node::Text:
        /* Fallthrough. */
        case Node::Comment:
            return n1.stringValue() == n2.stringValue();
        case Node::Namespace:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "Not implemented");
            return false;
        }
    }

    return false;
}

bool NodeModel::isIgnorable(const Node n)
{
    Q_ASSERT(n);
    const Node::NodeKind nk = n.kind();
    return nk == Node::ProcessingInstruction ||
           nk == Node::Comment;
}


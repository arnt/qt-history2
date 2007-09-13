/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include <QVariant>

#include "qlistiterator_p.h"
#include "qitem_p.h"
#include "qxmlname.h"
#include "qxmlquery_p.h"

#include "qpullbridge_p.h"

QT_BEGIN_NAMESPACE

/*!
  \brief Bridges a Patternist::SequenceIterator to QAbstractXmlPullProvider.
  \internal
  \since 4.4
  \reentrant

  The approach of this class is rather straight forward since Patternist::SequenceIterator
  and QAbstractXmlPullProvider are conceptually similar. While Patternist::SequenceIterator only
  delivers top level items(since it's not an event stream, it's a list of items), PullBridge
  needs to recursively iterate the children of nodes too, which is achieved through the
  stack m_iterators.
 */

QAbstractXmlPullProvider::Event PullBridge::next()
{
    m_item = m_iterators.top().second->next();

    if(m_item)
    {
        if(m_item.isAtomicValue())
            m_current = AtomicValue;
        else
        {
            Q_ASSERT(m_item.isNode());

            switch(m_item.asNode().kind())
            {
                case Patternist::Node::Attribute:
                {
                    m_current = Attribute;
                    break;
                }
                case Patternist::Node::Comment:
                {
                    m_current = Comment;
                    break;
                }
                case Patternist::Node::Element:
                {
                    m_iterators.push(qMakePair(StartElement, m_item.asNode().iterate(Patternist::Node::Child)));
                    m_current = StartElement;
                    break;
                }
                case Patternist::Node::Document:
                {
                    m_iterators.push(qMakePair(StartDocument, m_item.asNode().iterate(Patternist::Node::Child)));
                    m_current = StartDocument;
                    break;
                }
                case Patternist::Node::Namespace:
                {
                    m_current = Namespace;
                    break;
                }
                case Patternist::Node::ProcessingInstruction:
                {
                    m_current = ProcessingInstruction;
                    break;
                }
                case Patternist::Node::Text:
                {
                    m_current = Text;
                    break;
                }
            }
        }
    }
    else
    {
        if(m_iterators.isEmpty())
            m_current = EndOfInput;
        else
        {
            switch(m_iterators.top().first)
            {
                case StartOfInput:
                {
                    m_current = EndOfInput;
                    break;
                }
                case StartElement:
                {
                    m_current = EndElement;
                    m_iterators.pop();
                    break;
                }
                case StartDocument:
                {
                    m_current = EndDocument;
                    m_iterators.pop();
                    break;
                }
                default:
                {
                    Q_ASSERT_X(false, Q_FUNC_INFO,
                               "Invalid value.");
                    m_current = EndOfInput;
                }
            }
        }

    }

    return m_current;
}

QAbstractXmlPullProvider::Event PullBridge::current() const
{
    return m_current;
}

QXmlName PullBridge::name() const
{
    if(m_item && m_item.isNode())
        return QXmlQueryPrivate::fromPoolName(m_item.asNode().name());
    else
        return QXmlName();
}

QVariant PullBridge::atomicValue() const
{
    // TODO
    return QVariant();
}

QString PullBridge::stringValue() const
{
    if(m_item)
        return m_item.stringValue();
    else
        return QString();
}

QT_END_NAMESPACE

// vim: et:ts=4:sw=4:sts=4

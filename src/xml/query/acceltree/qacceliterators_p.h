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

#ifndef Patternist_AccelIterators_H
#define Patternist_AccelIterators_H

#include "AccelTree.h"
#include "Item.h"
#include "ListIterator.h"
#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Abstract base class for Iterators for the AccelTree, that
     * contains common functions and members.
     */
    class AccelIterator : public Item::Iterator
    {
    public:
        virtual xsInteger position() const;
        virtual Item current() const;

    protected:
        inline AccelIterator(const AccelTree *const doc,
                             const AccelTree::PreNumber pre,
                             const AccelTree::PreNumber currentPre) : m_document(doc),
                                                                      m_preNumber(pre),
                                                                      m_currentPre(currentPre),
                                                                      m_position(0)

        {
            Q_ASSERT(m_document);
            Q_ASSERT(m_preNumber >= 0);
        }

        inline Item closedExit()
        {
            m_position = -1;
            m_current.reset();
            return Item();
        }

        /**
         * We do not own it.
         */
        const AccelTree *const      m_document;

        const AccelTree::PreNumber  m_preNumber;
        AccelTree::PreNumber        m_currentPre;
        xsInteger                   m_position;
        Item                        m_current;
    };

    template<const bool IncludeSelf>
    class AncestorIterator : public AccelIterator
    {
    public:
        /**
         * @p pre is the node from which iteration starts
         * from. In the @c ancestor axis it is excluded,
         * while in @c ancestor-or-self it is included. @p pre
         * must have at least one ancestor.
         */
        inline AncestorIterator(const AccelTree *const doc,
                                const AccelTree::PreNumber pre) : AccelIterator(doc, pre, IncludeSelf ? pre : doc->basicData.at(pre).parent())
        {
            Q_ASSERT(IncludeSelf || m_document->hasParent(pre));
        }

        virtual Item next()
        {
            if(m_currentPre == -1)
                return closedExit();
            else
            {
                ++m_position;
                m_current = m_document->createNode(m_currentPre);
                m_currentPre = m_document->basicData.at(m_currentPre).parent();

                return m_current;
            }
        }

        virtual Item::Iterator::Ptr copy() const
        {
            return Item::Iterator::Ptr(new AncestorIterator<IncludeSelf>(m_document, m_preNumber));
        }
    };

    class ChildIterator : public AccelIterator
    {
    public:
        /**
         * @p pre must have at least one child.
         */
        inline ChildIterator(const AccelTree *const doc,
                             const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre + 1),
                                                               m_depth(m_document->depth(m_currentPre))
        {
            Q_ASSERT(m_document->hasChildren(pre));

            /* Skip the attributes, that are children in the pre/post plane, of
             * the node we're applying the child axis to. */
            while(m_document->kind(m_currentPre) == Node::Attribute)
            {
                ++m_currentPre;
                /* We check the depth here because we would otherwise include
                 * following siblings. */
                if(m_currentPre > m_document->maximumPreNumber() || m_document->depth(m_currentPre) != m_depth)
                {
                    m_currentPre = -1;
                    break;
                }
            }
        }

        virtual Item next();
        virtual Item::Iterator::Ptr copy() const;

    private:
        const AccelTree::Depth m_depth;
    };

    template<const bool IsFollowing>
    class SiblingIterator : public AccelIterator
    {
    public:
        inline SiblingIterator(const AccelTree *const doc,
                               const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre + (IsFollowing ? 0 : -1)),
                                                                 m_depth(doc->depth(pre))
        {
            Q_ASSERT_X(IsFollowing || pre != 0, "",
                       "When being preceding-sibling, the context node cannot be the first node in the document.");
            Q_ASSERT_X(!IsFollowing || pre != m_document->maximumPreNumber(), "",
                       "When being following-sibling, the context node cannot be the last node in the document.");
        }

        virtual Item next()
        {
            if(m_currentPre == -1)
                return Item();

            if(IsFollowing)
            {
                /* Skip the descendants, and jump to the next node. */
                m_currentPre += m_document->size(m_currentPre) + 1;

                if(m_currentPre > m_document->maximumPreNumber() || m_document->depth(m_currentPre) != m_depth)
                    return closedExit();
                else
                {
                    ++m_position;
                    m_current = m_document->createNode(m_currentPre);
                    return m_current;
                }
            }
            else
            {
                qDebug() << "m_depth:" << m_depth << "m_currentPre:" << m_currentPre << "depth:" << m_document->depth(m_currentPre)
                         << "kind:" << m_document->kind(m_currentPre);

                while(m_document->depth(m_currentPre) > m_depth)
                    --m_currentPre;

                while(m_document->kind(m_currentPre) == Node::Attribute)
                    --m_currentPre;

                if(m_document->depth(m_currentPre) == m_depth &&
                   m_document->kind(m_currentPre) != Node::Attribute)
                {
                    m_current = m_document->createNode(m_currentPre);
                    ++m_position;
                    --m_currentPre;
                    return m_current;
                }
                else
                {
                    m_currentPre = -1;
                    return closedExit();
                }
            }
        }

        virtual Item::Iterator::Ptr copy() const
        {
            return Item::Iterator::Ptr(new SiblingIterator<IsFollowing>(m_document, m_preNumber));
        }

    private:
        const AccelTree::Depth m_depth;
    };

    /**
     * @short Implements axis @c descendant and @c descendant-or-self for the
     * AccelTree.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<const bool IncludeSelf>
    class DescendantIterator : public AccelIterator
    {
    public:
        /**
         * @p pre must have at least one child.
         */
        inline DescendantIterator(const AccelTree *const doc,
                                  const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre + (IncludeSelf ? 0 : 1)),
                                                                    m_postNumber(doc->postNumber(pre))
        {
            Q_ASSERT(IncludeSelf || m_document->hasChildren(pre));

            /* Make sure that m_currentPre is the first node part of this axis.
             * Since we're not including ourself, advance to the node after our
             * attributes, if any. */
            if(!IncludeSelf)
            {
                while(m_document->kind(m_currentPre) == Node::Attribute)
                {
                    ++m_currentPre;
                    /* We check the depth here because we would otherwise include
                     * following siblings. */
                    if(m_currentPre > m_document->maximumPreNumber() || m_document->postNumber(m_currentPre) > m_postNumber)
                    {
                        m_currentPre = -1;
                        break;
                    }
                }
            }
        }

        virtual Item next()
        {
            //qDebug() << Q_FUNC_INFO << "m_currentPre:" << m_currentPre << "m_postNumber" << m_postNumber;
            if(m_currentPre == -1)
                return closedExit();

            ++m_position;
            m_current = m_document->createNode(m_currentPre);

            ++m_currentPre;

            if(m_currentPre > m_document->maximumPreNumber())
            {
                m_currentPre = -1;
                return m_current;
            }

            if(m_document->postNumber(m_currentPre) < m_postNumber)
            {
                while(m_document->kind(m_currentPre) == Node::Attribute)
                {
                    ++m_currentPre;
                    if(m_currentPre > m_document->maximumPreNumber())
                    {
                        m_currentPre = -1;
                        break;
                    }
                }
            }
            else
                m_currentPre = -1;

            return m_current;
        }

        virtual Item::Iterator::Ptr copy() const
        {
            return Item::Iterator::Ptr(new DescendantIterator<IncludeSelf>(m_document, m_preNumber));
        }

    private:
        const AccelTree::PreNumber m_postNumber;
    };

    /**
     * @short Implements axis @c following for the AccelTree.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FollowingIterator : public AccelIterator
    {
    public:
        /**
         * @ pre must have at least one child.
         */
        inline FollowingIterator(const AccelTree *const doc,
                                 const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre)
        {
        }

        virtual Item next();
        virtual Item::Iterator::Ptr copy() const;
    };

    /**
     * @short Implements axis @c preceding for the AccelTree.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class PrecedingIterator : public AccelIterator
    {
    public:
        /**
         * @ pre must have at least one child.
         */
        inline PrecedingIterator(const AccelTree *const doc,
                                 const AccelTree::PreNumber pre) : AccelIterator(doc, pre, 0),
                                                                   m_postNumber(m_document->postNumber(m_preNumber))
        {
            Q_ASSERT(m_preNumber > 0);
        }

        virtual Item next();
        virtual Item::Iterator::Ptr copy() const;

    private:
        const AccelTree::PreNumber  m_postNumber;
    };

    class AttributeIterator : public AccelIterator
    {
    public:
        /**
         * @p pre must have at least one child.
         */
        inline AttributeIterator(const AccelTree *const doc, const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre + 1)
        {
            Q_ASSERT(m_document->hasChildren(pre));
            Q_ASSERT(m_document->kind(m_currentPre) == Node::Attribute);
        }

        virtual Item next();
        virtual Item::Iterator::Ptr copy() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

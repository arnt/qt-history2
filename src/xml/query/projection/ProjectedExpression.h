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
***************************************************************************
*/

#ifndef Patternist_ProjectedExpression_H
#define Patternist_ProjectedExpression_H

#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    class ProjectedExpression
    {
    public:
        typedef ProjectedExpression * Ptr;
        typedef QVector<ProjectedExpression::Ptr> Vector;
        virtual ~ProjectedExpression()
        {
        }

        enum Action
        {
            Move = 0,
            Skip = 1,
            Keep = 2,
            KeepSubtree = 4 | Keep
        };

        virtual Action actionForElement(const QName name,
                                        ProjectedExpression::Ptr &next) const
        {
            Q_UNUSED(name);
            Q_UNUSED(next);
            return Skip;
        }

    };

    class ProjectedNodeTest
    {
    public:
        typedef ProjectedNodeTest * Ptr;
        virtual ~ProjectedNodeTest()
        {
        }

        virtual bool isMatch(const Node::NodeKind kind) const
        {
            Q_UNUSED(kind);
            return false;
        }
    };

    class ProjectedStep : public ProjectedExpression
    {
    public:
        ProjectedStep(const ProjectedNodeTest::Ptr test,
                      const Node::Axis axis) : m_test(test),
                                               m_axis(axis)
        {
            Q_ASSERT(m_test);
        }

        virtual Action actionForElement(const QName name,
                                        ProjectedExpression::Ptr &next) const
        {
            Q_UNUSED(name);
            Q_UNUSED(next);
            // TODO
            return Skip;
        }

    private:
        const ProjectedNodeTest::Ptr    m_test;
        const Node::Axis                m_axis;
    };

    class ProjectedPath : public ProjectedExpression
    {
    public:
        ProjectedPath(const ProjectedExpression::Ptr left,
                      const ProjectedExpression::Ptr right) : m_left(left),
                                                              m_right(right)
        {
            Q_ASSERT(m_left);
            Q_ASSERT(m_right);
        }

        virtual Action actionForElement(const QName name,
                                        ProjectedExpression::Ptr &next) const
        {
            ProjectedExpression::Ptr &candidateNext = next;
            const Action a = m_left->actionForElement(name, candidateNext);

            if(a != Skip)
            {
                /* The test accepted it, so let's replace us with the new step. */
                next = candidateNext;
            }

            return a;
        }

    private:
        const ProjectedExpression::Ptr  m_left;
        const ProjectedExpression::Ptr  m_right;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#include "qdocumentprojector_p.h"

using namespace Patternist;

DocumentProjector::DocumentProjector(const ProjectedExpression::Vector &paths,
                                     const SequenceReceiver::Ptr &receiver) : m_paths(paths),
                                                                              m_pathCount(paths.count()),
                                                                              m_action(ProjectedExpression::Move),
                                                                              m_nodesInProcess(0),
                                                                              m_receiver(receiver)
{
    Q_ASSERT_X(paths.count() > 0, Q_FUNC_INFO,
               "Using DocumentProjector with no paths is an "
               "overhead and has also undefined behavior.");
    Q_ASSERT(receiver);
}

void DocumentProjector::startElement(const QName name)
{
    Q_UNUSED(name);

    switch(m_action)
    {
        case ProjectedExpression::KeepSubtree:
        {
            m_receiver->startElement(name);
            /* Fallthrough. */
        }
        case ProjectedExpression::Skip:
        {
            ++m_nodesInProcess;
            return;
        }
        default:
        {
            Q_ASSERT_X(m_action == ProjectedExpression::Move, Q_FUNC_INFO,
                       "We're not supposed to receive Keep here, because "
                       "endElement() should always end that state.");

            for(int i = 0; i < m_pathCount; ++i)
            {
                m_action = m_paths.at(i)->actionForElement(name, m_paths[i]);

                switch(m_action)
                {
                    case ProjectedExpression::Keep:
                    {
                        m_action = ProjectedExpression::Keep;
                        continue;
                    }
                    case ProjectedExpression::KeepSubtree:
                    {
                        /* Ok, at least one path wanted this node. Pass it on,
                         * and exit. */
                        m_receiver->startElement(name);
                        ++m_nodesInProcess;
                        return;
                    }
                    case ProjectedExpression::Skip:
                    {
                        /* This particular path doesn't need it, but
                         * some other path might, so continue looping. */
                        continue;
                    }
                    case ProjectedExpression::Move:
                        Q_ASSERT_X(false, Q_FUNC_INFO, "The action functions can never return Move.");
                }
            }

            ++m_nodesInProcess;

            if(m_action == ProjectedExpression::Keep)
                m_receiver->startElement(name);
            else
            {
                Q_ASSERT(m_action == ProjectedExpression::Skip);
            }
        }
    }
}

void DocumentProjector::endElement()
{
    if(m_action == ProjectedExpression::Keep)
    {
        Q_ASSERT(m_nodesInProcess == 1);

        m_receiver->endElement();

        /* We have now kept the single node, and now wants to skip
         * all its children. */
        m_action = ProjectedExpression::Skip;
        m_nodesInProcess = 0;
    }
    else if(m_action == ProjectedExpression::KeepSubtree)
    {
        m_receiver->endElement();
        --m_nodesInProcess;

        if(m_nodesInProcess == 0)
        {
            /* We have now skipped all the children, let's do
             * a new path analysis. */
            m_action = ProjectedExpression::Move;
        }
    }
    else
    {
        Q_ASSERT_X(m_action == ProjectedExpression::Skip, Q_FUNC_INFO,
                   "We're not supposed to be in a Move action here.");
        /* We skip calling m_receiver's endElement() here since we're
         * skipping. */
        Q_ASSERT(m_nodesInProcess > 0);
        --m_nodesInProcess;

        if(m_nodesInProcess == 0)
        {
            /* Ok, we've skipped them all, let's do something
             * new -- let's Move on to the next path! */
            m_action = ProjectedExpression::Move;
        }
    }
}

void DocumentProjector::attribute(const QName name,
                                  const QString &value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

void DocumentProjector::namespaceBinding(const NamespaceBinding nb)
{
    Q_UNUSED(nb);
}

void DocumentProjector::comment(const QString &value)
{
    Q_ASSERT_X(!value.contains(QLatin1String("--")), Q_FUNC_INFO,
               "Invalid input; it's the caller's responsibility to ensure the input is correct.");
    Q_UNUSED(value);
}

void DocumentProjector::characters(const QString &value)
{
    Q_UNUSED(value);
}

void DocumentProjector::processingInstruction(const QName name,
                                              const QString &value)
{
    Q_ASSERT_X(!value.contains(QLatin1String("?>")), Q_FUNC_INFO,
               "Invalid input; it's the caller's responsibility to ensure the input is correct.");
    Q_UNUSED(name);
    Q_UNUSED(value);
}

void DocumentProjector::item(const Item &outputItem)
{
    Q_UNUSED(outputItem);
}

void DocumentProjector::startDocument()
{
}

void DocumentProjector::endDocument()
{
}

// vim: et:ts=4:sw=4:sts=4

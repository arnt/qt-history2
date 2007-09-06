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

#ifndef Patternist_DocumentProjector_H
#define Patternist_DocumentProjector_H

#include "ProjectedExpression.h"
#include "SequenceReceiver.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DocumentProjector : public SequenceReceiver
    {
    public:
        DocumentProjector(const ProjectedExpression::Vector &paths,
                          const SequenceReceiver::Ptr &receiver);

        virtual void namespaceBinding(const NamespaceBinding nb);

        virtual void characters(const QString &value);
        virtual void comment(const QString &value);

        virtual void startElement(const QName name);

        virtual void endElement();

        virtual void attribute(const QName name,
                               const QString &value);

        virtual void processingInstruction(const QName name,
                                           const QString &value);

        virtual void item(const Item &item);

        virtual void startDocument();
        virtual void endDocument();

        ProjectedExpression::Vector m_paths;
        const int                   m_pathCount;
        ProjectedExpression::Action m_action;
        int                         m_nodesInProcess;
        const SequenceReceiver::Ptr m_receiver;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

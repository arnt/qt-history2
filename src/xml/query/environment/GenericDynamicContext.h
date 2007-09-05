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

#ifndef Patternist_GenericDynamicContext_H
#define Patternist_GenericDynamicContext_H

#include <QDateTime>
#include <QVector>

#include "DayTimeDuration.h"
#include "StackContextBase.h"
#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A DynamicContext supplying basic information that always is used.
     *
     * This DynamicContext is the first DynamicContext used during
     * a run and is always used. In addition, more contexts, such as
     * a Focus can be created.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GenericDynamicContext : public StackContextBase<DynamicContext>
    {
    public:
        typedef PlainSharedPtr<GenericDynamicContext> Ptr;

        GenericDynamicContext(const NamePool::Ptr &np,
                              QAbstractMessageHandler *const messageHandler,
                              const LocationHash &locations);

        virtual xsInteger contextPosition() const;
        /**
         * @returns always @c null, the focus is always undefined when an GenericDynamicContext
         * is used.
         */
        virtual Item contextItem() const;
        virtual xsInteger contextSize();

        virtual void setFocusIterator(const Item::Iterator::Ptr &it);
        virtual Item::Iterator::Ptr focusIterator() const;

        virtual QAbstractMessageHandler * messageHandler() const;
        virtual PlainSharedPtr<DayTimeDuration> implicitTimezone() const;
        virtual QDateTime currentDateTime() const;

        virtual SequenceReceiver::Ptr outputReceiver() const;
        void setOutputReceiver(const SequenceReceiver::Ptr &receiver);

        virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const;
        void setNodeBuilder(const NodeBuilder::Ptr &builder);

        virtual ResourceLoader::Ptr resourceLoader() const;
        void setResourceLoader(const ResourceLoader::Ptr &loader);

        virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
        void setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader);
        virtual NamePool::Ptr namePool() const;
        virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;
        virtual void addNodeModel(const NodeModel::Ptr &nm);
        virtual QAbstractUriResolver::Ptr uriResolver() const;

    private:
        QAbstractMessageHandler *       m_messageHandler;
        const QDateTime                 m_currentDateTime;
        const DayTimeDuration::Ptr      m_zoneOffset;
        SequenceReceiver::Ptr           m_outputReceiver;
        NodeBuilder::Ptr                m_nodeBuilder;
        ExternalVariableLoader::Ptr     m_externalVariableLoader;
        ResourceLoader::Ptr             m_resourceLoader;
        NamePool::Ptr                   m_namePool;
        const LocationHash              m_locations;
        NodeModel::List                 m_nodeModels;
        QAbstractUriResolver::Ptr       m_uriResolver;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

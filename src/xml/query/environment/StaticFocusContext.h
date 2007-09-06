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

#ifndef Patternist_StaticFocusContext_H
#define Patternist_StaticFocusContext_H

#include <QUrl>

#include "StaticContext.h"
#include "FunctionFactory.h"
#include "SchemaTypeFactory.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A StaticContext that carries a specified static type
     * for the context item, but otherwise delegates to another StaticContext.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StaticFocusContext : public StaticContext
    {
    public:
        StaticFocusContext(const ItemType::Ptr &contextItemType,
                           const StaticContext::Ptr &context);

        virtual NamespaceResolver::Ptr namespaceBindings() const;
        virtual void setNamespaceBindings(const NamespaceResolver::Ptr &);

        virtual FunctionFactory::Ptr functionSignatures() const;
        virtual SchemaTypeFactory::Ptr schemaDefinitions() const;
        virtual DynamicContext::Ptr dynamicContext() const;

        virtual QUrl baseURI() const;
        virtual void setBaseURI(const QUrl &uri);

        virtual bool compatModeEnabled() const;

        virtual QUrl defaultCollation() const;

        virtual QAbstractMessageHandler * messageHandler() const;

        virtual void setDefaultCollation(const QUrl &uri);

        virtual BoundarySpacePolicy boundarySpacePolicy() const;
        virtual void setBoundarySpacePolicy(const BoundarySpacePolicy policy);

        virtual ConstructionMode constructionMode() const;
        virtual void setConstructionMode(const ConstructionMode mode);

        virtual OrderingMode orderingMode() const;
        virtual void setOrderingMode(const OrderingMode mode);
        virtual OrderingEmptySequence orderingEmptySequence() const;
        virtual void setOrderingEmptySequence(const OrderingEmptySequence ordering);

        virtual QString defaultFunctionNamespace() const;
        virtual void setDefaultFunctionNamespace(const QString &ns);

        virtual QString defaultElementNamespace() const;
        virtual void setDefaultElementNamespace(const QString &ns);

        virtual InheritMode inheritMode() const;
        virtual void setInheritMode(const InheritMode mode);

        virtual PreserveMode preserveMode() const;
        virtual void setPreserveMode(const PreserveMode mode);

        /**
         * @returns the type passed in the constructor.
         */
        virtual ItemType::Ptr contextItemType() const;

        virtual StaticContext::Ptr copy() const;

        virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
        virtual ResourceLoader::Ptr resourceLoader() const;
        virtual NamePool::Ptr namePool() const;
        virtual void addLocation(const SourceLocationReflection *const reflection,
                                 const QSourceLocation &location);
        virtual LocationHash sourceLocations() const;
        virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;
        virtual QAbstractUriResolver::Ptr uriResolver() const;

    private:
        const ItemType::Ptr         m_contextItemType;
        const StaticContext::Ptr    m_context;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

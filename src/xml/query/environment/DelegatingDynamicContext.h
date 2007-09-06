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

#ifndef Patternist_DelegatingDynamicContext_H
#define Patternist_DelegatingDynamicContext_H

#include "DynamicContext.h"
#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for dynamic contexts that are
     * created from an existing one.
     *
     * In some cases multiple DynamicContext instances must be used in
     * order to maintain somekind of scope. This class delegates
     * the DynamicContext interface onto another DynamicContext instance,
     * allowing the sub-class to only implement what it needs to.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DelegatingDynamicContext : public DynamicContext
    {
    public:
        virtual xsInteger contextPosition() const;
        virtual Item contextItem() const;
        virtual xsInteger contextSize();

        virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot);
        virtual ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot);

        virtual void setRangeVariable(const VariableSlotID slotNumber,
                                      const Item &newValue);
        virtual Item rangeVariable(const VariableSlotID slotNumber) const;

        virtual void setExpressionVariable(const VariableSlotID slotNumber,
                                           const Expression::Ptr &newValue);
        virtual Expression::Ptr expressionVariable(const VariableSlotID slotNumber) const;

        virtual void setFocusIterator(const Item::Iterator::Ptr &it);
        virtual Item::Iterator::Ptr focusIterator() const;

        virtual Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const;
        virtual void setPositionIterator(const VariableSlotID slot,
                                         const Item::Iterator::Ptr &newValue);

        virtual QAbstractMessageHandler * messageHandler() const;
        virtual PlainSharedPtr<DayTimeDuration> implicitTimezone() const;
        virtual QDateTime currentDateTime() const;
        virtual SequenceReceiver::Ptr outputReceiver() const;
        virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const;
        virtual ResourceLoader::Ptr resourceLoader() const;
        virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
        virtual NamePool::Ptr namePool() const;
        virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;
        virtual void addNodeModel(const NodeModel::Ptr &nm);
        virtual QAbstractUriResolver::Ptr uriResolver() const;

    protected:
        DelegatingDynamicContext(const DynamicContext::Ptr &prevContext);

    private:
        const DynamicContext::Ptr m_prevContext;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

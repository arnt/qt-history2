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

#ifndef Patternist_DynamicContext_H
#define Patternist_DynamicContext_H

class QDateTime;
template<typename T> class QVector;

#include "CacheCells.h"
#include "ExternalVariableLoader.h"
#include "Item.h"
#include "NamePool.h"
#include "NodeBuilder.h"
#include "ListIterator.h"
#include "Primitives.h"
#include "ReportContext.h"
#include "ResourceLoader.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    class DayTimeDuration;
    class Expression;

    /**
     * @short Carries information and facilities used at runtime, and hence
     * provides a state for that stage in a thread-safe manner.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#eval_context">XQuery
     * 1.0: An XML Query Language, 2.1.2 Dynamic Context</a>
     * @see <a href="http://www.w3.org/TR/xquery/#id-dynamic-evaluation">XQuery
     * 1.0: An XML Query Language, 2.2.3.2 Dynamic Evaluation Phase</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DynamicContext : public ReportContext
    {
    public:
        typedef PlainSharedPtr<DynamicContext> Ptr;

        virtual ~DynamicContext()
        {
        }

        /**
         * This function intentionally returns by reference.
         */
        virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot) = 0;

        /**
         * This function intentionally returns by reference.
         */
        virtual ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot) = 0;

        virtual xsInteger contextPosition() const = 0;
        virtual Item contextItem() const = 0;
        virtual xsInteger contextSize() = 0;

        virtual void setRangeVariable(const VariableSlotID slot,
                                      const Item &newValue) = 0;
        virtual Item rangeVariable(const VariableSlotID slot) const = 0;
        virtual void setExpressionVariable(const VariableSlotID slot,
                                           const PlainSharedPtr<Expression> &newValue) = 0;
        virtual PlainSharedPtr<Expression>
        expressionVariable(const VariableSlotID slot) const = 0;

        virtual Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const = 0;
        virtual void setPositionIterator(const VariableSlotID slot,
                                         const Item::Iterator::Ptr &newValue) = 0;

        virtual void setFocusIterator(const Item::Iterator::Ptr &it) = 0;
        virtual Item::Iterator::Ptr focusIterator() const = 0;

        virtual PlainSharedPtr<DayTimeDuration> implicitTimezone() const = 0;
        virtual QDateTime currentDateTime() const = 0;

        virtual SequenceReceiver::Ptr outputReceiver() const = 0;
        virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const = 0;
        virtual ResourceLoader::Ptr resourceLoader() const = 0;
        virtual ExternalVariableLoader::Ptr externalVariableLoader() const = 0;
        virtual NamePool::Ptr namePool() const = 0;

        DynamicContext::Ptr createFocus() const;
        DynamicContext::Ptr createStack() const;
        DynamicContext::Ptr createReceiverContext(const SequenceReceiver::Ptr &receiver) const;

        /**
         * Whenever a tree gets built, this function is called. DynamicContext
         * has the responsibility of keeping a copy of @p nm, such that it
         * doesn't go out of scope, since no one else will reference @p nm.
         *
         * The caller guarantees that @p nm is not @c null.
         */
        virtual void addNodeModel(const NodeModel::Ptr &nm) = 0;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

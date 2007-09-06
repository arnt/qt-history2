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

#ifndef Patternist_StackContextBase_H
#define Patternist_StackContextBase_H

#include <QVector>

#include "DayTimeDuration.h"
#include "DelegatingDynamicContext.h"
#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for all DynamicContext classes that needs to
     * supply variables.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSuperClass>
    class StackContextBase : public TSuperClass
    {
    public:
        StackContextBase();
        /**
         * Construct a StackContextBase and passes @p prevContext to its super class. This
         * constructor is typically used when the super class is DelegatingDynamicContext.
         */
        StackContextBase(const DynamicContext::Ptr &prevContext);

        virtual void setRangeVariable(const VariableSlotID slotNumber,
                                      const Item &newValue);
        virtual Item rangeVariable(const VariableSlotID slotNumber) const;

        virtual void setExpressionVariable(const VariableSlotID slotNumber,
                                           const Expression::Ptr &newValue);
        virtual Expression::Ptr expressionVariable(const VariableSlotID slotNumber) const;

        virtual Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const;
        virtual void setPositionIterator(const VariableSlotID slot,
                                         const Item::Iterator::Ptr &newValue);
        virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot);
        virtual ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot);

    protected:
        /**
         * This function is protected, although it only is used in this class. I don't
         * know why it has to be, but it won't compile when private.
         */
        template<typename VectorType, typename UnitType>
        inline
        void setSlotVariable(const VariableSlotID slot,
                             const UnitType &newValue,
                             VectorType &container) const;

    private:
        Item::Vector                    m_rangeVariables;
        Expression::Vector              m_expressionVariables;
        Item::Iterator::Vector          m_positionIterators;
        ItemCacheCell::Vector           m_itemCacheCells;
        ItemSequenceCacheCell::Vector   m_itemSequenceCacheCells;
    };

    #include "StackContextBase.cpp"

    /**
     * @short A DynamicContext that creates a new scope for variables.
     *
     * This DynamicContext is used for recursive user function calls, for example.
     */
    typedef StackContextBase<DelegatingDynamicContext> StackContext;
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

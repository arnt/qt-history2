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

#ifndef Patternist_VariableReference_H
#define Patternist_VariableReference_H

template<typename T> class QList;

#include "EmptyContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Baseclass for classes being references to variables.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class VariableReference : public EmptyContainer
    {
    public:
        typedef PlainSharedPtr<VariableReference> Ptr;
        typedef QList<VariableReference::Ptr> List;

        /**
         * Creates a VariableReference.
         *
         * @param slot must be a valid slot. That is, zero or larger.
         */
        VariableReference(const VariableSlotID slot);

        /**
         * @returns the slot that this reference communicates through.
         *
         * This is a slot in the DynamicContext. Which one, depends on the
         * type, which this VariableReference does not have information about.
         * For instance, it could DynamicContext::expressionVariable() or
         * DynamicContext::rangeVariable().
         */
        inline VariableSlotID slot() const;

        /**
         * @returns DisableElimination
         */
        virtual Properties properties() const;

    private:
        /**
         * The slot. Same as returned by slot().
         */
        const VariableSlotID m_slot;
    };

    inline VariableSlotID VariableReference::slot() const
    {
        return m_slot;
    }

}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

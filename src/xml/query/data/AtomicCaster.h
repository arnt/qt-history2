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

#ifndef Patternist_AtomicCaster_H
#define Patternist_AtomicCaster_H

#include "DynamicContext.h"
#include "Item.h"
#include "AtomicTypeDispatch.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short AtomicCaster is an abstract base class for classes
     * that performs casting between two atomic values of specific types.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicCaster : public AtomicTypeVisitorResult
    {
    public:
        typedef PlainSharedPtr<AtomicCaster> Ptr;
        AtomicCaster();
        virtual ~AtomicCaster();

        /**
         * Casts @p from to an atomic value of the type this class
         * casts to, and returns that value. The @p context is used
         * for reporting errors in case the casting fails, and to in general
         * access information from the dynamic context.
         */
        virtual Item castFrom(const Item &from,
                                   const PlainSharedPtr<DynamicContext> &context) const = 0;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

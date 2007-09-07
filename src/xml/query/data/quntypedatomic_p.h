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

#ifndef Patternist_UntypedAtomic_H
#define Patternist_UntypedAtomic_H

#include "qatomicstring_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the value instance of the @c xs:untypedAtomic type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class UntypedAtomic : public AtomicString
    {
    public:
        friend class CommonValues;

        /**
         * Creates an instance representing @p value.
         *
         * This fromValue function takes no context argument because it doesn't need it -- it
         * casting to xs:untypedAtomic always succeeds.
         *
         * @note This function does not remove the string literal escaping allowed in XPath 2.0
         */
        static UntypedAtomic::Ptr fromValue(const QString &value);

        virtual ItemType::Ptr type() const;

    protected:

        UntypedAtomic(const QString &value);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

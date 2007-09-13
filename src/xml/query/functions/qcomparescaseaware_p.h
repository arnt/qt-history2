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

#ifndef Patternist_ComparesCaseAware_H
#define Patternist_ComparesCaseAware_H

#include "qfunctioncall_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Base-class for functions that compares strings and provides
     * an opportunity to optimize compares intended to be case insensitive.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ComparesCaseAware : public FunctionCall
    {
    public:
        /**
         * Performs initialization.
         */
        ComparesCaseAware();

        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * Tells whether the return value of the two operands must be compared
         * case insensitively or not.
         */
        inline Qt::CaseSensitivity caseSensitivity() const
        {
            return m_caseSensitivity;
        }

    private:
        Qt::CaseSensitivity m_caseSensitivity;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

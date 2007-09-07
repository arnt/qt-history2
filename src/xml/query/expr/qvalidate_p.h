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

#ifndef Patternist_Validate_H
#define Patternist_Validate_H

#include "qexpression_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Handles XQuery 1.0's <tt>validate</tt> expression.
     *
     * This class is currently not used. The Schema Validation Feature is not supported.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#id-validate">XQuery 1.0: An XML
     * Query Language, 3.13 Validate Expressions</a>
     * @see <a href="http://www.w3.org/TR/xquery/#id-schema-validation-feature">XQuery 1.0: An
     * XML Query Language, 5.2.2 Schema Validation Feature</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class Validate
    {
    public:

        /**
         * Represents the validation mode.
         */
        enum Mode
        {
            Lax = 1,
            Strict
        };

        /**
         * Creates the necessary Expression instances
         * that validates the operand node @p operandNode in mode @p validationMode,
         * and returns it.
         */
        static Expression::Ptr create(const Expression::Ptr &operandNode,
                                      const Mode validationMode,
                                      const StaticContext::Ptr &context);
    private:
        Validate();
        Q_DISABLE_COPY(Validate)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

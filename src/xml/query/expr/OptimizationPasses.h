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

#ifndef Patternist_OptimizationBlocks_H
#define Patternist_OptimizationBlocks_H

#include "AtomicComparator.h"
#include "Expression.h"
#include "OptimizerFramework.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Contains a set of common OptimizerPass instances.
     *
     * @author Frans englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    namespace OptimizationPasses
    {
        /**
         * A list of OptimizerPass instances that performs the
         * following rewrites:
         *
         * - <tt>count([expr]) ne 0</tt> into <tt>exists([expr])</tt>
         * - <tt>count([expr]) != 0</tt> into <tt>exists([expr])</tt>
         * - <tt>0 ne count([expr])</tt> into <tt>exists([expr])</tt>
         * - <tt>0 != count([expr])</tt> into <tt>exists([expr])</tt>
         * - <tt>count([expr]) eq 0</tt> into <tt>empty([expr])</tt>
         * - <tt>count([expr]) = 0</tt> into <tt>empty([expr])</tt>
         * - <tt>0 eq count([expr])</tt> into <tt>empty([expr])</tt>
         * - <tt>0 = count([expr])</tt> into <tt>empty([expr])</tt>
         * - <tt>count([expr]) ge 1</tt> into <tt>exists([expr])</tt>
         * - <tt>count([expr]) >= 1</tt> into <tt>exists([expr])</tt>
         */
        extern OptimizationPass::List comparisonPasses;

        /**
         * A list of OptimizerPass instances that performs the
         * following rewrites:
         *
         * - <tt>for $var in [expr] return $var</tt> into <tt>[expr]</tt>
         */
        extern OptimizationPass::List forPasses;

        /**
         * A list of OptimizerPass instances that performs the
         * following rewrites:
         *
         * - <tt>if([expr of type xs:boolean]) then true() else false()</tt>
         *   into <tt>[expr of type xs:boolean]</tt>
         */
        extern OptimizationPass::List ifThenPasses;

        /**
         * A list of OptimizerPass instances that performs the
         * following rewrites:
         *
         * - <tt>fn:not(fn:exists([expr]))</tt> into <tt>fn:empty([expr])</tt>
         * - <tt>fn:not(fn:empty([expr]))</tt> into <tt>fn:exists([expr])</tt>
         */
        extern OptimizationPass::List notFN;

        /**
         * Initializes the data members in the OptimizationPasses namespace.
         *
         * This class is not supposed to be instantiated, but to be used via its init()
         * function. In fact, this class cannot be instantiated.
         *
         * @author Frans englich <fenglich@trolltech.com>
         */
        class Coordinator
        {
        public:
            /**
             * Initializes the members in the OptimizationPasses namespace.
             */
            static void init();

        private:
            Q_DISABLE_COPY(Coordinator)
            inline Coordinator();
        };
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

#ifndef Patternist_Aggregator_H
#define Patternist_Aggregator_H

#include "qfunctioncall_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{

    /**
     * @short Carries a staticType() implementation appropriate
     * for functions which returns a singleton value derived from its first argument.
     *
     * One example of such a function is FloorFN, implementing <tt>fn:floor()</tt>,
     * which returns a single value of the same type as the first argument, or the empty
     * sequence if the first argument evaluates to the empty sequence.
     *
     * Aggregator is abstract, and exists for saving code. It is inherited
     * by classes which needs the staticType() implementation this class provides.
     *
     * @see Piper
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Aggregator : public FunctionCall
    {
    public:
        /**
         * @returns a static type where the ItemType is the same as this FunctionCall's first
         * argument, and the Cardinality is as return from Cardinality::toWithoutMany().
         */
        virtual SequenceType::Ptr staticType() const;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

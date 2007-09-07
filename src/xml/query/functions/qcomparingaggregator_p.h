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

#ifndef Patternist_ComparingAggregator_H
#define Patternist_ComparingAggregator_H

/**
 * @file qcomparingaggregator_p.h
 * @short Contains the implementations for the functions <tt>fn:max()</tt>, MaxFN,
 * and <tt>fn:min()</tt>, MinFN, and the class ComparingAggregator.
 */

#include "qabstractfloat_p.h"
#include "qdecimal_p.h"
#include "qcastingplatform_p.h"
#include "qcomparisonplatform_p.h"
#include "qliteral_p.h"
#include "qaggregator_p.h"
#include "quntypedatomicconverter_p.h"
#include "qpatternistlocale_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for the implementations of the <tt>fn:min()</tt> and <tt>fn:max()</tt> function.
     *
     * What function that more specifically is
     * followed, depends on how the constructor is called.
     *
     * @see MaxFN
     * @see MinFN
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template <AtomicComparator::Operator oper, AtomicComparator::ComparisonResult result>
    class ComparingAggregator : public Aggregator,
                                protected ComparisonPlatform<ComparingAggregator<oper, result>,
                                                             true, AtomicComparator::AsValueComparison,
                                                             ReportContext::FORG0006>,
                                protected CastingPlatform<ComparingAggregator<oper, result>, true>
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        inline AtomicComparator::Operator operatorID() const
        {
            return oper;
        }

        inline ItemType::Ptr targetType() const
        {
            return BuiltinTypes::xsDouble;
        }

    private:
        inline Item applyNumericPromotion(const Item &old,
                                               const Item &nev,
                                               const Item &newVal) const;

        using ComparisonPlatform<ComparingAggregator<oper, result>,
                                 true,
                                 AtomicComparator::AsValueComparison,
                                 ReportContext::FORG0006>::comparator;
        using ComparisonPlatform<ComparingAggregator<oper, result>,
                                 true,
                                 AtomicComparator::AsValueComparison,
                                 ReportContext::FORG0006>::fetchComparator;
        using CastingPlatform<ComparingAggregator<oper, result>, true>::cast;
    };

#include "qcomparingaggregator.cpp"

    /**
     * @short An instantiation of ComparingAggregator suitable for <tt>fn:max()</tt>.
     *
     * @ingroup Patternist_functions
     */
    typedef ComparingAggregator<AtomicComparator::OperatorGreaterThan, AtomicComparator::GreaterThan> MaxFN;

    /**
     * @short An instantiation of ComparingAggregator suitable for <tt>fn:max()</tt>.
     *
     * @ingroup Patternist_functions
     */
    typedef ComparingAggregator<AtomicComparator::OperatorLessThan, AtomicComparator::LessThan> MinFN;
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

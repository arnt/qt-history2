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

#ifndef Patternist_ComparisonPlatform_H
#define Patternist_ComparisonPlatform_H

#include "AtomicComparators.h"
#include "Item.h"
#include "DynamicContext.h"
#include "BuiltinTypes.h"
#include "ItemType.h"
#include "Debug.h"
#include "PatternistLocale.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Provides comparison functionality for classes that compare Items,
     * such as ValueComparison or MaxFN.
     *
     * Classes which need comparison functionalities should inherit from this class.
     *
     * The parameter of this template class is the class inheriting from ComparisonPlatform.
     *
     * The class inheriting ComparisonPlatform must implement the following function:
     * @code
     * AtomicComparator::Operator operatorID() const
     * @endcode
     *
     * @author Vincent Ricard <magic@magicninja.org>
     * @ingroup Patternist_expressions
     */
    template <typename TSubClass,
              const bool issueError,
              const AtomicComparator::ComparisonType comparisonType = AtomicComparator::AsValueComparison,
              const ReportContext::ErrorCode errorCode = ReportContext::XPTY0004>
    class ComparisonPlatform
    {
    protected:
        /**
         * Makes ComparisonPlatform use the AtomicComparator @p comparator.
         */
        void prepareComparison(const AtomicComparator::Ptr &comparator);

        /**
         * Default constructor. Does nothing. It is implemented in order make template
         * instantiation easier.
         */
        inline ComparisonPlatform()
        {
        }

        /**
         * Utility function for fetching the appropriate AtomicComparator
         * for two atomic values of type @p type1 and @p type2, for the operator @p op.
         *
         * This function is used throughout the implementation, ranging from the ValueComparison
         * itself, to for example the aggregate functions.
         *
         * @param context the ordinary ReportContext, used for issuing errors.
         * @param type1 the type of the first operand value in a comparison for which the
         * returned AtomicComparator is intended for
         * @param type2 the type of the second operand value in a comparison for which the
         * returned AtomicComparator is intended for. Whether @p type1 and @p type2 corresponds
         * to what is the first second operand type does not have significance, the order
         * can be arbitrary
         */
        AtomicComparator::Ptr
        fetchComparator(const ItemType::Ptr &type1,
                        const ItemType::Ptr &type2,
                        const ReportContext::Ptr &context) const;

        bool compare(const Item &i1,
                     const Item &i2,
                     const AtomicComparator::Ptr &comp,
                     const AtomicComparator::Operator op) const;

        bool
        flexibleCompare(const Item &it1,
                        const Item &it2,
                        const DynamicContext::Ptr &context) const;

        /**
         * @returns the AtomicComparator that has been allocated at compile time,
         * with prepareComparison(). If no AtomicComparator has been allocated
         * for some reason, this function returns @c null.
         */
        inline const AtomicComparator::Ptr &comparator() const
        {
            return m_comparator;
        }

        /**
         * Calling this function makes ComparisonPlatform use a comparator that
         * compares strings case insensitively.
         *
         * @see ValueComparison::isCaseInsensitiveCompare()
         */
        inline void useCaseInsensitiveComparator()
        {
            m_comparator = AtomicComparator::Ptr(new CaseInsensitiveStringComparator());
        }

    private:
        /**
         * @returns the operator that is used.
         */
        inline AtomicComparator::Operator operatorID() const
        {
            Q_ASSERT(static_cast<const TSubClass *>(this)->operatorID());
            return static_cast<const TSubClass *>(this)->operatorID();
        }

        Q_DISABLE_COPY(ComparisonPlatform)

        /**
         * The comparator that is used for comparing atomic values. The AtomicComparator
         * that is used, depends on the static type of the operands. m_comparator can be
         * @c null if it wasn't possible to determine what comparator to use at compile time.
         */
        AtomicComparator::Ptr m_comparator;
    };

#include "ComparisonPlatform.cpp"

}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4

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

/**
 * @file
 * @short This file is included by qcomparisonplatform_p.h.
 * If you need includes in this file, put them in qcomparisonplatform_p.h, outside of the namespace.
 */

template <typename TSubClass, const bool issueError,
          const AtomicComparator::ComparisonType comparisonType, const ReportContext::ErrorCode errorCode>
bool ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
flexibleCompare(const Item &it1,
                const Item &it2,
                const DynamicContext::Ptr &context) const
{
    if(m_comparator)
        /* The comparator was located at compile time. */
        return compare(it1, it2, m_comparator, operatorID());
    else
    {
        const AtomicComparator::Ptr cp(fetchComparator(it1.type(),
                                                       it2.type(),
                                                       context));

        return cp ? compare(it1, it2, cp, operatorID()) : false;
    }
}

template <typename TSubClass, const bool issueError,
          const AtomicComparator::ComparisonType comparisonType, const ReportContext::ErrorCode errorCode>
bool ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
compare(const Item &oand1,
        const Item &oand2,
        const AtomicComparator::Ptr &comp,
        const AtomicComparator::Operator op) const
{
    Q_ASSERT(oand1);
    Q_ASSERT(oand2);
    Q_ASSERT(comp);

    switch(op)
    {
        case AtomicComparator::OperatorEqual:
            return comp->equals(oand1, oand2);
        case AtomicComparator::OperatorNotEqual:
            return !comp->equals(oand1, oand2);
        case AtomicComparator::OperatorLessThan:
            return comp->compare(oand1, op, oand2) == AtomicComparator::LessThan;
        case AtomicComparator::OperatorGreaterThan:
            return comp->compare(oand1, op, oand2) == AtomicComparator::GreaterThan;
        case AtomicComparator::OperatorLessOrEqual:
        {
            const AtomicComparator::ComparisonResult ret = comp->compare(oand1, op, oand2);
            return ret == AtomicComparator::LessThan || ret == AtomicComparator::Equal;
        }
        case(AtomicComparator::OperatorGreaterOrEqual):
        {
            const AtomicComparator::ComparisonResult ret = comp->compare(oand1, op, oand2);
            return ret == AtomicComparator::GreaterThan || ret == AtomicComparator::Equal;
        }
    }

    /* GCC unbarfer, this line should never be reached. */
    Q_ASSERT(false);
    return false;
}

template <typename TSubClass, const bool issueError,
          const AtomicComparator::ComparisonType comparisonType, const ReportContext::ErrorCode errorCode>
AtomicComparator::Ptr ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
fetchComparator(const ItemType::Ptr &t1,
                const ItemType::Ptr &t2,
                const ReportContext::Ptr &context) const
{
    qDebug();
    Q_ASSERT(t1);
    Q_ASSERT(t2);

    if(*BuiltinTypes::xsAnyAtomicType == *t1    ||
       *BuiltinTypes::xsAnyAtomicType == *t2    ||
       *BuiltinTypes::item == *t1               ||
       *BuiltinTypes::item == *t2               ||
       *BuiltinTypes::numeric == *t1            ||
       *BuiltinTypes::numeric == *t2)
    {
        /* The static type of(at least) one of the operands could not
         * be narrowed further, so we do the operator
         * lookup at runtime.
         */
        return AtomicComparator::Ptr();
    }

    const AtomicComparatorLocator::Ptr locator
        (static_cast<const AtomicType *>(t1.get())->comparatorLocator());

    if(!locator)
    {
        if(issueError)
        {
            context->error(tr("No comparisons can be done involving the type %1.")
                                            .arg(formatType(context->namePool(), t1)),
                                       errorCode, static_cast<const TSubClass *>(this));
        }
        return AtomicComparator::Ptr();
    }

    const AtomicComparator::Ptr comp(static_cast<const AtomicType *>(t2.get())->accept(locator, operatorID(),
                                                                                       static_cast<const TSubClass *>(this)));

    if(comp)
        return comp;
    else if(issueError)
    {
        context->error(tr("Operator %1 is not available between atomic values of type %2 and %3.")
                                        .arg(formatKeyword(AtomicComparator::displayName(operatorID(),
                                                                                       comparisonType)),
                                             formatType(context->namePool(), t1),
                                             formatType(context->namePool(), t2)),
                                   errorCode, static_cast<const TSubClass *>(this));
    }

    return AtomicComparator::Ptr();
}

template <typename TSubClass, const bool issueError,
          const AtomicComparator::ComparisonType comparisonType, const ReportContext::ErrorCode errorCode>
void ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
prepareComparison(const AtomicComparator::Ptr &c)
{
    m_comparator = c;
}

// vim: et:ts=4:sw=4:sts=4

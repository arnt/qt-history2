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

#include <QString>

#include "qatomiccomparator_p.h"

using namespace Patternist;

AtomicComparator::AtomicComparator()
{
}

AtomicComparator::~AtomicComparator()
{
}

AtomicComparator::ComparisonResult
AtomicComparator::compare(const Item &,
                          const AtomicComparator::Operator,
                          const Item &) const
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
    return LessThan;
}

QString AtomicComparator::displayName(const AtomicComparator::Operator op,
                                      const ComparisonType type)
{
    Q_ASSERT(type == AsGeneralComparison || type == AsValueComparison);
    if(type == AsGeneralComparison)
    {
        switch(op)
        {
            case OperatorEqual:
                return QLatin1String("=");
            case OperatorGreaterOrEqual:
                return QLatin1String("<=");
            case OperatorGreaterThan:
                return QLatin1String("<");
            case OperatorLessOrEqual:
                return QLatin1String(">");
            case OperatorLessThan:
                return QLatin1String(">=");
            case OperatorNotEqual:
                return QLatin1String("!=");
        }
    }

    switch(op)
    {
        case OperatorEqual:
            return QLatin1String("eq");
        case OperatorGreaterOrEqual:
            return QLatin1String("ge");
        case OperatorGreaterThan:
            return QLatin1String("gt");
        case OperatorLessOrEqual:
            return QLatin1String("le");
        case OperatorLessThan:
            return QLatin1String("lt");
        case OperatorNotEqual:
            return QLatin1String("ne");
    }

    Q_ASSERT(false);
    return QString(); /* GCC unbarfer. */
}

// vim: et:ts=4:sw=4:sts=4

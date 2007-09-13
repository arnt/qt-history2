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

#include "qatomicmathematician_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

AtomicMathematician::~AtomicMathematician()
{
}

QString AtomicMathematician::displayName(const AtomicMathematician::Operator op)
{
    switch(op)
    {
        case AtomicMathematician::Div:
            return QLatin1String("div");
        case AtomicMathematician::IDiv:
            return QLatin1String("idiv");
        case AtomicMathematician::Substract:
            return QLatin1String("-");
        case AtomicMathematician::Mod:
            return QLatin1String("mod");
        case AtomicMathematician::Multiply:
            return QLatin1String("*");
        case AtomicMathematician::Add:
            return QLatin1String("+");
    }

    return QString(); /* Silence GCC warning. */
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE

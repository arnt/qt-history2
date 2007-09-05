/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include "PatternistLocale.h"

#include "Cardinality.h"

using namespace Patternist;

QString Cardinality::displayName(const CustomizeDisplayName explain) const
{
    if(explain == IncludeExplanation)
    {
        if(isEmpty())
            return QString(tr("empty") + QLatin1String("(\"empty-sequence()\")"));
        else if(isZeroOrOne())
            return QString(tr("zero or one") + QLatin1String("(\"?\")"));
        else if(isExactlyOne())
            return QString(tr("exactly one"));
        else if(isOneOrMore())
            return QString(tr("one or more") + QLatin1String("(\"+\")"));
        else
            return QString(tr("zero or more") + QLatin1String("(\"*\")"));
    }
    else
    {
        Q_ASSERT(explain == ExcludeExplanation);

        if(isEmpty() || isZeroOrOne())
            return QLatin1String("?");
        else if(isExactlyOne())
            return QString();
        else if(isExact())
        {
            return QString(QLatin1Char('{'))    +
                   QString::number(maximum())   +
                   QLatin1Char('}');
        }
        else
        {
            if(m_max == -1)
            {
                if(isOneOrMore())
                    return QChar::fromLatin1('+');
                else
                    return QChar::fromLatin1('*');
            }
            else
            {
                /* We have a range. We use a RegExp-like syntax. */
                return QString(QLatin1Char('{'))    +
                       QString::number(minimum())   +
                       QLatin1String(", ")          +
                       QString::number(maximum())   +
                       QLatin1Char('}');

            }
        }
    }
}

// vim: et:ts=4:sw=4:sts=4

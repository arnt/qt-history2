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

#include "PatternistLocale.h"

namespace Patternist
{
    QString escape(const QString &input)
    {
        QString rich;
        rich.reserve(int(input.length() * 1.1));

        for(int i = 0; i < input.length(); ++i)
        {
            if(input.at(i) == QLatin1Char('<'))
                rich += QLatin1String("&lt;");
            else if (input.at(i) == QLatin1Char('>'))
                rich += QLatin1String("&gt;");
            else if (input.at(i) == QLatin1Char('&'))
                rich += QLatin1String("&amp;");
            else
                rich += input.at(i);
        }

        return rich;
    }
}

// vim: et:ts=4:sw=4:sts=4

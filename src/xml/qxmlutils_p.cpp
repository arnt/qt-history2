/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QRegExp>
#include <QString>

#include "qxmlutils_p.h"

bool QXmlUtils::isEncName(const QString &encName)
{
    /* Right, we here have a dependency on QRegExp. Writing a manual parser to
     * replace that regexp is probably a 70 lines so I prioritize this to when
     * the dependency is considered alarming, or when the rest of the bugs
     * are fixed. */
    const QRegExp encNameRegExp(QLatin1String("[A-Za-z][A-Za-z0-9._\\-]*"));
    Q_ASSERT(encNameRegExp.isValid());

    return encNameRegExp.exactMatch(encName);
}


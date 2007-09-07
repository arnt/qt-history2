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

#include <QStringList>

#include "CommonValues.h"
#include "ListIterator.h"

#include "XPathHelper.h"

using namespace Patternist;

bool XPathHelper::isReservedNamespace(const QName::NamespaceCode ns)
{
    /* The order is because of that XFN and WXS are the most common. */
    return ns == StandardNamespaces::fn     ||
           ns == StandardNamespaces::xs     ||
           ns == StandardNamespaces::xml    ||
           ns == StandardNamespaces::xsi;
}

bool XPathHelper::isNCName(const QString &ncName)
{
    if(ncName.isEmpty())
        return false;

    QChar ch(ncName.at(0));

    if(!ch.isLetter() && ch != QLatin1Char('_'))
        return false;

    const int len = ncName.length();

    for(int i = 1; i < len; ++i)
    {
        ch = ncName.at(i);

        if(!ch.isLetter() && !ch.isDigit() && ch != QLatin1Char('.') &&
           ch != QLatin1Char('-') && ch != QLatin1Char('_') &&
           ch.category() != QChar::Mark_SpacingCombining)
        {
            return false;
        }
    }

    return true;
}

bool XPathHelper::isQName(const QString &qName)
{
    const QStringList result(qName.split(QLatin1Char(':')));
    const int c = result.count();

    if(c == 2)
    {
        return isNCName(result.first()) &&
               isNCName(result.last());
    }
    else if(c == 1)
        return isNCName(result.first());
    else
        return false;
}

void XPathHelper::splitQName(const QString &qName, QString &prefix, QString &ncName)
{
    Q_ASSERT_X(isQName(qName), Q_FUNC_INFO,
               "qName must be a valid QName.");

    const QStringList result(qName.split(QLatin1Char(':')));

    if(result.count() == 1)
    {
        Q_ASSERT(isNCName(result.first()));
        ncName = result.first();
    }
    else
    {
        Q_ASSERT(result.count() == 2);
        Q_ASSERT(isNCName(result.first()));
        Q_ASSERT(isNCName(result.last()));

        prefix = result.first();
        ncName = result.last();
    }
}

// vim: et:ts=4:sw=4:sts=4

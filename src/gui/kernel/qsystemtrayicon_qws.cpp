/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsystemtrayicon_p.h"

void QSystemTrayIconPrivate::install()
{
}

void QSystemTrayIconPrivate::remove()
{
}

void QSystemTrayIconPrivate::updateIcon()
{
}

void QSystemTrayIconPrivate::updateMenu()
{
}

void QSystemTrayIconPrivate::updateToolTip()
{
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable()
{
    return false;
}

void QSystemTrayIconPrivate::showMessage(const QString &message,
                                         const QString &title,
                                         QSystemTrayIcon::MessageIcon icon,
                                         int msecs)
{
    Q_UNUSED(message);
    Q_UNUSED(title);
    Q_UNUSED(icon);
    Q_UNUSED(msecs);
}


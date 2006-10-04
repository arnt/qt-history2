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

#include "qsystemtrayicon_p.h"

#ifndef QT_NO_SYSTEMTRAYICON

void QSystemTrayIconPrivate::install_sys()
{
}

void QSystemTrayIconPrivate::remove_sys()
{
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
    return QRect();
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return false;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &message,
                                             const QString &title,
                                             QSystemTrayIcon::MessageIcon icon,
                                             int msecs)
{
    Q_UNUSED(message);
    Q_UNUSED(title);
    Q_UNUSED(icon);
    Q_UNUSED(msecs);
}

#endif // QT_NO_SYSTEMTRAYICON

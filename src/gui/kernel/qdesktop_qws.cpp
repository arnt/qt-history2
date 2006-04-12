/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#include "qdesktop.h"
#include <qsettings.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>

#ifndef QT_NO_DESKTOP

bool QDesktop::launchMailComposer(const QString &to, const QString &subject, const QString &body)
{
    qDebug("QDesktop::launchMailComposer not implemented");
    return false;
}

bool QDesktop::launchMailComposer(const QStringList &to, const QString &subject, const QString &body)
{
    return launchMailComposer(to.join(","), subject, body);
}

bool QDesktop::launchWebBrowser(const QUrl &url)
{
    qDebug("QDesktop::launchWebBrowser not implemented");
    return false;
}

bool QDesktop::open(const QUrl &file)
{
    qDebug("QDesktop::open not implemented");
    return false;
}
/*
QString QDesktop::storageLocation(const Location type)
{
    QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
    settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
    switch (type) {
    case Desktop:
        return settings.value("Desktop").toString();
        break;

    case Documents:
        return settings.value("Personal").toString();
        break;

    case Fonts:
        return settings.value("Fonts").toString();
        break;

    case Applications:
        return settings.value("Programs").toString();
        break;

    case Music:
        return settings.value("My Music").toString();
        break;

    case Movies:
        return settings.value("My Video").toString();
        break;

    case Pictures:
        return settings.value("My Pictures").toString();
        break;

    case QDesktop::Home:
        return QDir::homePath(); break;

    case QDesktop::Temp:
        return QDir::tempPath(); break;

    default:
        break;
    }

    return QString();
}

QString QDesktop::displayName(const Location type)
{
    Q_UNUSED(type);
    return QString();
}
*/
#endif // QT_NO_DESKTOP


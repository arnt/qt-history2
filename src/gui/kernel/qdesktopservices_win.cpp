/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#include "qdesktopservices.h"
#include <windows.h>
#include <qsettings.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>

#ifndef QT_NO_DESKTOP

bool QDesktop::launchMailComposer(const QString &to, const QString &subject, const QString &body)
{
    QString url = QString("mailto:%1?subject='%2'&body='%3'").arg(to).arg(subject).arg(body);
    return launchWebBrowser(url);
}

bool QDesktop::launchMailComposer(const QStringList &to, const QString &subject, const QString &body)
{
    return launchMailComposer(to.join(","), subject, body);
}

bool QDesktop::launchWebBrowser(const QUrl &url)
{
    return open(url);
}

bool QDesktop::open(const QUrl &file)
{
    if (!file.isValid())
        return false;

    QT_WA({
                ShellExecute(0, 0, (TCHAR*)QString(file.toEncoded()).utf16(), 0, 0, SW_SHOWNORMAL);
            } , {
                ShellExecuteA(0, 0, QString(file.toEncoded()).toLocal8Bit(), 0, 0, SW_SHOWNORMAL);
            });

    return true;
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


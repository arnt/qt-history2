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

#include <QtGui/QtGui>
#ifndef QT_NO_DESKTOP

#include <qprocess.h>
#include <qdir.h>
#include <QX11Info>
#include <private/qt_x11_p.h>
#include <qdebug.h>
#include <qlocale.h>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#endif

/*!
  QDesktop::QDesktop provides an interface into the basic desktop capabilities.
 */


/*!
  Starts the default e-mail client with the specified to, subject and body
  */
bool QDesktop::launchMailComposer(const QString &to, const QString &subject, const QString &body)
{
    QString url = QString("mailto:%1?subject='%2'&body='%3'").arg(to).arg(subject).arg(body);
    return open(url);
}

bool QDesktop::launchMailComposer(const QStringList &to, const QString &subject, const QString &body)
{
    return launchMailComposer(to.join(","), subject, body);
}

/*!
  Opens the \a url in the default web browser and returns true on success.
 */
bool QDesktop::launchWebBrowser(const QUrl &url)
{
    if (!url.isValid())
        return false;

    // TODO use dbus when it is ready else fall back to the following

    QStringList clients = QString("kfmclient:openURL,gnome-open,firefox,mozilla,netscape").split(",");
    if (X11->desktopEnvironment == DE_GNOME)
        clients.swap(0,1);

    for (int i = 0; i < clients.size(); ++i){
        QString client = clients.at(i);
        QStringList args = client.split(":");
        client = args.at(0);
        args.pop_front();
        args += url.toString();
        if(QProcess::startDetached(client, args))
            return true;
    }
    return false;
}

/*!
  Opens the \a file using the system to determine what application should handle it.
  Returns true on success otherwise false.
 */
bool QDesktop::open(const QUrl &file)
{
    if (!file.isValid())
        return false;

    // TODO use dbus when it is ready else fall back to the following

    QStringList clients = QString("kfmclient:exec,gnome-open,firefox,mozilla,netscape").split(",");
    if (X11->desktopEnvironment == DE_GNOME)
        clients.swap(0,1);

    for (int i = 0; i < clients.size(); ++i){
        QString client = clients.at(i);
        QStringList args = client.split(":");
        client = args.at(0);
        args.pop_front();
        args += file.toString();
        if(QProcess::startDetached(client, args))
            return true;
    }
    return false;
}
/*
    enum Location {  // -> StandardLocation
        Desktop, // -> add Path suffix to each value
        Documents,
        Fonts,
        Applications,
        Music,
        Movies,
        Pictures,
        Temp,
        Home
    };

    QString storageLocation(Location type); // -> location() ### Qt 5: rename to path()
    QString displayName(Location type);
*/
/*!
    \enum QDesktop::Location

    This enum describes the different locations that can be queried
    by QDesktop::storageLocation and QDesktop::displayName.

    \value Desktop Returns the users desktop.
    \value Documents Returns the users document.
    \value Fonts Returns the users fonts.
    \value Applications Returns the users applications.
    \value Music Returns the users music.
    \value Movies Returns the users movies.
    \value Pictures Returns the users pictures.
    \value Temp Returns the system's temporary directory.
    \value Home Returns the user's home directory.

    \sa storageLocation() displayName()
*/

/*!
  Returns the default system directory where file of type belong or an invalid QUrl if it is unable to figure out.
  */
/*
QString QDesktop::storageLocation(const Location type)
{
    QDir emptyDir;
    switch (type) {
        case Desktop:
            return QDir::homePath()+"/Desktop";
            break;

        case Documents:
            if (emptyDir.exists(QDir::homePath()+"/Documents"))
                return QDir::homePath()+"/Documents";
            break;

        case Pictures:
            if (emptyDir.exists(QDir::homePath()+"/Pictures"))
                 return QDir::homePath()+"/Pictures";
            break;

        case Fonts:
            return QDir::homePath()+"/.fonts";
            break;

        case Music:
            if (emptyDir.exists(QDir::homePath()+"/Music"))
                    return QDir::homePath()+"/Music";
            break;

        case Movies:
            if (emptyDir.exists(QDir::homePath()+"/Movies"))
                    return QDir::homePath()+"/Movies";
            break;

        case QDesktop::Home:
            return QDir::homePath();
            break;

        case QDesktop::Temp:
            return QDir::tempPath();
            break;

        case Applications:
        default:
            break;
    }
    return QString();
}
*/
/*!
  Returns a localized display name for a location type or an empty QString if it is unable to figure out.
  */
/*
QString QDesktop::displayName(const Location type)
{
    Q_UNUSED(type);
    switch (type) {
        case Desktop:
            return QObject::tr("Desktop");
            break;

        case Documents:
            return QObject::tr("Documents");
            break;

        case Pictures:
            return QObject::tr("Pictures");
            break;

        case Fonts:
            return QObject::tr("Fonts");
            break;

        case Music:
            return QObject::tr("Music");
            break;

        case Movies:
            return QObject::tr("Movies");
            break;

        case QDesktop::Home:
            return QObject::tr("Home");
            break;

        case QDesktop::Temp:
            return QObject::tr("Temp");
            break;

        case Applications:
            return QObject::tr("Applications");
        default:
            break;
    }
    return QString();
}
*/
#endif // QT_NO_DESKTOP


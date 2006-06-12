/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesktopservices.h"

#ifndef QT_NO_DESKTOPSERVICES

#include <QtGui/QtGui>
#include <qtextstream.h>
#include <qprocess.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qx11info_x11.h>
#include <private/qt_x11_p.h>
#include <qdebug.h>
#include <qlocale.h>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#endif
#include <stdlib.h>

static bool launch(const QUrl &url, const QStringList &clients)
{
    if (!url.isValid())
        return false;

    // TODO use dbus when it is ready else fall back to the following

    for (int i = 0; i < clients.size(); ++i) {
        if (QProcess::startDetached(clients.at(i) + " " + url.toEncoded()))
            return true;
    }
    return false;
}

/*!
    Opens the \a url in the default web browser and returns true on success otherwise false.

    Passing a mailto url will result in a e-mail composer window opening in the default
    e-mail client similar to when a user clicks on a mailto link in a web browser.

    Example mailto url:
    <code>
    "mailto:user@foo.com?subject=Test&body=Just a test"
    </code>

    Note: Only some e-mail clients support @attachement and can handle unicode.
*/
bool QDesktopServices::launchWebBrowser(const QUrl &url)
{
    if (url.scheme() == "mailto")
        return openDocument(url);

    QStringList clients;
    clients << "firefox" << "mozilla" << "netscape" << "opera";
    if (X11->desktopEnvironment == DE_GNOME)
        clients.prepend("gnome-open");
    else if (X11->desktopEnvironment == DE_KDE)
        clients.prepend("kfmclient openURL");

    clients.prepend(getenv("BROWSER"));
    clients.prepend(getenv("DEFAULT_BROWSER")); // AIX
    clients.prepend("xdg-open");
    return launch(url, clients);
}

/*!
    Opens the \a file using the system to determine what application should handle it.
    Returns true on success otherwise false.
 */
bool QDesktopServices::openDocument(const QUrl &file)
{
    QStringList clients;
    clients << "firefox" << "mozilla" << "netscape" << "opera";
    if (X11->desktopEnvironment == DE_GNOME)
        clients.prepend("gnome-open");
    else if (X11->desktopEnvironment == DE_KDE)
        clients.prepend("kfmclient exec");

    clients.prepend("xdg-open");
    return launch(file, clients);
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
/*
    \enum QDesktopServices::Location

    This enum describes the different locations that can be queried
    by QDesktopServices::storageLocation and QDesktopServices::displayName.

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

/*
    Returns the default system directory where file of type belong or an invalid QUrl
    if it is unable to figure out.
  */
/*
QString QDesktopServices::storageLocation(const Location type)
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

    case QDesktopServices::Home:
        return QDir::homePath();
        break;

    case QDesktopServices::Temp:
        return QDir::tempPath();
        break;

    case Applications:
    default:
        break;
    }
    return QString();
}
*/
/*
    Returns a localized display name for a location type or
    an empty QString if it is unable to figure out.
  */
/*
QString QDesktopServices::displayName(const Location type)
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

    case QDesktopServices::Home:
        return QObject::tr("Home");
        break;

    case QDesktopServices::Temp:
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
#endif // QT_NO_DESKTOPSERVICES


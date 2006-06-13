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

#include <qprocess.h>
#include <qurl.h>
#include <private/qt_x11_p.h>
#include <stdlib.h>

inline static bool launch(const QUrl &url, const QString &client)
{
    return (QProcess::startDetached(client + " " + url.toEncoded()));
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
    if (!url.isValid())
        return false;
    if (url.scheme() == "mailto")
        return openDocument(url);

    if (launch(url, "xdg-open"))
        return true;
    if (launch(url, getenv("DEFAULT_BROWSER")))
        return true;
    if (launch(url, getenv("BROWSER")))
        return true;

    if (X11->desktopEnvironment == DE_GNOME && launch(url, "gnome-open")) {
        return true;
    } else {
        if (X11->desktopEnvironment == DE_KDE && launch(url, "kfmclient openURL"))
            return true;
    }

    if (launch(url, "firefox"))
        return true;
    if (launch(url, "mozilla"))
        return true;
    if (launch(url, "netscape"))
        return true;
    if (launch(url, "opera"))
        return true;
    return false;
}

/*!
    Opens the \a file using the system to determine what application should handle it.
    Returns true on success otherwise false.
 */
bool QDesktopServices::openDocument(const QUrl &url)
{
    if (!url.isValid())
        return false;

    if (launch(url, "xdg-open"))
        return true;

    if (X11->desktopEnvironment == DE_GNOME && launch(url, "gnome-open")) {
        return true;
    } else {
        if (X11->desktopEnvironment == DE_KDE && launch(url, "kfmclient exec"))
            return true;
    }

    if (launch(url, "firefox"))
        return true;
    if (launch(url, "mozilla"))
        return true;
    if (launch(url, "netscape"))
        return true;
    if (launch(url, "opera"))
        return true;

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


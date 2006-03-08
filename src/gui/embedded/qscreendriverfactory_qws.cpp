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

#include "qscreendriverfactory_qws.h"

#include "qscreen_qws.h"
#include "qapplication.h"
#include "qscreenlinuxfb_qws.h"
#include "qscreentransformed_qws.h"
#include "qscreenvfb_qws.h"
#include "qscreenvnc_qws.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qscreendriverplugin_qws.h"

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QScreenDriverFactoryInterface_iid, QCoreApplication::libraryPaths(), "/gfxdrivers"))

#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

/*!
    \class QScreenDriverFactory
    \ingroup qws

    \brief The QScreenDriverFactory class creates QScreen objects.

    Note that this class is only available in \l {Qtopia Core}.

    The QScreen class and its descendants manage the framebuffer and
    palette. QScreenDriverFactory creates a QScreen object using the
    create() function and a key identifying the driver.

    The valid keys (i.e. the supported drivers) can be retrieved using
    the keys() function. The drivers are either built-in or
    dynamically loaded from a driver plugin (see \l
    QScreenDriverPlugin).

    \sa QScreen, QScreenDriverPlugin
*/

/*!
    Creates a QScreen object for the driver specified by the given \a
    key using the display specified by \a displayId. The returned
    QScreen object may be created from a built-in driver, or from a
    driver plugin. Keys are case sensitive.

    \sa keys()
*/
QScreen *QScreenDriverFactory::create(const QString& key, int displayId)
{
    QString driver = key.toLower();
#ifndef QT_NO_QWS_QVFB
    if (driver == "qvfb" || driver.isEmpty())
        return new QVFbScreen(displayId);
#endif
#ifndef QT_NO_QWS_LINUXFB
    if (driver == "linuxfb" || driver.isEmpty())
        return new QLinuxFbScreen(displayId);
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    if (driver == "transformed")
        return new QTransformedScreen(displayId);
#endif
#ifndef QT_NO_QWS_VNC
    if (driver == "vnc")
        return new QVNCScreen(displayId);
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY

    if (QScreenDriverFactoryInterface *factory = qobject_cast<QScreenDriverFactoryInterface*>(loader()->instance(key)))
        return factory->create(driver, displayId);

#endif
#endif
    return 0;
}

/*!
    Returns the list of valid keys, i.e. the drivers this factory can
    create screens for.

    \l {Qtopia Core} currently supports the following drivers by
    default: \c QVfb (\l {Qtopia Core}'s virtual framebuffer), \c
    LinuxFb (The Linux framebuffer), \c Transformed (for rotated
    displays) and \c VNC (a \l {Running as a VNC Server}{VNC} server).

    \sa create()
*/
QStringList QScreenDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_QVFB
    if (!list.contains("QVFb"))
        list << "QVFb";
#endif
#ifndef QT_NO_QWS_LINUXFB
    if (!list.contains("LinuxFb"))
        list << "LinuxFb";
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    if (!list.contains("Transformed"))
        list << "Transformed";
#endif
#ifndef QT_NO_QWS_VNC
    if (!list.contains("VNC"))
        list << "VNC";
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
     list += loader()->keys();
#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

    return list;
}

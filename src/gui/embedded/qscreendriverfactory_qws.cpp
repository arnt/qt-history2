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
    \class QScreenDriverFactory qscreendriverfactory.h
    \brief The QScreenDriverFactory class creates QScreen objects for Qtopia Core.

    \ingroup qws

    The graphics driver factory creates a QScreen object for a given
    key with QScreenDriverFactory::create(key).

    The drivers are either built-in or dynamically loaded from a
    driver plugin (see \l QScreenDriverPlugin).

    QScreenDriverFactory::keys() returns a list of valid keys. Qt
    currently ships with "LinuxFb".

    This class is only available in \l{Qtopia Core}.
*/

/*!
    Creates a QScreen object of a type that matches \a key, and with
    the ID, \a displayId. The QScreen object returned may be from a
    built-in driver, or from a driver plugin.

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
    Returns the list of keys this factory can create drivers for.

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

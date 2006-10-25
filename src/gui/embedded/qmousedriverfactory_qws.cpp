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

#include "qmousedriverfactory_qws.h"

#include "qapplication.h"
#include "qmousepc_qws.h"
#include "qmousebus_qws.h"
#include "qmousevr41xx_qws.h"
#include "qmouseyopy_qws.h"
#include "qmouselinuxtp_qws.h"
#include "qmousevfb_qws.h"
#include "qmousetslib_qws.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qmousedriverplugin_qws.h"
#include "qdebug.h"

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWSMouseHandlerFactoryInterface_iid, QCoreApplication::libraryPaths(),
     QLatin1String("/mousedrivers")))

#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

/*!
    \class QMouseDriverFactory
    \ingroup qws

    \brief The QMouseDriverFactory class creates QWSMouseHandler
    objects.

    Note that this class is only available in \l {Qtopia Core}.

    The QWSMouseHandler class is used to implement mouse
    drivers. QMouseDriverFactory creates a QWSMouseHandler object
    using the create() function and a key identifying the driver.

    The valid keys can be retrieved using the keys() function.  The
    drivers are either built-in or dynamically loaded from a driver
    plugin (see \l QMouseDriverPlugin).

    \sa QWSMouseHandler, QMouseDriverPlugin, {Qtopia Core Pointer
    Handling}
*/

/*!
    Creates a QWSMouseHandler object for the driver specified by the
    given \a key, for the given \a device. The device is either a
    built-in driver, or a driver from a driver plugin. Keys are
    case-insensitive.

    \sa keys()
*/
QWSMouseHandler *QMouseDriverFactory::create(const QString& key, const QString &device)
{
    QString driver = key.toLower();
#ifndef QT_NO_QWS_MOUSE_LINUXTP
    if (driver == QLatin1String("linuxtp") || driver.isEmpty())
        return new QWSLinuxTPMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_YOPY
    if (driver == QLatin1String("yopy") || driver.isEmpty())
        return new QWSYopyMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_VR41XX
    if (driver == QLatin1String("vr41xx") || driver.isEmpty())
        return new QWSVr41xxMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_PC
    if (driver == QLatin1String("auto")
        || driver == QLatin1String("intellimouse")
        || driver == QLatin1String("microsoft")
        || driver == QLatin1String("mousesystems")
        || driver == QLatin1String("mouseman")
        || driver.isEmpty()) {
        return new QWSPcMouseHandler(key, device);
    }
#endif
#ifndef QT_NO_QWS_MOUSE_BUS
    if (driver == QLatin1String("bus"))
        return new QWSBusMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_TSLIB
    if (driver == QLatin1String("tslib") || driver.isEmpty())
        return new QWSTslibMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_QVFB
    if (driver == QLatin1String("qvfbmouse") || driver == QLatin1String("qvfb"))
        return new QVFbMouseHandler(key, device);
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    if (QWSMouseHandlerFactoryInterface *factory = qobject_cast<QWSMouseHandlerFactoryInterface*>(loader()->instance(driver)))
        return factory->create(driver);
#endif
#endif
    return 0;
}

/*!
    Returns the list of valid keys, i.e. the keys this factory can
    create drivers for.

    \l {Qtopia Core} currently supports the following drivers by
    default: \c MouseMan, \c IntelliMouse, \c Microsoft, \c VR41xx, \c
    LinuxTP, \c Yopy and \c Tslib.

    \sa create()
*/
QStringList QMouseDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_MOUSE_LINUXTP
    list << QLatin1String("LinuxTP");
#endif
#ifndef QT_NO_QWS_MOUSE_YOPY
    list << QLatin1String("Yopy");
#endif
#ifndef QT_NO_QWS_MOUSE_VR41XX
    list << QLatin1String("VR41xx");
#endif
#ifndef QT_NO_QWS_MOUSE_PC
    list << QLatin1String("Auto")
         << QLatin1String("IntelliMouse")
         << QLatin1String("Microsoft")
         << QLatin1String("MouseSystems")
         << QLatin1String("MouseMan");
#endif
#ifndef QT_NO_QWS_MOUSE_BUS
    list << QLatin1String("Bus");
#endif
#ifndef QT_NO_QWS_MOUSE_TSLIB
    list << QLatin1String("Tslib");
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    QStringList plugins = loader()->keys();
    for (int i = 0; i < plugins.size(); ++i) {
        if (!list.contains(plugins.at(i)))
            list += plugins.at(i);
    }
#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

    return list;
}

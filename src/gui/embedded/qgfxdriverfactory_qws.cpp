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

#include "qgfxdriverfactory_qws.h"

#include "qscreen_qws.h"
#include "qapplication.h"
#include "qgfxlinuxfb_qws.h"
#include "qgfxtransformed_qws.h"
#include "qgfxsnap_qws.h"
#include "qgfxmach64_qws.h"
#include "qgfxvoodoo_qws.h"
#include "qgfxmatrox_qws.h"
#include "qgfxvfb_qws.h"
#include "qgfxvnc_qws.h"
#include "qgfxvga16_qws.h"
#include "qgfxshadowfb_qws.h"
#include "qgfxrepeater_qws.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qgfxdriverplugin_qws.h"

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QGfxDriverFactoryInterface_iid, QCoreApplication::libraryPaths(), "/gfxdrivers"))

#endif //QT_NO_COMPONENT
#endif //QT_MAKEDLL

/*!
    \class QGfxDriverFactory qgfxdriverfactory.h
    \brief The QGfxDriverFactory class creates QScreen objects for Qt/Embedded.

    \ingroup qws

    The graphics driver factory creates a QScreen object for a given
    key with QGfxDriverFactory::create(key).

    The drivers are either built-in or dynamically loaded from a
    driver plugin (see \l QGfxDriverPlugin).

    QGfxDriverFactory::keys() returns a list of valid keys. Qt
    currently ships with "LinuxFb".

    This class is only available in Qt/Embedded.
*/

/*!
    Creates a QScreen object of a type that matches \a key, and with
    the ID, \a displayId. The QScreen object returned may be from a
    built-in driver, or from a driver plugin.

    \sa keys()
*/
QScreen *QGfxDriverFactory::create(const QString& key, int displayId)
{
    QString driver = key.toLower();
#ifndef QT_NO_QWS_QVFB
    if (driver == "qvfb" || driver.isEmpty())
        return new QVFbScreen(displayId);
#endif
#ifndef QT_NO_QWS_SNAP
    if ( driver == "snap" )
        return new QSNAPScreen( displayId );
#endif
#ifndef QT_NO_QWS_LINUXFB
    if (driver == "linuxfb" || driver.isEmpty())
        return new QLinuxFbScreen(displayId);
#endif
#ifndef QT_NO_QWS_VGA16
    if (driver == "vga16" || driver.isEmpty())
        return new QVga16Screen(displayId);
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    if (driver == "transformed")
        return new QTransformedScreen(displayId);
#endif
#ifndef QT_NO_QWS_MACH64
    if (driver == "mach64")
        return new QMachScreen(displayId);
#endif
#ifndef QT_NO_QWS_VOODOO
    if (driver == "voodoo3")
        return new QVoodooScreen(displayId);
#endif
#ifndef QT_NO_QWS_MATROX
    if (driver == "matrox")
        return new QMatroxScreen(displayId);
#endif
#ifndef QT_NO_QWS_VNC
    if (driver == "vnc")
        return new QVNCScreen(displayId);
#endif
#ifndef QT_NO_QWS_SHADOWFB
    if (driver == "shadowfb")
        return new QShadowFbScreen(displayId);
#endif
#ifndef QT_NO_QWS_REPEATER
    if (driver == "repeater")
        return new QRepeaterScreen(displayId);
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT

    if (QGfxDriverFactoryInterface *factory = qobject_cast<QGfxDriverFactoryInterface*>(loader()->instance(key)))
        return factory->create(driver, displayId);

#endif
#endif
    return 0;
}

/*!
    Returns the list of keys this factory can create drivers for.

    \sa create()
*/
QStringList QGfxDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_QVFB
    if (!list.contains("QVFb"))
        list << "QVFb";
#endif
#ifndef QT_NO_QWS_SNAP
    if ( !list.contains( "snap" ) )
        list << "snap";
#endif
#ifndef QT_NO_QWS_LINUXFB
    if (!list.contains("LinuxFb"))
        list << "LinuxFb";
#endif
#ifndef QT_NO_QWS_VGA16
    if (!list.contains("VGA16"))
        list << "VGA16";
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    if (!list.contains("Transformed"))
        list << "Transformed";
#endif
#ifndef QT_NO_QWS_MACH64
    if (!list.contains("Mach64"))
        list << "Mach64";
#endif
#ifndef QT_NO_QWS_VOODOO
    if (!list.contains("Voodoo3"))
        list << "Voodoo3";
#endif
#ifndef QT_NO_QWS_MATROX
    if (!list.contains("Matrox"))
        list << "Matrox";
#endif
#ifndef QT_NO_QWS_VNC
    if (!list.contains("VNC"))
        list << "VNC";
#endif
#ifndef QT_NO_QWS_SHADOWFB
    if (!list.contains("ShadowFb"))
        list << "ShadowFb";
#endif
#ifndef QT_NO_QWS_REPEATER
     if (!list.contains("Repeater"))
        list << "Repeater";
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT
     list += loader()->keys();
#endif //QT_NO_COMPONENT
#endif //QT_MAKEDLL

    return list;
}

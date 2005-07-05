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

#include "qkbddriverfactory_qws.h"

#include "qapplication.h"
#include "qkbdtty_qws.h"
#include "qkbdusb_qws.h"
#include "qkbdsl5000_qws.h"
#include "qkbdyopy_qws.h"
#include "qkbdvr41xx_qws.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qkbddriverplugin_qws.h"

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWSKeyboardHandlerFactoryInterface_iid, QCoreApplication::libraryPaths(), "/kbddrivers"))

#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

/*!
    \class QKbdDriverFactory qkbddriverfactory.h
    \brief The QKbdDriverFactory class creates QWSKeyboardHandler objects
    for Qt/Embedded.

    \ingroup qws

    The graphics driver factory creates a QWSKeyboardHandler object
    for a given key with QKbdDriverFactory::create(key).

    The drivers are either built-in or dynamically loaded from a
    driver plugin (see \l QKbdDriverPlugin).

    This class is only available in Qt/Embedded.

    QKbdDriverFactory::keys() returns a list of valid keys.
*/

/*!
    Creates a QWSKeyboardHandler object that matches \a key for device
    \a device. This is either a built-in driver, or a driver from a
    driver plugin.

    \sa keys()
*/
QWSKeyboardHandler *QKbdDriverFactory::create(const QString& key, const QString& device)
{
    QString driver = key.toLower();
#ifdef QT_QWS_KBD_SL5000
    if (driver == "sl5000" || driver.isEmpty())
        return new QWSSL5000KeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    if (driver == "yopy" || driver.isEmpty())
        return new QWSYopyKeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KBD_VR41XX
    if (driver == "vr41xx" || driver.isEmpty())
        return new QWSVr41xxKeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KEYBOARD
# ifndef QT_NO_QWS_KBD_TTY
    if (driver =="tty" || driver.isEmpty())
        return new QWSTtyKeyboardHandler(device);
# endif
# ifndef QT_NO_QWS_KBD_USB
    if (driver == "usb")
        return new QWSUsbKeyboardHandler(device);
# endif
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
        if (QWSKeyboardHandlerFactoryInterface *factory = qobject_cast<QWSKeyboardHandlerFactoryInterface*>(loader()->instance(driver)))
            return factory->create(driver);
#endif
#endif
    return 0;
}

/*!
    Returns the list of keys this factory can create drivers for.

    \sa create()
*/
QStringList QKbdDriverFactory::keys()
{
    QStringList list;

#ifdef QT_QWS_KBD_SL5000
    if (!list.contains("SL5000"))
        list << "SL5000";
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    if (!list.contains("YOPY"))
        list << "YOPY";
#endif
#ifndef QT_NO_QWS_KBD_VR41XX
    if (!list.contains("VR41xx"))
        list << "VR41xx";
#endif
#ifndef QT_NO_QWS_KBD_TTY
    if (!list.contains("TTY"))
        list << "TTY";
#endif
#ifndef QT_NO_QWS_KBD_USB
    if (!list.contains("USB"))
        list << "USB";
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    list += loader()->keys();
#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

    return list;
}

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

#include "qkbddriverfactory_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#include "qapplication.h"
#include "qkbdtty_qws.h"
#include "qkbdusb_qws.h"
#include "qkbdum_qws.h"
#include "qkbdsl5000_qws.h"
#include "qkbdvfb_qws.h"
#include "qkbdyopy_qws.h"
#include "qkbdvr41xx_qws.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qkbddriverplugin_qws.h"

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWSKeyboardHandlerFactoryInterface_iid, QCoreApplication::libraryPaths(),
     QLatin1String("/kbddrivers")))

#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

/*!
    \class QKbdDriverFactory
    \ingroup qws

    \brief The QKbdDriverFactory class creates QWSKeyboardHandler
    objects.

    Note that this class is only available in \l {Qtopia Core}.

    The QWSKeyboardHandler class is used to implement keyboard
    drivers. QKbdDriverFactory creates a QWSKeyboardHandler object
    using the create() function and a key identifying the driver.

    The valid keys can be retrieved using the keys() function.  The
    drivers are either built-in or dynamically loaded from a driver
    plugin (see \l QKbdDriverPlugin for details).

    \sa QWSKeyboardHandler, QKbdDriverPlugin, {Qtopia Core Character Input}
*/

/*!
    Creates a QWSKeyboardHandler object for the driver specified by
    the given \a key, for the given \a device. The driver is either a
    built-in driver, or a driver plugin. Keys are case-insensitive.

    \sa keys()
*/
QWSKeyboardHandler *QKbdDriverFactory::create(const QString& key, const QString& device)
{
    QString driver = key.toLower();
    Q_UNUSED(device);
#ifndef QT_NO_QWS_KBD_SL5000
    if (driver == QLatin1String("sl5000") || driver.isEmpty())
        return new QWSSL5000KeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    if (driver == QLatin1String("yopy") || driver.isEmpty())
        return new QWSYopyKeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KBD_VR41XX
    if (driver == QLatin1String("vr41xx") || driver.isEmpty())
        return new QWSVr41xxKeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KEYBOARD
# ifndef QT_NO_QWS_KBD_TTY
    if (driver == QLatin1String("tty") || driver.isEmpty())
        return new QWSTtyKeyboardHandler(device);
# endif
# ifndef QT_NO_QWS_KBD_USB
    if (driver == QLatin1String("usb"))
        return new QWSUsbKeyboardHandler(device);
# endif
# ifndef QT_NO_QWS_KBD_UM
    if (driver == QLatin1String("um") || driver == QLatin1String("qvfbkeyboard"))
        return new QWSUmKeyboardHandler(device);
# endif
# ifndef QT_NO_QWS_KBD_QVFB
    if (driver == QLatin1String("qvfbkbd")
        || driver == QLatin1String("qvfbkeyboard")
        || driver == QLatin1String("qvfb"))
        return new QVFbKeyboardHandler(device);
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
    Returns the list of valid keys, i.e. the keys this factory can
    create drivers for.

    \l {Qtopia Core} currently supports the following drivers by
    default: \c SL5000, \c Yopy, \c VR41xx, \c TTY and \c USB.

    \sa create()
*/
QStringList QKbdDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_KBD_SL5000
    list << QLatin1String("SL5000");
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    list << QLatin1String("YOPY");
#endif
#ifndef QT_NO_QWS_KBD_VR41XX
    list << QLatin1String("VR41xx");
#endif
#ifndef QT_NO_QWS_KBD_TTY
    list << QLatin1String("TTY");
#endif
#ifndef QT_NO_QWS_KBD_USB
    list << QLatin1String("USB");
#endif
#ifndef QT_NO_QWS_KBD_UM
    list << QLatin1String("UM");
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

#endif // QT_NO_QWS_KEYBOARD

/****************************************************************************
**
** Implementation of QKbdDriverFactory class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qkbddriverinterface_p.h" // up here for GCC 2.7.* compatibility
#include "qkbddriverfactory_qws.h"

#include "qapplication.h"
#include "qkbdtty_qws.h"
#include "qkbdusb_qws.h"
#include "qkbdsl5000_qws.h"
#include "qkbdyopy_qws.h"
#include "qkbdvr41xx_qws.h"
#include <stdlib.h>

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#include <private/qpluginmanager_p.h>
#ifndef QT_NO_COMPONENT
class QKbdDriverFactoryPrivate : public QObject
{
public:
    QKbdDriverFactoryPrivate();
    ~QKbdDriverFactoryPrivate();

    static QPluginManager<QKbdDriverInterface> *manager;
};

static QKbdDriverFactoryPrivate *instance = 0;
QPluginManager<QKbdDriverInterface> *QKbdDriverFactoryPrivate::manager = 0;

QKbdDriverFactoryPrivate::QKbdDriverFactoryPrivate()
: QObject( qApp )
{
    manager = new QPluginManager<QKbdDriverInterface>( IID_QKbdDriver, QApplication::libraryPaths(), "/kbddrivers", FALSE );
}

QKbdDriverFactoryPrivate::~QKbdDriverFactoryPrivate()
{
    delete manager;
    manager = 0;

    instance = 0;
}

#endif //QT_NO_COMPONENT
#endif //QT_MAKEDLL

/*!
    \class QKbdDriverFactory qkbddriverfactory.h
    \brief The QKbdDriverFactory class creates QWSKeyboardHandler objects
    for Qt/Embedded.

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
QWSKeyboardHandler *QKbdDriverFactory::create( const QString& key, const QString& device )
{
    QString driver = key.lower();
#ifdef Q_OS_QNX6
    if ( driver == "qnx" || driver.isEmpty() )
	return new QWSQnxKeyboardHandler( device );
#endif
#ifndef QT_NO_QWS_KBD_SL5000
    if ( driver == "sl5000" || driver.isEmpty() )
	return new QWSSL5000KeyboardHandler( device );
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    if ( driver == "yopy" || driver.isEmpty() )
	return new QWSYopyKeyboardHandler( device );
#endif
#ifndef QT_NO_QWS_KBD_VR41
    if ( driver == "vr41xx" || driver.isEmpty() )
	return new QWSVr41xxKeyboardHandler( device );
#endif
#ifndef QT_NO_QWS_KEYBOARD
# ifndef QT_NO_QWS_KBD_TTY
    if ( driver =="tty" || driver.isEmpty() )
	return new QWSTtyKeyboardHandler( device );
# endif
# ifndef QT_NO_QWS_KBD_USB
    if ( driver == "usb" )
	return new QWSUsbKeyboardHandler( device );
# endif
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QKbdDriverFactoryPrivate;

    QInterfacePtr<QKbdDriverInterface> iface;
    QKbdDriverFactoryPrivate::manager->queryInterface( driver, &iface );

    if ( iface )
	return iface->create( driver, driver );
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

#ifdef Q_OS_QNX6
    if ( !list.contains( "Qnx" ) )
	list << "Qnx";
#endif
#ifndef QT_NO_QWS_KBD_SL5000
    if ( !list.contains( "SL5000" ) )
	list << "SL5000";
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    if ( !list.contains( "YOPY" ) )
	list << "YOPY";
#endif
#ifndef QT_NO_QWS_KBD_VR41
    if ( !list.contains( "VR41xx" ) )
	list << "VR41xx";
#endif
#ifndef QT_NO_QWS_KBD_TTY
    if ( !list.contains( "TTY" ) )
	list << "TTY";
#endif
#ifndef QT_NO_QWS_KBD_USB
    if ( !list.contains( "USB" ) )
	list << "USB";
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QKbdDriverFactoryPrivate;

    list += QKbdDriverFactoryPrivate::manager->featureList();
#endif //QT_NO_COMPONENT
#endif //QT_MAKEDLL

    return list;
}

/****************************************************************************
**
** Implementation of QKbdDriverFactory class
**
** Created : 20020218
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qkbddriverinterface_p.h" // up here for GCC 2.7.* compatibility
#include "qkbddriverfactory_qws.h"

#include "qapplication.h"
#include "qkbdtty_qws.h"
#include "qkbdusb_qws.h"
#include "qkbdsl5000_qws.h"
#include "qkbdyopy_qws.h"
#include <stdlib.h>

#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)) || defined(QT_MAKEDLL)
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
  \brief The QKbdDriverFactory class creates QWSKeyboardHandler objects.

  The graphics driver factory creates a QWSKeyboardHandler object for a given
  key with QKbdDriverFactory::create(key).

  The drivers are either built-in or dynamically loaded from a driver
  plugin (see \l QKbdDriverPlugin).

  QKbdDriverFactory::keys() returns a list of valid keys.

*/

/*!  Creates a QWSKeyboardHandler object that matches \a key. This is either a
  built-in driver, or a driver from a driver plugin.

  \sa keys()
*/
QWSKeyboardHandler *QKbdDriverFactory::create( const QString& key )
{
    QString driver = key.lower();
#ifdef Q_OS_QNX6
    if ( driver == "qnx" || driver.isEmpty() )
	return new QWSQnxKeyboardHandler();
#endif
#ifdef QT_QWS_SHARP
    if ( driver == "sl5000" || driver.isEmpty() )
	return new QWSSL5000KeyboardHandler();
#endif
#ifdef QT_QWS_YOPY
    if ( driver == "yopy" || driver.isEmpty() )
	return new QWSYopyKeyboardHandler();
#endif
#ifndef QT_NO_QWS_KBDTTY
    if ( driver =="tty" || driver.isEmpty() )
	return new QWSTtyKeyboardHandler();
#endif
#ifndef QT_NO_QWS_KBDUSB
    if ( driver == "usb" )
	return new QWSUsbKeyboardHandler();
#endif

#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QKbdDriverFactoryPrivate;

    QInterfacePtr<QKbdDriverInterface> iface;
    QKbdDriverFactoryPrivate::manager->queryInterface( driver, &iface );

    if ( iface )
	return iface->create( driver );
#endif
#endif
    return 0;
}

#ifndef QT_NO_STRINGLIST
/*!
  Returns the list of keys  this factory can create
  drivers for.

  \sa create()
*/
QStringList QKbdDriverFactory::keys()
{
    QStringList list;

#ifdef Q_OS_QNX6
    if ( !list.contains( "Qnx" ) )
	list << "Qnx";
#endif
#ifdef QT_QWS_SHARP
    if ( !list.contains( "SL5000" ) )
	list << "SL5000";
#endif
#ifdef QT_QWS_YOPY
    if ( !list.contains( "SL5000" ) )
	list << "SL5000";
#endif
#ifndef QT_NO_QWS_KBDTTY
    if ( !list.contains( "TTY" ) )
	list << "TTY";
#endif
#ifndef QT_NO_QWS_KBDUSB
    if ( !list.contains( "USB" ) )
	list << "USB";
#endif

#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QKbdDriverFactoryPrivate;

    list += QKbdDriverFactoryPrivate::manager->featureList();
#endif //QT_NO_COMPONENT
#endif //QT_MAKEDLL

    return list;
}
#endif

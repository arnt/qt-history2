/****************************************************************************
**
** Implementation of QGfxDriverFactory class
**
** Created : 20020212
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

#include "qgfxdriverinterface_p.h" // up here for GCC 2.7.* compatibility
#include "qgfxdriverfactory_qws.h"

#include "qapplication.h"
#include "qgfxlinuxfb_qws.h"
#include "qgfxtransformed_qws.h"
#include "qgfxmach64_qws.h"
#include "qgfxvoodoo_qws.h"
#include "qgfxmatrox_qws.h"
#include "qgfxvfb_qws.h"
#include "qgfxvnc_qws.h"
#include "qgfxvga16_qws.h"
#include "qgfxshadowfb_qws.h"
#include <stdlib.h>

#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)) || defined(QT_MAKEDLL)
#include <private/qpluginmanager_p.h>
#ifndef QT_NO_COMPONENT
class QGfxDriverFactoryPrivate : public QObject
{
public:
    QGfxDriverFactoryPrivate();
    ~QGfxDriverFactoryPrivate();

    static QPluginManager<QGfxDriverInterface> *manager;
};

static QGfxDriverFactoryPrivate *instance = 0;
QPluginManager<QGfxDriverInterface> *QGfxDriverFactoryPrivate::manager = 0;

QGfxDriverFactoryPrivate::QGfxDriverFactoryPrivate()
: QObject( qApp )
{
    manager = new QPluginManager<QGfxDriverInterface>( IID_QGfxDriver, QApplication::libraryPaths(), "/gfxdrivers", FALSE );
}

QGfxDriverFactoryPrivate::~QGfxDriverFactoryPrivate()
{
    delete manager;
    manager = 0;

    instance = 0;
}

#endif //QT_NO_COMPONENT
#endif //QT_MAKEDLL

/*!
  \class QGfxDriverFactory qgfxdriverfactory.h
  \brief The QGfxDriverFactory class creates QScreen objects.

  The graphics driver factory creates a QScreen object for a given key with
  QGfxDriverFactory::create(key).

  The drivers are either built-in or dynamically loaded from a driver
  plugin (see \l QGfxDriverPlugin).

  QGfxDriverFactory::keys() returns a list of valid keys. Qt currently
  ships with "LinuxFb"

*/

/*!  Creates a QScreen object that matches \a key. This is either a
  built-in driver, or a driver from a driver plugin.

  \sa keys()
*/
QScreen *QGfxDriverFactory::create( const QString& key, int displayId )
{
    QString driver = key.lower();
#ifndef QT_NO_QWS_VFB
    if ( driver == "qvfb" || driver.isEmpty() )
	return new QVFbScreen( displayId );
#endif
#ifndef QT_NO_QWS_LINUXFB
    if ( driver == "linuxfb" || driver.isEmpty() )
        return new QLinuxFbScreen( displayId );
#endif
#ifndef QT_NO_QWS_VGA16
    if ( driver == "vga16" || driver.isEmpty() )
	return new QVga16Screen( displayId );
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    if ( driver == "transformed" )
        return new QTransformedScreen( displayId );
#endif
#ifndef QT_NO_QWS_MACH64
    if ( driver == "mach64" )
        return new QMachScreen( displayId );
#endif
#ifndef QT_NO_QWS_VOODOO3
    if ( driver == "voodoo3" )
        return new QVoodooScreen( displayId );
#endif
#ifndef QT_NO_QWS_MATROX
    if ( driver == "matrox" )
        return new QMatroxScreen( displayId );
#endif
#ifndef QT_NO_QWS_VNC
    if ( driver == "vnc" )
	return new QVNCScreen( displayId );
#endif
#ifndef QT_NO_QWS_SHADOWFB
    if ( driver == "shadowfb" )
	return new QShadowFbScreen( displayId );
#endif

#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QGfxDriverFactoryPrivate;

    QInterfacePtr<QGfxDriverInterface> iface;
    QGfxDriverFactoryPrivate::manager->queryInterface( driver, &iface );

    if ( iface )
	return iface->create( driver, displayId );
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
QStringList QGfxDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_VFB
    if ( !list.contains( "QVFb" ) )
	list << "QVFb";
#endif
#ifndef QT_NO_QWS_LINUXFB
    if ( !list.contains( "LinuxFb" ) )
	list << "LinuxFb";
#endif
#ifndef QT_NO_QWS_VGA16
    if ( !list.contains( "VGA16" ) )
	list << "VGA16";
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    if ( !list.contains( "Transformed" ) )
	list << "Transformed";
#endif
#ifndef QT_NO_QWS_MACH64
    if ( !list.contains( "Mach64" ) )
	list << "Mach64";
#endif
#ifndef QT_NO_QWS_VOODOO3
    if ( !list.contains( "Voodoo3" ) )
	list << "Voodoo3";
#endif
#ifndef QT_NO_QWS_MATROX
    if ( !list.contains( "Matrox" ) )
	list << "Matrox";
#endif
#ifndef QT_NO_QWS_VNC
    if ( !list.contains( "VNC" ) )
	list << "VNC";
#endif
#ifndef QT_NO_QWS_SHADOWFB
    if ( !list.contains( "ShadowFb" ) )
	list << "ShadowFb";
#endif

#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)) || defined(QT_MAKEDLL)
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QGfxDriverFactoryPrivate;

    list += QGfxDriverFactoryPrivate::manager->featureList();
#endif //QT_NO_COMPONENT
#endif //QT_MAKEDLL

    return list;
}
#endif

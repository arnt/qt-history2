/****************************************************************************
** $Id: $
**
** Implementation of the QComponentFactory class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "qcomponentfactory.h"

#ifndef QT_NO_COMPONENT
#include "qsettings.h"
#include "qlibrary.h"
#include "qdir.h"

/*!
  \class QComponentFactory qcomponentfactory.h
  \brief The QComponentFactory class provides static functions to create and register components.

  \ingroup componentmodel

  The component factory provides static convenience functions that can be used both by 
  applications to instantiate components, and by component servers to register components.

  The createInstance() function provides a pointer to an interface implemented in a specific 
  component.

  Use registerServer() to load a component server and register its components, and unregisterServer()
  to unregsiter the components. The component exported by the component server has to implement the
  QComponentRegistrationInterface. registerComponent() and unregisterComponent() register and unregister
  single components from the system component registry, and should be used by implementations of the 
  QComponentServerInterface.

  \sa QComponentRegistrationInterface QComponentFactoryInterface
*/

/*!
  Searches for the component identifier \a cid in the system component registry,
  loads the corresponding component server and queries for the interface \a iid. 
  \a iface is set to the resulting interface pointer. \a cid can either be the
  UUID or the human-readable name of the component.

  The parameter \a outer is a pointer to the outer interface used
  for containment and aggregation and is propagated to the \link
  QComponentFactoryInterface::createInstance() createInstance \endlink
  implementation of the QComponentFactoryInterface in the component server if 
  provided.

  The function returns QS_OK if the interface was successfully instantiated, QE_NOINTERFACE if
  the component does not provide an interface \a iid, or QE_NOCOMPONENT if there was 
  an error loading the component.

  Example:
  \code
  MyInterface *iface;
  if ( QComponentFactory::createInstance( IID_MyInterface, CID_MyComponent, (QUnknownInterface**)&iface ) == QS_OK )
      ...
      iface->release();
  }
  \endcode
*/

QRESULT QComponentFactory::createInstance( const QString &cid, const QUuid &iid, QUnknownInterface** iface, QUnknownInterface *outer )
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    bool ok = FALSE;
    QString cidStr = cid;
    QUuid uuid( cidStr ); // try to parse, and resolve CLSID if necessary
    if ( uuid.isNull() ) {
	uuid = settings.readEntry( "/" + cid + "/CLSID/Default", QString::null, &ok );
	cidStr = uuid.toString();
    }

    if ( cidStr.isEmpty() )
	return QE_NOCOMPONENT;

    QString file = settings.readEntry( "/CLSID/" + cidStr + "/InprocServer32/Default", QString::null, &ok );
    if ( !ok )
	return QE_NOCOMPONENT;

    QLibrary library( file, QLibrary::Manual );

    QComponentFactoryInterface *cfIface =0;
    library.queryInterface( IID_QComponentFactory, (QUnknownInterface**)&cfIface );
    if ( cfIface ) {
	QRESULT res = cfIface->createInstance( uuid, iid, iface, outer );
	cfIface->release();
	return res;
    }
    return library.queryInterface( iid, iface );
}

/*!
  Loads the shared library \a filename and queries for a
  QComponentRegistrationInterface. If the library implements this interface,
  the \link QComponentRegistrationInterface::registerComponents()
  registerComponents \endlink function is called.

  Returns TRUE if the interface is found and successfully registered,
  otherwise returns FALSE.
*/
QRESULT QComponentFactory::registerServer( const QString &filename )
{
    QLibrary lib( filename, QLibrary::Immediately );
    QComponentRegistrationInterface *iface = 0;
    QRESULT res = lib.queryInterface( IID_QComponentRegistration, (QUnknownInterface**)&iface );
    if ( res != QS_OK )
	return res;
    QDir dir( filename );
    bool ok = iface->registerComponents( dir.absPath() );
    iface->release();
    return ok ? QS_OK : QS_FALSE;
}

/*!
  Loads the shared library \a filename and queries for a
  QComponentRegistrationInterface. If the library implements this interface,
  the \link QComponentRegistrationInterface::unregisterComponents()
  unregisterComponents \endlink function is called.

  Returns TRUE if the interface is found and successfully unregistered,
  otherwise returns FALSE.
*/
QRESULT QComponentFactory::unregisterServer( const QString &filename )
{
    QLibrary lib( filename, QLibrary::Immediately );
    QComponentRegistrationInterface *iface = 0;
    QRESULT res = lib.queryInterface( IID_QComponentRegistration, (QUnknownInterface**)&iface );
    if ( res != QS_OK )
	return res;
    bool ok = iface->unregisterComponents();
    iface->release();
    return ok ? QS_OK : QS_FALSE;
}

/*!
  Registers the component with id \a cid in the system component registry and
  returns TRUE if the component was registerd successfully, otherwise returns
  FALSE. The component is registered with an optional \a name and \a description 
  and is provided by the server at \a filepath. This function does nothing if a 
  component with an identical \a cid is already registered on the system.

  A component that has been registered with a \a name can be created using both the
  \a cid and the \a name value using createInstance().

  Call this function for each component in an implementation of
  \link QComponentRegistrationInterface::registerComponents() registerComponents \endlink.

  \sa unregisterComponent(), registerServer(), createInstance()
*/
bool QComponentFactory::registerComponent( const QUuid &cid, const QString &filepath, const QString &name, const QString &description )
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    bool ok = FALSE;

    QString cidStr = cid.toString().upper();
    QString old = settings.readEntry( "/CLSID/" + cidStr + "/InprocServer32/Default", QString::null, &ok );
    ok = !ok && settings.writeEntry( "/CLSID/" + cidStr + "/InprocServer32/Default", filepath );
    if ( !!description )
	ok = ok && settings.writeEntry( "/CLSID/" + cidStr + "/Default", description );

    if ( ok && !name.isEmpty() ) {
	settings.writeEntry( "/CLSID/" + cidStr + "/ProgID/Default", name );
	ok = settings.writeEntry( "/" + name + "/CLSID/Default", cidStr );
	if ( !!description )
	    ok = ok && settings.writeEntry( "/" + name + "/Default", description );
    }

    return ok;
}

/*!
  Unregisters the component with id \a cid from the system component registry and returns
  TRUE if the component was unregistered successfully, otherwise returns FALSE. \a cid can 
  either be the UUID or the human-readable name of the component.

  Call this function for each component in an implementation of
  \link QComponentRegistrationInterface::unregisterComponents() unregisterComponents \endlink.

  \sa registerComponent(), unregisterServer()
*/
bool QComponentFactory::unregisterComponent( const QString &cid )
{
    QSettings settings;
    bool ok = FALSE;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );

    QString cidStr = cid;
    QString name;
    QUuid uuid( cidStr ); // try to parse, and resolve CLSID if necessary
    if ( uuid.isNull() ) {
	name = cid;
	uuid = settings.readEntry( "/" + name + "/CLSID/Default", QString::null, &ok );
	ok = ok && settings.removeEntry( "/" + name + "/CLSID/Default" );
	cidStr = uuid.toString();
    } else {
	name = settings.readEntry( "/CLSID/" + cidStr + "/ProgID/Default", QString::null, &ok );
	ok = settings.removeEntry( "/" + name + "/CLSID/Default" );
	ok = ok && settings.removeEntry( "/" + name + "/Default" );
    }

    if ( cidStr.isEmpty() )
	return FALSE;

    settings.readEntry( "/CLSID/" + cidStr + "/InprocServer32/Default", QString::null, &ok );
    ok = settings.removeEntry( "/CLSID/" + cidStr + "/ProgID/Default" );
    ok = ok && settings.removeEntry( "/CLSID/" + cidStr + "/InprocServer32/Default" );
    ok = ok && settings.removeEntry( "/CLSID/" + cidStr + "/Default" );

    return ok;
}

#endif // QT_NO_COMPONENT

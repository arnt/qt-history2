/****************************************************************************
** $Id: $
**
** Implementation of the QAxFactory classes
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qaxfactory.h"

#include <qsettings.h>

/*!
    \class QAxFactoryInterface qactiveqt.h
    \brief The QAxFactoryInterface class is an interface for the creation of ActiveX components.
    \internal
    \module QAxServer
    \extension ActiveQt

    Implement this interface once in your ActiveX server to provide information about the components
    this server can create. The interface inherits the QFeatureListInterface and works key-based. A
    key in this interface is the class name of the ActiveX object.

    To instantiate and export your implementation of the factory interface, use the Q_EXPORT_COMPONENT
    and Q_CREATE_INSTANCE macros:

    \code
    class MyFactory : public QAxFactoryInterface
    {
	...
    };

    Q_EXPORT_COMPONENT()
    {
	Q_CREATE_INSTANCE( MyFactory )
    }
    \endcode

    The QAxFactory class provide a convenient implementation of this interface.
*/

/*!
    \class QAxFactory qaxbindable.h
    \brief The QAxFactory class defines a factory for the creation of ActiveX components.

    \module QAxServer
    \extension ActiveQt
    \keyword QAXFACTORY_DEFAULT
    \keyword QAXFACTORY_EXPORT

    Implement this factory once in your ActiveX server to provide information about the components
    this server can create. If your server supports only a single ActiveX control, you can use the 
    default factory implementation instead of implementing the factory on your own. Use the 
    \c QAXFACTORY_DEFAULT macro in any implementation file (e.g. main.cpp) to instantiate and export 
    the default factory:

    \code
    #include <qapplication.h>
    #include <qaxfactory.h>

    #include "theactivex.h"

    QAXFACTORY_DEFAULT( TheActiveX,				  // widget class
                        "{01234567-89AB-CDEF-0123-456789ABCDEF}", // class ID
			"{01234567-89AB-CDEF-0123-456789ABCDEF}", // interface ID
			"{01234567-89AB-CDEF-0123-456789ABCDEF}", // event interface ID
			"{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
			"{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
		      )
    \endcode

    If you implement your own factory reimplement the pure virtual functions to provide the unique identifiers
    for the ActiveX controls, and use the QAXFACTORY_EXPORT macro to instantiate and export it: 

    \code
    QStringList ActiveQtFactory::featureList() const
    {
	QStringList list;
	list << "ActiveX1";
	list << "ActiveX2";
	...
	return list;
    }

    QWidget *ActiveQtFactory::create( const QString &key, QWidget *parent, const char *name )
    {
	if ( key == "ActiveX1" )
	    return new ActiveX1( parent, name );
        if ( key == "ActiveX2" )
	    return new ActiveX2( parent, name );
	...
	return 0;
    }
    
    QMetaObject *ActiveQtFactory::metaObject( const QString &key ) const
    {
        if ( key == "ActiveX1" )
	    return ActiveX1::staticMetaObject();
	...
	return 0;
    }

    QUuid ActiveQtFactory::classID( const QString &key ) const
    {
        if ( key == "ActiveX1" )
	    return "{01234567-89AB-CDEF-0123-456789ABCDEF}";
	...
	return QUuid();
    }

    QUuid ActiveQtFactory::interfaceID( const QString &key ) const
    {
	if ( key == "ActiveX1" )
	    return "{01234567-89AB-CDEF-0123-456789ABCDEF}";
	...
	return QUuid();
    }

    QUuid ActiveQtFactory::eventsID( const QString &key ) const
    {
	if ( key == "ActiveX1" )
	    return "{01234567-89AB-CDEF-0123-456789ABCDEF}";
	...
	return QUuid();
    }

    QAXFACTORY_EXPORT( MyFactory,			         // factory class
		       "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
		       "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
		     )
    \endcode

    Every ActiveX server application can only instantiate and export a single QAxFactory
    implementation.

    A factory can additionally reimplement the registerClass() and unregisterClass()
    functions to set additional flags for an ActiveX control in the registry. To limit
    the number of methods or properties a widget class exposes from its parent classes
    reimplement exposeParentClass().
*/

/*!
    Constructs a QAxFactory object that returns \a libid and \a appid
    in the implementation of the respective interface functions.
*/

QAxFactory::QAxFactory( const QUuid &libid, const QUuid &appid )
    : typelib( libid ), app( appid )
{
}

/*!
    \internal
*/
QRESULT QAxFactory::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QAxFactory )
	*iface = this;
    else
	return QE_NOINTERFACE;
    addRef();
    return QS_OK;
}

/*!
    \fn QUuid QAxFactory::typeLibID() const

    Reimplement this function to return the type library identifier for this ActiveX server.
*/
QUuid QAxFactory::typeLibID() const
{
    return typelib;
}

/*!
    \fn QUuid QAxFactory::appID() const

    Reimplement this function to return the application identifier for this ActiveX server.
*/
QUuid QAxFactory::appID() const
{
    return app;
}

/*!
    \fn QStringList QAxFactory::featureList() const

    Reimplement this function to return a list of class names of the widgets supported by this factory.
*/

/*!
    \fn QWidget *QAxFactory::create( const QString &key, QWidget *parent = 0, const char *name = 0 )

    Reimplement this function to return a new widget for each \a key returned by the featureList implementation. 
    Propagate \a parent and \a name to the QWidget constructor. 
    Return 0 if this factory doesn't support the value of \a key.
*/

/*!
    \fn QMetaObject *QAxFactory::metaObject( const QString &key ) const

    Reimplement this function to return the QMetaObject for each \a key returned by the featureList implementation. 
    Use the QObject::staticMetaObject() function to get the QMetaObject for a class. The class implementing the 
    ActiveX control has to use the Q_OBJECT macro to generate meta object information.
    Return 0 if this factory doesn't support the value of \a key.
*/

/*!
    \fn QUuid QAxFactory::classID( const QString &key ) const

    Reimplement this function to return the class identifier for each \a key returned by the featureList implementation, 
    or an empty QUuid if this factory doesn't support the value of \a key.
*/

/*!
    \fn QUuid QAxFactory::interfaceID( const QString &key ) const

    Reimplement this function to return the interface identifier for each \a key returned by the featureList implementation, 
    or an empty QUuid if this factory doesn't support the value of \a key.
*/

/*!
    \fn QUuid QAxFactory::eventsID( const QString &key ) const

    Reimplement this function to return the identifier of the event interface for each \a key returned by the featureList 
    implementation, or an empty QUuid if this factory doesn't support the value of \a key.
*/

/*!
    Registers the class factory in the system registry, and returns TRUE if registration
    succeeds. Otherwise returns FALSE.

    This function calls registerClass for each key returned by the featureList implementation.
*/
bool QAxFactory::registerFactory() const
{
    QStringList keys = featureList();
    for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	if ( !registerClass( *key ) ) 
	    return FALSE;
    }

    return TRUE;
}

/*!
    Unregisters the class factory from the system registry, and returns TRUE if the
    unregistration succeeds. Otherwise returns FALSE.

    This function calls unregisterClass for each key returned by the featureList implementation.
*/
bool QAxFactory::unregisterFactory() const
{
    QStringList keys = featureList();
    for ( QStringList::Iterator key = keys.begin(); key != keys.end(); ++key ) {
	if ( !unregisterClass( *key ) ) 
	    return FALSE;
    }

    return TRUE;
}

/*!
    Registers the class \a key in the system registry, and returns TRUE if the
    registration succeeds. Otherwise returns FALSE.

    Reimplementations of this function should call this implementation first,
    and add additional registry values afterwards.

    If you reimplement this function you will also have to reimplement 
    unregisterClass to remove the additional registry values.
*/    
bool QAxFactory::registerClass( const QString &key ) const
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );

    const QString appId = appID().toString().upper();
    const QString libId = typeLibID().toString().upper();
    const QString classId = classID(key).toString().upper();
    const QString eventId = eventsID(key).toString().upper();
    const QString ifaceId = interfaceID(key).toString().upper();
    const QString className = key;
    QString module = settings.readEntry( "/AppID/" + appId + "/." );
    QString file = settings.readEntry( "/TypeLib/" + libId + "/1.0/0/win32/." );

    settings.writeEntry( "/" + module + "." + className + ".1/.", className + " Class" );
    settings.writeEntry( "/" + module + "." + className + ".1/CLSID/.", classId );
    settings.writeEntry( "/" + module + "." + className + ".1/Insertable/.", QString::null );
    
    settings.writeEntry( "/" + module + "." + className + "/.", className + " Class" );
    settings.writeEntry( "/" + module + "." + className + "/CLSID/.", classId );
    settings.writeEntry( "/" + module + "." + className + "/CurVer/.", module + "." + className + ".1" );
    
    settings.writeEntry( "/CLSID/" + classId + "/.", className + " Class" );
    settings.writeEntry( "/CLSID/" + classId + "/AppID", appId );
    settings.writeEntry( "/CLSID/" + classId + "/Control/.", QString::null );
    settings.writeEntry( "/CLSID/" + classId + "/Insertable/.", QString::null );
    if ( file.right( 3 ).lower() == "dll" )
	settings.writeEntry( "/CLSID/" + classId + "/InProcServer32/.", file );
    else
	settings.writeEntry( "/CLSID/" + classId + "/LocalServer32/.", file + " -activex" );
    settings.writeEntry( "/CLSID/" + classId + "/MiscStatus/.", "0" );
    settings.writeEntry( "/CLSID/" + classId + "/MiscStatus/1/.", "131473" );
    settings.writeEntry( "/CLSID/" + classId + "/Programmable/.", QString::null );
    settings.writeEntry( "/CLSID/" + classId + "/ToolboxBitmap32/.", file + ", 101" );
    settings.writeEntry( "/CLSID/" + classId + "/TypeLib/.", libId );
    settings.writeEntry( "/CLSID/" + classId + "/Version/.", "1.0" );
    settings.writeEntry( "/CLSID/" + classId + "/VersionIndependentProgID/.", module + "." + className );
    settings.writeEntry( "/CLSID/" + classId + "/ProgID/.", module + "." + className + ".1" );
    settings.writeEntry( "/CLSID/" + classId + "/Implemented Categories/.", QString::null );
    //### TODO: write some list of categories
    
    settings.writeEntry( "/Interface/" + ifaceId + "/.", "I" + className );
    settings.writeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid/.", "{00020424-0000-0000-C000-000000000046}" );
    settings.writeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid32/.", "{00020424-0000-0000-C000-000000000046}" );
    settings.writeEntry( "/Interface/" + ifaceId + "/TypeLib/.", libId );
    settings.writeEntry( "/Interface/" + ifaceId + "/TypeLib/Version", "1.0" );
    
    settings.writeEntry( "/Interface/" + eventId + "/.", "I" + className + "Events" );
    settings.writeEntry( "/Interface/" + eventId + "/ProxyStubClsid/.", "{00020420-0000-0000-C000-000000000046}" );
    settings.writeEntry( "/Interface/" + eventId + "/ProxyStubClsid32/.", "{00020420-0000-0000-C000-000000000046}" );
    settings.writeEntry( "/Interface/" + eventId + "/TypeLib/.", libId );
    settings.writeEntry( "/Interface/" + eventId + "/TypeLib/Version", "1.0" );

    return TRUE;
}

/*!
    Unegisters the class \a key from the system registry, and returns TRUE if the
    unregistration succeeds. Otherwise returns FALSE.

    Reimplementations of this function should call this implementation first,
    and remove additional registry values afterwards.

    \sa registerClass()
*/
bool QAxFactory::unregisterClass( const QString &key ) const
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Classes" );

    const QString appId = appID().toString().upper();
    const QString classId = classID(key).toString().upper();
    const QString eventId = eventsID(key).toString().upper();
    const QString ifaceId = interfaceID(key).toString().upper();
    const QString className = key;
    QString module = settings.readEntry( "/AppID/" + appId + "/." );
    if ( module.isEmpty() )
	return TRUE;

    settings.removeEntry( "/" + module + "." + className + ".1/CLSID/." );
    settings.removeEntry( "/" + module + "." + className + ".1/Insertable/." );
    settings.removeEntry( "/" + module + "." + className + ".1/." );
    
    settings.removeEntry( "/" + module + "." + className + "/CLSID/." );
    settings.removeEntry( "/" + module + "." + className + "/CurVer/." );
    settings.removeEntry( "/" + module + "." + className + "/." );
    
    settings.removeEntry( "/CLSID/" + classId + "/AppID" );
    settings.removeEntry( "/CLSID/" + classId + "/Control/." );
    settings.removeEntry( "/CLSID/" + classId + "/Insertable/." );
    settings.removeEntry( "/CLSID/" + classId + "/InProcServer32/." );
    settings.removeEntry( "/CLSID/" + classId + "/LocalServer32/." );
    settings.removeEntry( "/CLSID/" + classId + "/MiscStatus/1/." );
    settings.removeEntry( "/CLSID/" + classId + "/MiscStatus/." );	    
    settings.removeEntry( "/CLSID/" + classId + "/Programmable/." );
    settings.removeEntry( "/CLSID/" + classId + "/ToolboxBitmap32/." );
    settings.removeEntry( "/CLSID/" + classId + "/TypeLib/." );
    settings.removeEntry( "/CLSID/" + classId + "/Version/." );
    settings.removeEntry( "/CLSID/" + classId + "/VersionIndependentProgID/." );
    settings.removeEntry( "/CLSID/" + classId + "/ProgID/." );
    //### TODO: remove some list of categories
    settings.removeEntry( "/CLSID/" + classId + "/Implemented Categories/." );
    settings.removeEntry( "/CLSID/" + classId + "/." );
    
    settings.removeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid/." );
    settings.removeEntry( "/Interface/" + ifaceId + "/ProxyStubClsid32/." );
    settings.removeEntry( "/Interface/" + ifaceId + "/TypeLib/Version" );
    settings.removeEntry( "/Interface/" + ifaceId + "/TypeLib/." );
    settings.removeEntry( "/Interface/" + ifaceId + "/." );
    
    settings.removeEntry( "/Interface/" + eventId + "/ProxyStubClsid/." );
    settings.removeEntry( "/Interface/" + eventId + "/ProxyStubClsid32/." );
    settings.removeEntry( "/Interface/" + eventId + "/TypeLib/Version" );
    settings.removeEntry( "/Interface/" + eventId + "/TypeLib/." );
    settings.removeEntry( "/Interface/" + eventId + "/." );

    return TRUE;
}

/*!
    Reimplement this function to return the name of the super class of \a key up to 
    which methods and properties should be exposed by the ActiveX control.

    The default implementation returns "QWidget". All methods and properties of all super classes
    including QWidget will be exposed.

    To expose only methods and properties of the class itself, reimplement this function to return
    \a key.
*/
QString QAxFactory::exposeToSuperClass( const QString &key ) const
{
    return "QWidget";
}

extern bool is_server;

/*!
    Returns TRUE if the application has been started as an ActiveX server,
    otherwise returns FALSE.

    \code
    int main( int argc, char**argv ) 
    {
	QApplication app( argc, argv );

	if ( !QAxFactory::isServer() ) {
	    // initialize for stand-alone execution
	}

	return app.exec() // standard event processing
    }

    \endcode
*/

bool QAxFactory::isServer()
{
    return is_server;
}

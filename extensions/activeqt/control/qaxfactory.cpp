/****************************************************************************
**
** Implementation of the QAxFactory classes.
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaxfactory.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qwidget.h>
#include <qt_windows.h>


/*!
    \class QAxFactoryInterface qaxfactory.h
    \brief The QAxFactoryInterface class is an interface for the creation of COM components.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \internal

    \module QAxServer
    \extension ActiveQt

    Implement this interface once in your ActiveX server to provide
    information about the components the server can create. The
    interface inherits the QFeatureListInterface and is key-based.
    A key in this interface is the class name of the ActiveX object.

    To instantiate and export your implementation of the factory
    interface, use the \c Q_EXPORT_COMPONENT and \c Q_CREATE_INSTANCE
    macros:

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

    The QAxFactory class provides a convenient implementation of this
    interface.
*/

/*!
    \class QAxFactory qaxfactory.h
    \brief The QAxFactory class defines a factory for the creation of COM components.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module QAxServer
    \extension ActiveQt
    \keyword QAXFACTORY_DEFAULT
    \keyword QAXFACTORY_EXPORT
    \keyword QAXFACTORY_BEGIN
    \keyword QAXFACTORY_END
    \keyword QAXCLASS

    Implement this factory once in your ActiveX server to provide
    information about the components the server can create. If your
    server supports just a single ActiveX control, you can use the
    default factory implementation instead of implementing the factory
    yourself. Use the \c QAXFACTORY_DEFAULT macro in any
    implementation file (e.g. main.cpp) to instantiate and export the
    default factory:

    \code
    #include <qapplication.h>
    #include <qaxfactory.h>

    #include "theactivex.h"

    QAXFACTORY_DEFAULT(
	TheActiveX,				  // widget class
        "{01234567-89AB-CDEF-0123-456789ABCDEF}", // class ID
	"{01234567-89AB-CDEF-0123-456789ABCDEF}", // interface ID
	"{01234567-89AB-CDEF-0123-456789ABCDEF}", // event interface ID
	"{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
	"{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
	)
    \endcode

    If you implement your own factory reimplement the pure virtual
    functions, provide the unique identifiers for the ActiveX
    controls, and use the \c QAXFACTORY_EXPORT macro to instantiate
    and export it:

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

    QAXFACTORY_EXPORT(
	MyFactory,			          // factory class
	"{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
	"{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
	)
    \endcode

    If you use the \c Q_CLASSINFO macro to provide the unique identifiers
    or other attributes for your class you can use the \c QAXFACTORY_BEGIN,
    \c QAXCLASS and \c QAXFACTORY_END macros to expose one or more classes
    as COM objects.

    \code
    QAXFACTORY_BEGIN(
	"{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
	"{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
    )
	QAXCLASS(Class1)
	QAXCLASS(Class2)
    QAXFACTORY_END()
    \endcode

    Only one QAxFactory implementation may be instantiated and
    exported by an ActiveX server application. This instance is accessible
    through the global qAxFactory() function.

    A factory can also reimplement the registerClass() and
    unregisterClass() functions to set additional flags for an ActiveX
    control in the registry. To limit the number of methods or
    properties a widget class exposes from its parent classes
    reimplement exposeToSuperClass().
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
    Destroys the QAxFactory object.
*/
QAxFactory::~QAxFactory()
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

    Reimplement this function to return the ActiveX server's type
    library identifier.
*/
QUuid QAxFactory::typeLibID() const
{
    return typelib;
}

/*!
    \fn QUuid QAxFactory::appID() const

    Reimplement this function to return the ActiveX server's
    application identifier.
*/
QUuid QAxFactory::appID() const
{
    return app;
}

/*!
    \fn QStringList QAxFactory::featureList() const

    Reimplement this function to return a list of the widgets (class
    names) supported by this factory.
*/

/*!
    Reimplement this function to return a new widget for \a key.
    Propagate \a parent and \a name to the QWidget constructor. Return 
    0 if this factory doesn't support the value of \a key.

    The returned widget will be exposed as an ActiveX control, e.g.
    a COM object that can be embedded as a control into applications.

    The default implementation returns 0.
*/
QWidget *QAxFactory::create( const QString &key, QWidget *parent, const char *name )
{
    Q_UNUSED(key)
    Q_UNUSED(parent)
    Q_UNUSED(name)
    return 0;
}

/*!
    Reimplement this function to return a new object for \a key.
    Propagate \a parent and \a name to the QWidget constructor. Return 
    0 if this factory doesn't support the value of \a key.

    If the object returned is a QWidget it will be exposed as an ActiveX
    control, otherwise the returned object will be exposed as a COM object.

    The default implementation returns the result QAxFactory::create() if 
    \a parent is 0 or a widget, otherwise returns 0.
*/
QObject *QAxFactory::createObject( const QString &key, QObject *parent, const char *name )
{
    if (!parent || parent->isWidgetType())
	return create(key, (QWidget*)parent, name);

    return 0;
}

/*!
    Reimplement this function to return the QMetaObject corresponding to
    \a key, or 0 if this factory doesn't support the value of \a key.

    The default implementation returns the QMetaObject for the class
    \a key.
*/
const QMetaObject *QAxFactory::metaObject(const QString &key) const
{
    // XXX return QMetaObject::metaObject(key.latin1());
    return 0;
}

/*!
    \fn bool QAxFactory::createObjectWrapper(QObject *object, IDispatch **wrapper)

    Reimplement this function to provide the COM object for \a object
    in \a wrapper. Return TRUE if the function was successfull, otherwise
    return FALSE.

    The default implementation creates a generic automation wrapper based 
    on the meta object information of \a object.
*/
// implementation in qaxserverbase.cpp

/*!
    Reimplement this function to return the class identifier for each
    \a key returned by the featureList() implementation, or an empty
    QUuid if this factory doesn't support the value of \a key.

    The default implementation interprets \a key as the class name,
    and returns the value of the Q_CLASSINFO entry "ClassID".
*/
QUuid QAxFactory::classID( const QString &key ) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
	return QUuid();
    QString id = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("ClassID")).value());

    return QUuid(id);
}

/*!
    Reimplement this function to return the interface identifier for
    each \a key returned by the featureList() implementation, or an
    empty QUuid if this factory doesn't support the value of \a key.

    The default implementation interprets \a key as the class name,
    and returns the value of the Q_CLASSINFO entry "InterfaceID".
*/
QUuid QAxFactory::interfaceID( const QString &key ) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
	return QUuid();
    QString id = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("InterfaceID")).value());

    return QUuid(id);
}

/*!
    Reimplement this function to return the identifier of the event
    interface for each \a key returned by the featureList()
    implementation, or an empty QUuid if this factory doesn't support
    the value of \a key.

    The default implementation interprets \a key as the class name,
    and returns the value of the Q_CLASSINFO entry "EventsID".
*/
QUuid QAxFactory::eventsID( const QString &key ) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
	return QUuid();
    QString id = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("EventsID")).value());

    return QUuid(id);
}

/*!
    Registers additional values for the class \a key in the system
    registry using the \a settings object. The standard values have
    already been registed by the framework, but additional values,
    e.g. implemented categories, can be added in an implementation of
    this function.

    \code
    settings->writeEntry( "/CLSID/" + classID(key) + "/Implemented Categories/{00000000-0000-0000-000000000000}/.", QString::null );
    \endcode

    If you reimplement this function you must also reimplement
    unregisterClass() to remove the additional registry values.

    \sa QSettings
*/
void QAxFactory::registerClass( const QString &key, QSettings *settings ) const
{
    Q_UNUSED(key);
    Q_UNUSED(settings)
}

/*!
    Unregisters any additional values for the class \a key from the
    system registry using the \a settings object.

    \code
    settings->removeEntry( "/CLSID/" + classID(key) + "/Implemented Categories/{00000000-0000-0000-000000000000}/." );
    \endcode

    \sa registerClass() QSettings
*/
void QAxFactory::unregisterClass( const QString &key, QSettings *settings ) const
{
    Q_UNUSED(key);
    Q_UNUSED(settings)
}

/*!
    Reimplement this function to return TRUE if \a licenseKey is a valid
    license for the class \a key, or if the current machine is licensed.

    The default implementation returns TRUE if the class \a key is not
    licensed (ie. no Q_CLASSINFO attribute "LicenseKey"), or if 
    \a licenseKey matches the value of the "LicenseKey" attribute, or
    if the machine is licensed through a .LIC file with the same filename
    as this COM server.
*/
bool QAxFactory::validateLicenseKey( const QString &key, const QString &licenseKey) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
	return TRUE;

    QString classKey = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("LicenseKey")).value());
    if (classKey.isEmpty())
	return TRUE;

    if (licenseKey.isEmpty()) {
	extern char qAxModuleFilename[MAX_PATH];
	QString licFile(QFile::decodeName(qAxModuleFilename));
	int lastDot = licFile.lastIndexOf('.');
	licFile = licFile.left(lastDot) + ".lic";
	if (QFile::exists(licFile))
	    return TRUE;
	return FALSE;
    }
    return licenseKey == classKey;
}

/*!
    Reimplement this function to return the name of the super class of
    \a key up to which methods and properties should be exposed by the
    ActiveX control.

    The default implementation interprets \a key as the class name,
    and returns the value of the Q_CLASSINFO entry "ToSuperClass". If
    no such value is set the null-string is returned, and the functions 
    and properties of all the super classes including QWidget will be 
    exposed.

    To only expose the functions and properties of the class itself,
    reimplement this function to return \a key.
*/
QString QAxFactory::exposeToSuperClass( const QString &key ) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
	return QString::null;
    QString str = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("ToSuperClass")).value());

    return str;
}

/*!
    Reimplement this function to return TRUE if the ActiveX control \a key
    should be a top level window, e.g. a dialog. The default implementation
    returns FALSE.
*/
bool QAxFactory::stayTopLevel( const QString &key ) const
{
    return FALSE;
}

/*!
    Reimplement this function to return TRUE if the ActiveX control
     \a key should support the standard ActiveX events
    \list
    \i Click
    \i DblClick
    \i KeyDown
    \i KeyPress
    \i KeyUp
    \i MouseDown
    \i MouseUp
    \i MouseMove
    \endlist

    The default implementation interprets \a key as the class name,
    and returns TRUE if the value of the Q_CLASSINFO entry "StockEvents"
    is "yes". Otherwise this function returns FALSE.
*/
bool QAxFactory::hasStockEvents( const QString &key ) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
	return false;
    return QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("StockEvents")).value()) == "yes";
}


extern bool qAxIsServer;

/*!
    Returns TRUE if the application has been started (by COM) as an ActiveX 
    server, otherwise returns FALSE.

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
    return qAxIsServer;
}

extern char qAxModuleFilename[MAX_PATH];

/*!
    Returns the directory that contains the server binary.

    For out-of-process servers this is the same as 
    QApplication::applicationDirPath(). For in-process servers
    that function returns the directory that contains the hosting
    application.
*/
QString QAxFactory::serverDirPath()
{
    return QFileInfo(QString::fromLocal8Bit(qAxModuleFilename)).dirPath();
}

/*!
    Returns the file path of the server binary.

    For out-of-process servers this is the same as 
    QApplication::applicationFilePath(). For in-process servers
    that function returns the file path of the hosting application.
*/
QString QAxFactory::serverFilePath()
{
    return QString::fromLocal8Bit(qAxModuleFilename);
}

/*!
    Reimplement this function to return TRUE if the server is
    running as a persistent service (e.g. an NT service) and should
    not terminate even when all objects provided have been released.

    The default implementation returns FALSE.
*/
bool QAxFactory::isService() const
{
    return FALSE;
}

/*!
    \enum QAxFactory::ServerType

    This enum specifies the different types of servers that can be
    started with startServer.

    \value SingleInstance The server can create only one instance of each 
    supplied class.
    \value MultipleInstances The server can create multiple instances of
    each supplied class.
*/

/*!
    \fn bool QAxFactory::startServer(ServerType type);

    Starts the COM server with \a type and returns TRUE if successful, 
    otherwise returns FALSE.

    Calling this function if the server is already running (or for an
    in-process server) does nothing and returns TRUE.

    The server is started automatically with \a type set to MultipleUse 
    if the server executable has been started with the \c -activex 
    command line parameter.
*/

/*!
    \fn bool QAxFactory::stopServer();

    Stops the COM server and returns TRUE if successful, otherwise 
    returns FALSE.

    Calling this function if the server is not running (or for an
    in-process server) does nothing and returns TRUE.

    Stopping the server will not invalidate existing objects, but no
    new objects can be created from the existing server process. Usually
    COM will start a new server process if additional objects are requested.

    The server is stopped automatically when the main() function returns.
*/

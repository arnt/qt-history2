#include "qcomponentfactory.h"

#ifndef QT_NO_COMPONENT
#include "qsettings.h"
#include "qlibrary.h"
#include "qcleanuphandler.h"

/*!
  \class QComponentFactory qcomponentfactory.h
  \brief The QComponentFactory class provides static functions to create components.
  \ingroup componentmodel

  The createInstance() function is used to obtain a pointer to an
  interface.

  Use registerServer() to load a shared library which provides the
  QComponentServerInterface and register its components. Use
  unregisterServer() to unregister a shared library's components.

  \sa QComponentServerInterface, QComponentFactoryInterface
*/

QCleanupHandler< QLibrary > qt_component_server_cleanup;

/*!
  Looks up the component identifier \a cid in the system registry, loads
  the corresponding component server and queries for the interface \a
  iid. The parameter \a outer is a pointer to the outer interface used
  for containment and aggregation and is propagated to the \link
  QComponentFactoryInterface::createInstance createInstance \endlink
  implementation of the QComponentFactoryInterface provided by the
  component server if provided.
  Returns the retrieved interface pointer, or NULL if there was an error.

  Example:
  \code
  MyInterface *iface = (MyInterface*)QComponentFactory::createInstance( IID_MyInterface, CID_MyComponent );
  if ( iface ) {
      ...
      iface->release();
  }
  \endcode
*/

QRESULT QComponentFactory::createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface** instance, QUnknownInterface *outer )
{
    QSettings settings;
    bool ok;

    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    QString file = settings.readEntry( "/CLSID/" + cid.toString() + "/InprocServer32/Default", QString::null, &ok );
    int dot = file.findRev( '.' );
    QString ext = file.right( file.length() - dot );
    if ( ext == ".dll" || ext == ".so" || ext == ".dylib" )
	file = file.left( dot );

    if ( !ok )
	return;

    QLibrary *library = new QLibrary( file );
    qt_component_server_cleanup.add( library );

    QComponentFactoryInterface *cfIface =0;
    library->queryInterface( IID_QComponentFactory, (QUnknownInterface**)&cfIface );
    if ( cfIface ) {
	cfIface->createInstance( iid, cid, instance, outer );
	cfIface->release();
    } else {
	library->queryInterface( iid, instance );
    }
}

/*!
  Loads the shared library \a filename and queries for a
  QComponentServerInterface. If the library implements this interface,
  the \link QComponentServerInterface::registerComponents
  registerComponents \endlink function is called.
  
  Returns TRUE if the interface is found and successfully registered,
  otherwise returns FALSE.
*/
bool QComponentFactory::registerServer( const QString &filename )
{
    QLibrary lib( filename, QLibrary::Immediately );
    QComponentServerInterface *iface = 0;
    lib.queryInterface( IID_QComponentServer, (QUnknownInterface**)&iface );
    if ( !iface )
	return FALSE;
    bool ok = iface->registerComponents( filename );
    iface->release();
    return ok;
}

/*!
  Loads the shared library \a filename and queries for a
  QComponentServerInterface. If the library implements this interface,
  the \link QComponentServerInterface::unregisterComponents
  unregisterComponents \endlink function is called.
  
  Returns TRUE if the interface is found and successfully unregistered,
  otherwise returns FALSE.
*/
bool QComponentFactory::unregisterServer( const QString &filename )
{
    QLibrary lib( filename, QLibrary::Immediately );
    QComponentServerInterface *iface = 0;
    lib.queryInterface( IID_QComponentServer, (QUnknownInterface**)&iface );
    if ( !iface )
	return FALSE;
    bool ok = iface->unregisterComponents();
    iface->release();
    return ok;
}

#endif // QT_NO_COMPONENT

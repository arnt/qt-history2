#include "qcomponentinterface.h"
#include "qapplication.h"
#include "qdir.h"

#ifndef QT_NO_PLUGIN

class QInterfaceList : public QList<QUnknownInterface>
{
public:
};

/*!
  \class QUnknownInterface qcomponentinterface.h
  \brief This class serves as a base class for interfaces.
*/

/*!
  Constructs a QUnknownInterface with parent interface /a parent.
*/

QUnknownInterface::QUnknownInterface( QUnknownInterface *parent, const char *name )
: children( 0 ), par( parent ), refcount( 0 ), appInterface( 0 )
{
    objname = name ? qstrdup(name) : 0;
    if ( par )
	par->insertChild( this );
}

/*!
  Destroys the interface and all children.
*/

QUnknownInterface::~QUnknownInterface()
{
    if ( par )
	par->removeChild( this );

    if ( children ) {
	QListIterator<QUnknownInterface> it( *children );
	while ( it.current() ) {
	    QUnknownInterface *obj = it.current();
	    ++it;
	    obj->par = 0;
	    children->removeRef( obj );
	    delete obj;
	}
	delete children;
    }
}

/*!
  Makes interface \a child a child of this interface.
*/

void QUnknownInterface::insertChild( QUnknownInterface *child )
{
    if ( !children )
	children = new QInterfaceList;

    children->append( child );
}

/*!
  Removes \a child from the list of children of this interface.
*/

void QUnknownInterface::removeChild( QUnknownInterface *child )
{
    if ( !children )
	return;

    child->par = 0;
    children->removeRef( child );

    if ( children->isEmpty() ) {
	delete children;
	children = 0;
    }
}

/*!
  Returns the parent interface of this interface, or null if this
  interface is top level.
*/

QUnknownInterface* QUnknownInterface::parent() const
{
    return par;
}

/*!
  Increases the reference counter for this interface by one.
  If the counter was zero, the initialize() function is executed, and
  the result of this routine is returned. Otherwise returns TRUE.
  This function is called automatically when this interface is returned
  as a result of a queryInterface call.

  \sa release
*/
bool QUnknownInterface::ref()
{
    if ( parent() )
	parent()->ref();

    if ( !refcount && !initialize( appInterface ) )
	return FALSE;

    ++refcount;
    return TRUE;
}

/*!
  Decreases the reference counter for this interface by one.
  If the counter becomes zero, the cleanUp() function is executed, and
  the result of this function is returned. Otherwise returns FALSE.

  \sa ref
*/

bool QUnknownInterface::release()
{
    if ( !refcount )
	return TRUE;

    if ( parent() )
	parent()->release();

    bool deref = !--refcount;
    if ( deref )
	return cleanUp( appInterface );

    return FALSE;
}

/*!
  Returns a string identifying the interface. Subclasses of QUnknownInterface
  have to reimplement this function, so that implementations of the interfaces
  can be recognized.
*/

QString QUnknownInterface::interfaceID() const
{
    return "QUnknownInterface";
}

/*!
  This function is called when this interface is referenced the first time. The
  application interface of this interface is set to \a appIface.
  Reimplement this function and return TRUE if the initialization succeeded,
  or FALSE when the process failed. The default implementation always returns
  TRUE. Reimplementations of this function have to  call the parent class implementation.
*/

bool QUnknownInterface::initialize( QApplicationInterface* appIface )
{
    appInterface = appIface;
    return TRUE;
}

/*!
  This function is called when this interface is released so that no further
  references to this interface exist.
  Reimplement this function and return TRUE if the cleanup process succeeded,
  otherwise return FALSE. The default implementation always returns TRUE.
*/

bool QUnknownInterface::cleanUp( QApplicationInterface* )
{
    return TRUE;
}

/*!
  Returns TRUE if this interface has a child interface with an interfaceID
  \a request. If \a rec is TRUE, this function will look for the requested interface
  in the child interfaces, too.
*/

bool QUnknownInterface::hasInterface( const QString &request, bool rec ) const
{
    if ( request.isEmpty() || request == interfaceID() )
	return TRUE;
    if ( !children )
	return FALSE;
    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	if ( it.current()->interfaceID() == request ) {
	    return TRUE;
	} else if ( rec ) {
	    QUnknownInterface *iface;
	    if ( ( iface = it.current()->queryInterface( request ) ) )
		return iface;
	}
	++it;
    }
    return FALSE;
}

/*!
  Returns the list of interface IDs this interface can provide. If \a rec is TRUE, this function
  will return all interface IDs the child interfaces can provide, too.
*/

QStringList QUnknownInterface::interfaceList( bool rec ) const
{
    QStringList list;

    list << interfaceID();

    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	if ( rec ) {
	    QStringList clist = it.current()->interfaceList( rec );
	    for ( QStringList::Iterator ct = clist.begin(); ct != clist.end(); ct++ )
		list << *ct ;
	} else {
	    list << it.current()->interfaceID();
	}
    }

    return list;
}

/*!
  Returns an interface that matches \a request. If \a rec is TRUE, this function will
  look for the requested interface in the child interfaces, too.
  The function returns NULL if this interface can't provide an interface
  with the requested interfaceID. If \a request is a null-string, this interface is returned.
*/

QUnknownInterface* QUnknownInterface::queryInterface( const QString& request, bool rec )
{
    if ( request.isEmpty() || request == interfaceID() )
	return this;
    if ( !children )
	return 0;
    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	if ( it.current()->interfaceID() == request ) {
	    it.current()->appInterface = appInterface;
	    if ( it.current()->ref() )
		return it.current();
	    return 0;
	} else if ( rec ) {
	    QUnknownInterface *iface;
	    if ( ( iface = it.current()->queryInterface( request ) ) )
		return iface;
	}
	++it;
    }
    return 0;
}

/*!
  Returns the QApplicationInterface this interface is connected to.
*/
QApplicationInterface* QUnknownInterface::applicationInterface() const
{
    return appInterface;
}


/*!
  \class QPlugInInterface qcomponentinterface.h

  \brief This class provides a top level interface for modules.

  \sa QApplicationInterfaces
*/

/*!
  Creates a QPlugInInterface. This is always a toplevel interface.
*/

QPlugInInterface::QPlugInInterface( const char* name )
: QUnknownInterface( 0, name )
{
}

/*!
  \reimp
*/

QString QPlugInInterface::interfaceID() const
{
    return "QPlugInInterface";
}

/*!
  Returns a string with the name of the module.
  The default implementation returns QString::null.
*/

QString QPlugInInterface::name() const
{
    return QString::null;
}

/*!
  Returns a string with a description of the module.
  The default implementation returns QString::null.
*/

QString QPlugInInterface::description() const
{
    return QString::null;
}

/*!
  Returns a string with information about the author of the module.
  The default implementation returns QString::null.
*/

QString QPlugInInterface::author() const
{
    return QString::null;
}

/*!
  Returns a string with information about the version of the module.
  The default implementation returns QString::null.
*/

QString QPlugInInterface::version() const
{
    return QString::null;
}

/*!
  \class QApplicationInterface qapplicationinterface.h

  \brief This class provides a top level interface for application modules.

  \sa QPlugInInterface
*/

/*!
  Creates an QApplicationInterface. This is always a toplevel interface.
*/
QApplicationInterface::QApplicationInterface( const char* name )
: QPlugInInterface( name )
{
}

/*!
  \reimp
*/

QString QApplicationInterface::interfaceID() const
{
    return "QApplicationInterface";
}

/*!
  \reimp

  Provides a default implementation that returns the name() of the QApplication object.
*/

QString QApplicationInterface::name() const
{
    return qApp->name();
}

/*!
  Returns the current directory of the application.
*/

QString QApplicationInterface::workDirectory() const
{
    return QDir::currentDirPath();
}

/*!
  Returns the command the application was executed with.
*/

QString QApplicationInterface::command() const
{
    return qApp->argv()[0];
}

/*!
  \class QApplicationComponentInterface qapplicationinterface.h

  \brief This class provides an interface to application components.
*/

/*!
  Creates a QApplicationComponentInterface that provides an interface to the application component
  \a object.
  Note that \a object must not be null.
*/
QApplicationComponentInterface::QApplicationComponentInterface( QObject* object, QUnknownInterface *parent, const char* name )
: QUnknownInterface( parent, name )
{
#if defined(DEBUG)
    if ( !parent )
	qWarning( "QApplicationComponentInterfaces can't be toplevel!" );
    if ( !object )
	qWarning( "Can't create interface with null-object!" );
#endif //DEBUG
    comp = object;
}

/*!
  \reimp
*/

QString QApplicationComponentInterface::interfaceID() const
{
    return "QApplicationComponentInterface";
}

/*!
  \fn QObject* QApplicationComponentInterface::component()

  Returns the pointer to the handled object.
*/

#ifndef QT_NO_PROPERTIES

/*!
  This function returns the value of the property \a p of the handled object.
  Reimplement this function for advanced processing.

  The default implementation makes use of the property system.
*/
QVariant QApplicationComponentInterface::requestProperty( const QCString& p )
{
    if ( !component() )
	return QVariant();
    return component()->property( p );
}
/*!
  This function changes the value of the property \a p of the handled object to \a value and
  returns TRUE if the operation was successful, otherwise returns FALSE.
  Reimplement this function for advanced processing.

  The default implementation sets the property \a p of the handled object to \a value.
*/
bool QApplicationComponentInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    if ( !component() )
	return FALSE;
    return component()->setProperty( p, v );
}

#endif

/*!
  This function connects the \a signal of the handled object to the \a slot of the \a target.
  It returns TRUE if the connection was made successfully, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation connects the \a signal of the handled object to the \a slot of \a target and
  returns the result.
*/
bool QApplicationComponentInterface::requestConnect( const char* signal, QObject* target, const char* slot )
{
    if( !component() )
	return FALSE;
    return QObject::connect( component(), signal, target, slot );
}

/*!
  This function can be used to connect the \a signal of the \a sender to the \a slot of the
  \a handled object.  It returns TRUE if the connection was made successfully, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation makes the connection and returns the result.
*/

bool QApplicationComponentInterface::requestConnect( QObject *sender, const char* signal, const char* slot )
{
    if( !component() )
	return FALSE;
    return QObject::connect( sender, signal, component(), slot );
}

/*!
  This function installs the event filter \e f for the handled object and returns TRUE if the
  eventfilter has been installed, otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation installes the event filter on for the handled object and returns TRUE.
*/
bool QApplicationComponentInterface::requestEvents( QObject* f )
{
    if( !component() )
	return FALSE;

    component()->installEventFilter( f );
    return TRUE;
}

#endif

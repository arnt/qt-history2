#include "qcomponentinterface.h"
#ifndef QT_NO_PLUGIN

#include "qapplication.h"
#include "qdir.h"
#include "qregexp.h"

class QInterfaceList : public QList<QUnknownInterface>
{
public:
};

/*!
  \class QUnknownInterface qcomponentinterface.h
  \brief This class serves as a base class for interfaces.
*/

/*!
  Constructs a QUnknownInterface with parent interface \a parent and name
  \a name.  The object name is a text that can be used to identify this
  QObject, and is particularly convenient for debugging.

  \sa parent(), name()
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

#if defined(QT_DEBUG)
    if ( refcount )
	qDebug( "Interface %s is destroyed while referenced %d times", objname, refcount );
#endif

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
  Returns the internal name of the interface.
*/
const char *QUnknownInterface::name() const
{
    return objname;
}

/*!
  Makes interface \a child a child of this interface.

  \sa removeChild(), parent()
*/

void QUnknownInterface::insertChild( QUnknownInterface *child )
{
    if ( !children )
	children = new QInterfaceList;

    children->append( child );
}

/*!
  Removes \a child from the list of children of this interface.

  \sa insertChild()
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
  as a result of a queryInterface() call.

  \sa release()
*/
bool QUnknownInterface::addRef()
{
    if ( !refcount && !initialize() )
	return FALSE;

    if ( parent() )
	parent()->addRef();

/*
    if ( objname )
	qDebug( "Referencing %s (%d ->%d)", objname, refcount, refcount+1 );*/
    ++refcount;

    return TRUE;
}

/*!
  Decreases the reference counter for this interface by one.
  If the counter becomes zero, the cleanup() function is executed, and
  the result of this function is returned. Otherwise returns FALSE.

  \sa addRef()
*/

bool QUnknownInterface::release()
{
    if ( !refcount ) {
#if defined(QT_CHECK_RANGE)
	if ( objname )
	    qWarning( "%s: Interface refcount out of sync!", objname );
#endif
	return TRUE;
    }
/*    
    if ( objname )
	qDebug( "Dereferencing %s (%d ->%d)", objname, refcount, refcount-1 );*/

    if ( parent() )
	parent()->release();

    bool deref = !--refcount;
    if ( deref )
	return cleanup();

    return FALSE;
}

/*!
  Returns a string identifying the interface. Subclasses of QUnknownInterface
  must reimplement this function like

  \code

    class MyInterface : public QUnknownInterface
    {
       ...
    };

    ...

    QString MyInterface::interfaceId() const
    {
	return createId( QUnknownInterface::interfaceId(), "MyInterface" );
    }

  \endcode

  This will return a string "/QUnknownInterface/MyInterface", identifying the
  interface uniquely in runtime.
    
  \sa ID()
*/

QString QUnknownInterface::interfaceId() const
{
    return createId( "","QUnknownInterface" );
}

/*!
  Returns the Id of this interface, that is, the last part of interfaceId().

  \sa interfaceId()
*/

QString QUnknownInterface::Id() const
{
    const QString id = interfaceId();

    int last = id.findRev( '/' );
    return ( last == -1 ) ? id : id.right( id.length() - last - 1 );
}

/*!
  Returns a QString that represents the interface hierarchy using \a parent
  and \a id.

  \sa interfaceId(), ID()
*/

QString QUnknownInterface::createId( const QString &parent, const QString &id ) const
{
    return parent + "/" + id;
}

/*!
  This function is called when this interface is referenced the first time.
  Reimplement this function and return TRUE if the initialization succeeded,
  or FALSE when the process failed. The default implementation always returns
  TRUE. Reimplementations of this function have to  call the parent class
  implementation.

  \sa addRef(), release(), cleanup()
*/

bool QUnknownInterface::initialize()
{
    return TRUE;
}

/*!
  This function is called when this interface is released so that no further
  references to this interface exist.
  Reimplement this function and return TRUE if the cleanup process succeeded,
  otherwise return FALSE. The default implementation always returns TRUE.

  \sa initialize(), release()
*/

bool QUnknownInterface::cleanup()
{
    return TRUE;
}

/*!
  Returns TRUE if this interface has a child interface with an interfaceId
  \a request. If \a recursive is TRUE, this function will look for the
  requested interface in the child interfaces, too.
  If \a regexp is TRUE, \a request will be interpreted as a regular expression
  with wildcards.

  \sa queryInterface()
*/

bool QUnknownInterface::hasInterface( const QString &request, bool recursive, bool regexp ) const
{
    if ( regexp ) {
	QRegExp re( request, TRUE, TRUE );
	if ( re.match( interfaceId() ) )
	    return TRUE;
    } else {
	if ( interfaceId() == request )
	    return TRUE;
    }
    if ( !children )
	return FALSE;
    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	if ( regexp ) {
	    QRegExp re( request, TRUE, TRUE );
	    if ( re.match( it.current()->interfaceId() ) )
		return TRUE;
	} else {
	    if ( it.current()->interfaceId() == request )
		return TRUE;
	} 
	if ( recursive ) {
	    bool has = it.current()->hasInterface( request, recursive, regexp );
	    if ( has )
		return TRUE;
	}

	++it;
    }
    return FALSE;
}

/*!
  Returns the list of interface IDs this interface can provide. If \a recursive is TRUE, this function
  will return all interface IDs the child interfaces can provide, too.

  \sa hasInterface()
*/

QStringList QUnknownInterface::interfaceList( bool recursive ) const
{
    QStringList list;

    list << interfaceId();

    if ( !children )
	return list;

    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	if ( recursive ) {
	    QStringList clist = it.current()->interfaceList( recursive );
	    for ( QStringList::Iterator ct = clist.begin(); ct != clist.end(); ct++ )
		list << *ct ;
	} else {
	    list << it.current()->interfaceId();
	}
	++it;
    }

    return list;
}

/*!
  Returns an interface that matches \a request. If \a recursive is TRUE, this
  function will look for the requested interface in the child interfaces, too.
  if \a regexp is TRUE, \a request will be interpreted as a regular expression
  with wildcards.  The function returns NULL if this interface can't provide
  the requested interface.

  \sa interfaceId(), hasInterface(), interfaceList()
*/

QUnknownInterface* QUnknownInterface::queryInterface( const QString& request, bool recursive, bool regexp )
{
    bool amI;
    if ( regexp ) {
	QRegExp re( request, TRUE, TRUE );
	amI = re.match( interfaceId() );
    } else {
	amI = interfaceId() == request;
    }
    if ( amI ) {
	if ( addRef() )
	    return this;
	return 0;
    }
    if ( !children )
	return 0;
    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	bool isIt;
	if ( regexp ) {
	    QRegExp re( request, TRUE, TRUE );
	    isIt = re.match( it.current()->interfaceId() );
	} else {
	    isIt = it.current()->interfaceId() == request;
	}
	if ( isIt ) {
	    it.current()->appInterface = appInterface;
	    if ( it.current()->addRef() )
		return it.current();
	    return 0;
	} else if ( recursive ) {
	    QUnknownInterface *iface = it.current()->queryInterface( request, recursive, regexp );
	    if ( iface )
		return iface;
	}
	++it;
    }
    return 0;
}

/*!
  Returns the childinterface with ID \a request, or null if there is no such
  child interface.  The returned interface will not be referenced.
*/

QUnknownInterface* QUnknownInterface::child( const QString &request ) const
{
    if ( !children )
	return 0;

    QRegExp regexp( request, TRUE, TRUE );

    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	if ( regexp.match( it.current()->interfaceId() ) )
	    return it.current();
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
  \class QComponentInterface qcomponentinterface.h

  \brief This class provides a top level interface for modules.

  \sa QApplicationInterfaces
*/

/*!
  Creates a QComponentInterface. This is always a top level interface.
*/

QComponentInterface::QComponentInterface( const char* name )
: QUnknownInterface( 0, name )
{
}

/*!
  \reimp
*/

QString QComponentInterface::interfaceId() const
{
    return createId( QUnknownInterface::interfaceId(), "QComponentInterface" );
}

/*!
  Returns a string with the name of the module.
  The default implementation returns QString::null.
*/

QString QComponentInterface::brief() const
{
    return QString::null;
}

/*!
  Returns a string with a description of the module.
  The default implementation returns QString::null.
*/

QString QComponentInterface::description() const
{
    return QString::null;
}

/*!
  Returns a string with information about the author of the module.
  The default implementation returns QString::null.
*/

QString QComponentInterface::author() const
{
    return QString::null;
}

/*!
  Returns a string with information about the version of the module.
  The default implementation returns QString::null.
*/

QString QComponentInterface::version() const
{
    return QString::null;
}

/*!
  \class QApplicationInterface qcomponentinterface.h

  \brief This class provides a top level interface for application modules.

  \sa QComponentInterface
*/

/*!
  Creates an QApplicationInterface. This is always a top level interface.
*/
QApplicationInterface::QApplicationInterface( const char* name )
: QComponentInterface( name )
{
    appInterface = this;
}

/*!
  \reimp
*/

QString QApplicationInterface::interfaceId() const
{
    return createId( QComponentInterface::interfaceId(), "QApplicationInterface" );
}

/*!
  \reimp

  The default implementation returns the name of the QApplication object.
*/

QString QApplicationInterface::brief() const
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
  \class QApplicationComponentInterface qcomponentinterface.h

  \brief This class provides an interface to application components.
*/

/*!
  Creates a QApplicationComponentInterface that provides an interface to
  the application component \a object.
*/
QApplicationComponentInterface::QApplicationComponentInterface( QObject* object, QUnknownInterface *parent, const char* name )
: QUnknownInterface( parent, name )
{
#if defined(QT_DEBUG)
    if ( !parent )
	qWarning( "QApplicationComponentInterfaces can't be top level!" );
#endif
    comp = object;
}

/*!
  \reimp
*/

QString QApplicationComponentInterface::interfaceId() const
{
    return createId( QUnknownInterface::interfaceId(), "QApplicationComponentInterface" );
}

/*!
  \fn QObject* QApplicationComponentInterface::component() const

  Returns the pointer to the handled object.
*/

/*!
  \fn void QApplicationComponentInterface::setComponent( QObject* c )
  
  Sets the handled object to \a c.
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
  This function changes the value of the property \a p of the handled object
  to \a value and returns TRUE if the operation was successful, otherwise
  returns FALSE.
  Reimplement this function for advanced processing.

  The default implementation sets the property \a p of the handled object
  to \a value.
*/
bool QApplicationComponentInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    if ( !component() )
	return FALSE;
    return component()->setProperty( p, v );
}

#endif

/*!
  This function connects the \a signal of the handled object to the \a slot of
  the \a target.  It returns TRUE if the connection was made successfully,
  otherwise FALSE.
  Reimplement this function for advanced processing.

  The default implementation connects the \a signal of the handled object to
  the \a slot of \a target and returns the result.
*/
bool QApplicationComponentInterface::requestConnect( const char* signal, QObject* target, const char* slot )
{
    if( !component() )
	return FALSE;
    return QObject::connect( component(), signal, target, slot );
}

/*!
  This function can be used to connect the \a signal of the \a sender to the
  \a slot of the \a handled object.  It returns TRUE if the connection was
  made successfully, otherwise FALSE.
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
  This function installs the event filter \e f for the handled object and
  returns TRUE if the eventfilter has been installed, otherwise FALSE.
  Reimplement this function for advanced processing.
*/
bool QApplicationComponentInterface::requestEvents( QObject* f )
{
    if( !component() )
	return FALSE;

    component()->installEventFilter( f );
    return TRUE;
}

#endif

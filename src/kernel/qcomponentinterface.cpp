#include "qcomponentinterface.h"
#ifndef QT_NO_COMPONENT

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
  Constructs a QUnknownInterface with parent interface \a parent. 

  \sa parent()
*/

QUnknownInterface::QUnknownInterface( QUnknownInterface *parent )
: children( 0 ), par( parent ), refcount( 0 )
{
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
	qDebug( "Interface is destroyed while referenced!" );
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
  Increases the reference counter for this interface by one and returns
  the old reference count. If the interface has a parent, addRef is called
  for the parent interface.
  This function is called automatically when this interface is returned
  as a result of a queryInterface() call.

  \sa release()
*/
unsigned long QUnknownInterface::addRef()
{
    if ( par )
	par->addRef();

    return refcount++;
}

/*!
  Decreases the reference counter for this interface by one and returns
  the new reference count.

  \sa addRef()
*/

unsigned long  QUnknownInterface::release()
{
    if ( !refcount ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "Interface refcount out of sync!" );
#endif
	return TRUE;
    }

    if ( par )
	par->release();

    return --refcount;
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

    const char* MyInterface::interfaceId() const
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
  Returns a QString that represents the interface hierarchy using \a parent
  and \a id.

  \sa interfaceId(), ID()
*/

QString QUnknownInterface::createId( const QString& parent, const QString& id ) const
{
    return parent + "/" + id;
}

/*!
  Returns an interface that matches \a request, or NULL if this interface 
  can't provide the requested interface.

  \sa interfaceId()
*/

QUnknownInterface* QUnknownInterface::queryInterface( const QString& request )
{
    QString r;
    if ( request[0] != '/' )
	r = "*/"+request+"*";
    else
	r = request;

    if ( QRegExp( r, TRUE, TRUE ).match( interfaceId() ) ) {
	addRef();
	return this;
    }
    if ( !children )
	return 0;
    QListIterator<QUnknownInterface> it( *children );
    while ( it.current() ) {
	QUnknownInterface *iface = it.current();
	++it;
	QUnknownInterface *riface = iface->queryInterface( r );
	if ( riface )
	    return riface;
    }
    return 0;
}

/*!
  \class QComponentInterface qcomponentinterface.h

  \brief This interface provides functions to get information about components.

  \sa QApplicationInterfaces
*/

/*!
  \reimp
*/

QString QComponentInterface::interfaceId() const
{
    return createId( QUnknownInterface::interfaceId(), "QComponentInterface" );
}

/*!
  \fn QString QComponentInterface::brief() const

  Returns a string with the name of the module.
*/

/*!
  \fn QString QComponentInterface::description() const

  Returns a string with a description of the module.
*/

/*!
  \fn QString QComponentInterface::author() const

  Returns a string with information about the author of the module.
*/

/*!
  \fn QString QComponentInterface::version() const

  Returns a string with information about the version of the module.
*/


#endif //QT_NO_COMPONENT

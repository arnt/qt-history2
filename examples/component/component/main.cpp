#include <qcomponentfactory.h>

#include "comp1.h"
#include "comp2.h"

class ExComponent : public QComponentRegistrationInterface, 
		    public QComponentFactoryInterface
{
public:
    ExComponent();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    bool registerComponents( const QString &filepath ) const;
    bool unregisterComponents() const;

    QRESULT createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface **iface, QUnknownInterface *outer );
};

ExComponent::ExComponent()
{
}

QRESULT ExComponent::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( iid == IID_QUnknown )
	*iface = (QUnknownInterface*)(QComponentRegistrationInterface*)this;
    else if ( iid == IID_QComponentRegistration )
	*iface = (QComponentRegistrationInterface*)this;
    else if ( iid == IID_QComponentFactory )
	*iface = (QComponentFactoryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

bool ExComponent::registerComponents( const QString &filepath ) const
{
    return QComponentFactory::registerComponent( Component1::cid, filepath, "Qt.Example", "1", "An example component for Qt" ) &&
	   QComponentFactory::registerComponent( Component2::cid, filepath, "Qt.AnotherExample", "1", "Another example component for Qt" );
}

bool ExComponent::unregisterComponents() const
{
    return QComponentFactory::unregisterComponent( Component1::cid ) &&
	   QComponentFactory::unregisterComponent( Component2::cid );
}

QRESULT ExComponent::createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface **iface, QUnknownInterface *outer )
{
    *iface = 0;
    QRESULT res = QE_NOCOMPONENT;
    if ( cid == Component1::cid ) {
	Component1 *c = new Component1;
	res = c->queryInterface( iid, iface );
	if ( res != QS_OK )
	    delete c;
    } else if ( cid == Component2::cid ) {
	Component2 *c = new Component2;
	res = c->queryInterface( iid, iface );
	if ( res != QS_OK )
	    delete c;
    }

    return res;
}


Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( ExComponent );
}

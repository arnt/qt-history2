#include <qcomponentfactory.h>

class Component : public QComponentRegistrationInterface, 
		  public QComponentFactoryInterface
{
public:
    Component();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    bool registerComponents( const QString &filepath ) const;
    bool unregisterComponents() const;

    QRESULT createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface **iface, QUnknownInterface *outer );

    static QUuid cid;
};

QUuid Component::cid = QUuid(0x1d8518cd, 0xe8f5, 0x4366, 0x99, 0xe8, 0x87, 0x9f, 0xd7, 0xe4, 0x82, 0xde );

Component::Component()
{
}

QRESULT Component::queryInterface( const QUuid &iid, QUnknownInterface **iface )
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

bool Component::registerComponents( const QString &filepath ) const
{
    return QComponentFactory::registerComponent( Component::cid, filepath, "Qt.Example", "An example component for Qt" );
}

bool Component::unregisterComponents() const
{
    return QComponentFactory::unregisterComponent( Component::cid );
}

QRESULT Component::createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface **iface, QUnknownInterface *outer )
{
    *iface = 0;
    if ( cid == Component::cid ) {
	Component *c = new Component;
	QRESULT res = c->queryInterface( iid, iface );
	if ( res != QS_OK )
	    delete c;
	return res;
    }

    return QE_NOCOMPONENT;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE(Component);
}

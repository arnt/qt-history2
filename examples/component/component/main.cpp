#include "../interfaces/printinterface.h"

#include <qcomponentfactory.h>

class Component : public QComponentRegistrationInterface, 
		  public QComponentFactoryInterface,
		  public PrintInterface
{
public:
    Component();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    bool registerComponents( const QString &filepath ) const;
    bool unregisterComponents() const;

    QRESULT createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface **iface, QUnknownInterface *outer );

    void sayHello();

    static QUuid cid;
};

// {8AA0BD9B-8136-47F3-8C4F-BC8C08E93D34} 
QUuid Component::cid = QUuid( 0x8aa0bd9b, 0x8136, 0x47f3, 0x8c, 0x4f, 0xbc, 0x8c, 0x08, 0xe9, 0x3d, 0x34 );

// {1DA52229-5C33-40C3-925C-53777D39E458} 
// QUuid Component::cid = QUuid( 0x1da52229, 0x5c33, 0x40c3, 0x92, 0x5c, 0x53, 0x77, 0x7d, 0x39, 0xe4, 0x58 );

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
    else if ( iid == IID_Print )
	*iface = (PrintInterface*)this;
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

void Component::sayHello()
{
    qDebug( "Hi there!" );
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE(Component);
}

#include <qpushbutton.h>
#include <qvalidator.h>
#include <qserversocket.h>

#include "some.h"
#include "thing.h"


class ThingServer : public QServerSocket
{
public:
    ThingServer( int port, QObject *parent )
	: QServerSocket( port, 0, parent )
    {
	if ( ok() )
	    qDebug( "server started" );
	else
	    qDebug( "failed to start server" );
    }

    ~ThingServer()
    {
    }

    void newConnection( int socket )
    {
	qDebug( "newConnection!" );
	new Thing( socket );
    }
};


Some::Some()
{
    QPushButton *pb;
    QLabel *lb;
    QHBox *hb;

    hb = new QHBox( this );
    lb = new QLabel( "Host ", hb );
    hostEdit = new QLineEdit( "localhost", hb );

    hb = new QHBox( this );
    lb = new QLabel( "Port ", hb );
    portEdit = new QLineEdit( "2323", hb );
    portEdit->setValidator( new QIntValidator( portEdit ) );

    hb = new QHBox( this );
    pb = new QPushButton( "Start Server", hb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(startServer()) );
    pb = new QPushButton( "Start Client", hb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(startClient()) );

    pb = new QPushButton( "Quit", this );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(quit()) );
}


Some::~Some()
{
}


void Some::quit()
{
    emit quitted();
}


void Some::startServer()
{
    new ThingServer( portEdit->text().toUInt(), this );
}


void Some::startClient()
{
    new Thing(  hostEdit->text(), portEdit->text().toUInt() );
}

#include "ipcserver.h"

#include <qsocket.h>

class IpcSocket : public QSocket
{
    Q_OBJECT

public:
    IpcSocket( QObject *parent) : QSocket( parent )
    {
	packetSize = 0;
	connect( this, SIGNAL(readyRead()), SLOT(read()) );
    }

signals:
    void receivedText( const QString& );
    void receivedPixmap( const QPixmap& );

private slots:
    void read()
    {
	Q_ULONG bytesAvail = bytesAvailable();
	QDataStream ds( this );
	for ( ;; ) {
	    if ( packetSize == 0 ) {
		if ( bytesAvail < 4 )
		    return;
		ds >> packetSize;
		bytesAvail -= 4;
	    } else {
		if ( bytesAvail < packetSize )
		    return;
		QString txt;
		ds >> txt;
		bytesAvail -= packetSize;
		packetSize = 0;
		emit receivedText( txt );
	    }
	}
    }

private:
    Q_UINT32 packetSize;
};

IpcServer::IpcServer( Q_UINT16 port, QObject *parent ) :
    QServerSocket( 0x7f000001, port, 1, parent )
{
}

void IpcServer::newConnection( int socket )
{
    IpcSocket *s = new IpcSocket( this );
    s->setSocket( socket );
    connect( s, SIGNAL(receivedText(const QString&)),
	    SIGNAL(receivedText(const QString&)) );
    connect( s, SIGNAL(receivedPixmap(const QPixmap&)),
	    SIGNAL(receivedPixmap(const QPixmap&)) );
}

#include "ipcserver.moc"

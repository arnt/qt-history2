#include "ipcserver.h"

#include <qsocket.h>
#include <qvariant.h>
#include <qimage.h>
#include <qpalette.h>
#include <qapplication.h>

class IpcSocket : public QSocket
{
    Q_OBJECT

public:
    IpcSocket( QObject *parent) : QSocket( parent )
    {
	packetSize = 0;
	packetType = 0;
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
	    } else if ( packetType == 0 ) {
		if ( bytesAvail < 1 )
		    return;
		ds >> packetType;
		bytesAvail -= 1;
	    } else {
		if ( bytesAvail < packetSize )
		    return;
		if ( packetType == QVariant::String ) {
		    QString txt;
		    ds >> txt;
		    bytesAvail -= packetSize;
		    emit receivedText( txt );
		} else if ( packetType == QVariant::Image ) {
		    QImage image;
		    ds >> image;
		    bytesAvail -= packetSize;
		    emit receivedPixmap( QPixmap(image) );
		} else if ( packetType == QVariant::Palette ) {
		    QPalette pal;
		    ds >> pal;
		    bytesAvail -= packetSize;
		    QApplication::setPalette( pal, TRUE );
		}
		packetSize = 0;
		packetType = 0;
	    }
	}
    }

private:
    Q_UINT32 packetSize;
    Q_UINT8 packetType;
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

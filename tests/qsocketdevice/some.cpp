#include <qpushbutton.h>
#include <qdatetime.h>
#include <qmessagebox.h>

#include "some.h"

Some::Some( const QString& host_, uint port_ )
    : port(port_)
{
    sd = new QSocketDevice( QSocketDevice::Stream );
    if ( !address.setAddress( host_ ) )
	qWarning( "Error parsing Host Address" );

    QPushButton *pb;
    QLabel *lb;
    QVBox *vb;

    vb = new QVBox( this );
    pb = new QPushButton( "Connect", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(connect()) );
    pb = new QPushButton( "Bind", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(bind()) );
    pb = new QPushButton( "Listen", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(listen()) );
    pb = new QPushButton( "Accept", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(accept()) );
    pb = new QPushButton( "Wait For More (10s)", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(waitForMore()) );
    pb = new QPushButton( "Quit", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(quit()) );

    vb = new QVBox( this );
    infoLabel = new QLabel( vb );
    blockingBox = new QCheckBox( "Blocking", vb );
    QObject::connect( blockingBox, SIGNAL(toggled(bool)), SLOT(setBlock(bool)) );
    infoLabel->setText( getInfo() );
    pb = new QPushButton( "Print Info", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(printInfo()) );

    vb = new QVBox( this );
    lb = new QLabel( vb );
    QObject::connect( this, SIGNAL(dataRead(const QString&)),
	    lb, SLOT(setText(const QString&)) );
    pb = new QPushButton( "Read", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(read()) );

    vb = new QVBox( this );
    writeEdit = new QMultiLineEdit( vb );
    pb = new QPushButton( "Write", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(write()) );
}

Some::~Some()
{
    sd->close();
    delete sd;
}

////////////////////////////////
// misc.
////////////////////////////////

void Some::quit()
{
    emit quitted();
}

void Some::printInfo()
{
    infoLabel->setText( getInfo() );
}

QString Some::getInfo()
{
    if ( sd->blocking() != blockingBox->isOn() )
	blockingBox->toggle();

    QString info;

    info =  QString("Socket: ")
	+ QString::number( sd->socket() ) + "\n";
    info += QString("Address: ")
	+ sd->address().toString() + " : "
	+ QString::number( (int)sd->port() ) + "\n";
    info += QString("Peer: ")
	+ sd->peerAddress().toString() + " : "
	+ QString::number( (int)sd->peerPort() ) + "\n";
    info += QString("Bytes available: ")
	+ QString::number( sd->bytesAvailable() ) + "\n";
    info += QString("Blocking: ")
	+ (sd->blocking()?"Y":"N") + "\n";
    info += QString("Rec Buf Size: ")
	+ QString::number( sd->receiveBufferSize() ) + "\n";
    info += QString("Snd Buf Size: ")
	+ QString::number( sd->sendBufferSize() ) + "\n";

    return info;
}

void Some::setBlock( bool on )
{
    sd->setBlocking( on );
}

////////////////////////////////
// connect and relatives
////////////////////////////////

void Some::connect()
{
    if ( !sd->connect( address, port ) ) {
	if ( sd->error() == QSocketDevice::NoError )
	    qWarning( "Connecting to socket device failed; no error" );
	else
	    qWarning( "Error connecting socket device" );
    }
}

void Some::bind()
{
    if ( !sd->bind( address, port ) ) {
	qWarning( "Error binding socket device" );
    }
}

void Some::listen()
{
    if ( !sd->listen( 50 ) ) {
	qWarning( "Error listening to socket device" );
    }
}

void Some::accept()
{
    int fd = sd->accept();
    if ( fd == -1 ) {
	qWarning( "Error accepting" );
    } else {
	Some *s = new Some( address.toString(), port );
	QObject::connect( s, SIGNAL(quitted()), SLOT(quit()) );
	s->sd->setSocket( fd, QSocketDevice::Stream );
	s->show();
    }
}

void Some::waitForMore()
{
    QTime waitTime;
    waitTime.start();
    int available = sd->waitForMore( 10000 );
    int waited = waitTime.elapsed();

    QString message =
	QString( "Stopped waiting after %1 s; %2 bytes available"
	       ).arg( (float)waited/1000 ).arg( available );
    QMessageBox::information( this, "", message );
}

////////////////////////////////
// io
////////////////////////////////

void Some::read()
{
    const int buflen = 16;
    char buf[buflen];
    int r = sd->readBlock( buf, buflen-1 );
    if ( r < 0 ) {
	qWarning( "Error readBlock" );
    } else {
	buf[r] = 0;
	emit dataRead( QString(buf) );
    }
}

void Some::write()
{
    QString text = writeEdit->text();
    int r = sd->writeBlock( text.latin1(), text.length() );
    qDebug( "Write %d bytes", r );
}

#include <qapplication.h>
#include <qpushbutton.h>
#include <qdatetime.h>

#include "thing.h"

static const int much = 54321;

Thing::Thing( int fd )
{
    socket = new QSocket( this );
    socket->setSocket( fd );
    init();
}


Thing::Thing( QString host, int port )
{
    socket = new QSocket( this );
    init();
    socket->connectToHost( host, port );
}


Thing::~Thing()
{
}


////////////////////////////////
// misc.
////////////////////////////////

void Thing::init()
{
    QPushButton *pb;
    QVBox *vb;
    QHBox *hb;

    // general buttons
    vb = new QVBox( this );
    pb = new QPushButton( "waitForMore( 10s )", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(waitForMore()) );
    pb = new QPushButton( "Close Socket", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(closeSocket()) );
    pb = new QPushButton( "Cancel", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(close()) );

    // info
    vb = new QVBox( this );
    infoText = new QTextView( vb );
    infoText->setText( getInfo() );
    pb = new QPushButton( "Print Info", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(printInfo()) );

    // signal info
    vb = new QVBox( this );
    signalText = new QTextView( vb );
    QObject::connect( socket, SIGNAL(hostFound()), SLOT(hostFound()) );
    QObject::connect( socket, SIGNAL(connected()), SLOT(connected()) );
    QObject::connect( socket, SIGNAL(connectionClosed()), SLOT(closed()) );
    QObject::connect( socket, SIGNAL(delayedCloseFinished()), SLOT(delayedCloseFinished()) );
    QObject::connect( socket, SIGNAL(readyRead()), SLOT(readyRead()) );
    QObject::connect( socket, SIGNAL(bytesWritten(int)), SLOT(bytesWritten(int)) );
    QObject::connect( socket, SIGNAL(error(int)), SLOT(error(int)) );

    // read
    vb = new QVBox( this );
    readText = new QTextView( vb );
    hb = new QHBox( vb );
    pb = new QPushButton( "Read", hb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(read()) );
    pb = new QPushButton( "Read Line", hb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(readLine()) );
    pb = new QPushButton( "Read Much", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(readMuch()) );

    // write
    vb = new QVBox( this );
    writeEdit = new QMultiLineEdit( vb );
    hb = new QHBox( vb );
    pb = new QPushButton( "Write", hb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(write()) );
    pb = new QPushButton( "Flush", hb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(flush()) );
    pb = new QPushButton( "Write Much", vb );
    QObject::connect( pb, SIGNAL(pressed()), SLOT(writeMuch()) );

    show();
}


QString Thing::getInfo()
{
    QString info;

    info =  QString("Socket: ")
	+ QString::number( socket->socket() ) + "\n";
    info += QString("Address: ")
	+ socket->address().toString() + " : "
	+ QString::number( (int)socket->port() ) + "\n";
    info += QString("Peer: ")
	+ socket->peerAddress().toString() + " : "
	+ QString::number( (int)socket->peerPort() ) + " name: "
	+ socket->peerName() + "\n";
    info += QString("Bytes available: ")
	+ QString::number( socket->bytesAvailable() ) + " resp. "
	+ QString::number( socket->size() ) + "\n";
    info += QString("Bytes to write: ")
	+ QString::number( socket->bytesToWrite() ) + "\n";
    info += QString("Can read line: ")
	+ (socket->canReadLine()?"Y":"N") + "\n";
    info += QString("At: ")
	+ QString::number( socket->at() ) + "\n";
    info += QString("At end: ")
	+ (socket->atEnd()?"Y":"N") + "\n";

    return info;
}


void Thing::printInfo()
{
    infoText->setText( getInfo() );
}


////////////////////////////////
// io
////////////////////////////////

void Thing::read()
{
    const int buflen = 16;
    char buf[buflen];
    int r = socket->readBlock( buf, buflen-1 );
    if ( r < 0 ) {
	qWarning( "Error readBlock" );
    } else {
	buf[r] = 0;
	readText->append( QString(buf) );
    }
}


void Thing::readLine()
{
    readText->append( socket->readLine() );
}


void Thing::readMuch()
{
    const int buflen = much + 1;
    char buf[buflen];
    int r = socket->readBlock( buf, buflen-1 );
    if ( r < 0 ) {
	qWarning( "Error readBlock" );
    } else {
	buf[r] = 0;
	readText->append( QString(buf) );
    }
}


void Thing::write()
{
    QString text = writeEdit->text();
    socket->writeBlock( text.latin1(), text.length() );
}


void Thing::flush()
{
    socket->flush();
}


void Thing::writeMuch()
{
    char buf[much];
    socket->writeBlock( buf, much );
}


void Thing::waitForMore()
{
    QTime waitTime;
    waitTime.start();
    signalText->append( "Start waiting for more\n" );
    qApp->processEvents();
    int available = socket->waitForMore( 10000 );
    int waited = waitTime.elapsed();
    signalText->append(
	    QString( "Stopped waiting after %1 s; %2 bytes available\n"
		).arg( (float)waited/1000 ).arg( available ) );
}


void Thing::closeSocket()
{
    socket->close();
}

////////////////////////////////
// signal info
////////////////////////////////

void Thing::hostFound()
{
    signalText->append( "hostFound()\n" );
}


void Thing::connected()
{
    signalText->append( "connected()\n" );
}


void Thing::closed()
{
    signalText->append( "connectionClosed()\n" );
}


void Thing::delayedCloseFinished()
{
    signalText->append( "delayedCloseFinished()\n" );
}


void Thing::readyRead()
{
    signalText->append( "readyRead()\n" );
}


void Thing::bytesWritten( int i )
{
    signalText->append( "bytesWritten( "
	    + QString::number( i )
	    +  " )\n" );
}


void Thing::error( int i )
{
    signalText->append( "error( "
	    + QString::number( i )
	    +  " )\n" );
}

#include <qsocket.h>
#include <qtextstream.h>
#include <qprocess.h>
#include <qtimer.h>

#include "qassistantclient.h"

QAssistantClient::QAssistantClient( const QString &path, QObject *parent, const char *name )
    : QObject( parent, name ), host ( "localhost" )
{
    if ( path.isEmpty() )
	assistantCommand = "assistant";
    else
	assistantCommand = path + "/assistant";
	socket = new QSocket( this );
    connect( socket, SIGNAL( connected() ),
	    SLOT( socketConnected() ) );
    connect( socket, SIGNAL( connectionClosed() ),
	    SLOT( socketConnectionClosed() ) );
    opened = FALSE;
    proc = 0;
    port = 0;
    pageBuffer = "";
}

QAssistantClient::~QAssistantClient()
{
    if ( proc ) {
	proc->tryTerminate();
	proc->kill();
    }
}

void QAssistantClient::openAssistant()
{
    if ( proc )
	return;
    proc = new QProcess( this );
    proc->addArgument( assistantCommand );
    proc->addArgument( "-server" );
    proc->launch( QString::null );
    connect( proc, SIGNAL( readyReadStdout() ),
	     this, SLOT( readPort() ) );
}

void QAssistantClient::readPort()
{
    QString p = proc->readLineStdout();
    Q_UINT16 port = p.toUShort();
    if ( port == 0 ) {
	emit error( tr( "Cannot connect to Qt Assistant." ) );
	return;
    }
    socket->connectToHost( host, port );
    disconnect( proc, SIGNAL( readyReadStdout() ),
		this, SLOT( readPort() ) );
}

void QAssistantClient::closeAssistant()
{
    if ( !opened )
	return;
    proc->tryTerminate();
    proc->kill();
}

void QAssistantClient::showPage( const QString &page )
{
    if ( !opened ) {
	pageBuffer = page;
	return;
    }
    QTextStream os( socket );
    os << page << "\n";
}

bool QAssistantClient::isOpen() const
{
    return opened;
}

void QAssistantClient::socketConnected()
{
    opened = TRUE;
    if ( !pageBuffer.isEmpty() )
	showPage( pageBuffer );
    emit assistantOpened();
}

void QAssistantClient::socketConnectionClosed()
{
    delete proc;
    proc = 0;
    opened = FALSE;
    emit assistantClosed();
}

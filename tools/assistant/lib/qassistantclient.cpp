/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qassistantclient.h"

#include <qtcpsocket.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qmap.h>

class QAssistantClientPrivate
{
    friend class QAssistantClient;
    QStringList arguments;
};

static QMap<const QAssistantClient*,QAssistantClientPrivate*> *dpointers = 0;

static QAssistantClientPrivate *data( const QAssistantClient *client, bool create=false )
{
    if( !dpointers )
        dpointers = new QMap<const QAssistantClient*,QAssistantClientPrivate*>;
    QAssistantClientPrivate *d = (*dpointers)[client];
    if( !d && create ) {
        d = new QAssistantClientPrivate;
        dpointers->insert( client, d );
    }
    return d;
}

/*!
    \class QAssistantClient
    \brief The QAssistantClient class provides a means of using Qt
    Assistant as an application's help tool.

    \ingroup helpsystem

    Using Qt Assistant is simple: create a QAssistantClient instance,
    then call showPage() as often as necessary to show your help
    pages. When you call showPage(), Qt Assistant will be launched if
    it isn't already running.

    The QAssistantClient instance can open (openAssistant()) or close
    (closeAssistant()) Qt Assistant whenever required. If Qt Assistant
    is open, isOpen() returns true.

    One QAssistantClient instance interacts with one Qt Assistant
    instance, so every time you call openAssistant(), showPage() or
    closeAssistant() they are applied to the particular Qt Assistant
    instance associated with the QAssistantClient.

    When you call openAssistant() the assistantOpened() signal is
    emitted. Similarly when closeAssistant() is called,
    assistantClosed() is emitted. In either case, if an error occurs,
    error() is emitted.

    This class is not included in the Qt library itself. To use it you
    must link against \c libqassistantclient.a (Unix) or \c
    qassistantclient.lib (Windows), which is built into \c INSTALL/lib
    if you built the Qt tools (\c INSTALL is the directory where Qt is
    installed). If you use qmake, then you can simply add the following
    line to your pro file:

    \code
        LIBS += -lqassistantclient
    \endcode

    See also "Adding Documentation to Qt Assistant" in the \link
    assistant.book Qt Assistant manual\endlink.
*/

/*!
    \fn void QAssistantClient::assistantOpened()

    This signal is emitted when Qt Assistant is open and the
    client-server communication is set up.
*/

/*!
    \fn void QAssistantClient::assistantClosed()

    This signal is emitted when the connection to Qt Assistant is
    closed. This happens when the user exits Qt Assistant, if an
    error in the server or client occurs, or if closeAssistant() is
    called.
*/

/*!
    \fn void QAssistantClient::error( const QString &message )

    This signal is emitted if Qt Assistant cannot be started, or if an
    error occurs during the initialization of the connection between
    Qt Assistant and the calling application. The \a message provides an
    explanation of the error.
*/

/*!
    Constructs an assistant client object. The \a path specifies the
    path to the Qt Assistant executable. If \a path is an empty
    string the system path (\c{%PATH%} or \c $PATH) is used.

    The assistant client object is a child of \a parent and is called
    \a name.
*/
QAssistantClient::QAssistantClient( const QString &path, QObject *parent )
    : QObject( parent ), host ( "localhost" )
{
    if ( path.isEmpty() )
        assistantCommand = "assistant";
    else {
        QFileInfo fi( path );
        if ( fi.isDir() )
            assistantCommand = path + "/assistant";
        else
            assistantCommand = path;
    }

#if defined(Q_OS_MAC)
    assistantCommand += ".app/Contents/MacOS/assistant";
#endif

    socket = new QTcpSocket( this );
    connect( socket, SIGNAL( connected() ),
            SLOT( socketConnected() ) );
    connect( socket, SIGNAL( disconnected() ),
            SLOT( socketConnectionClosed() ) );
    connect( socket, SIGNAL( error( QTcpSocket::SocketError ) ),
             SLOT( socketError( QTcpSocket::SocketError ) ) );
    opened = false;
    proc = new QProcess( this );
    port = 0;
    pageBuffer = "";
    connect( proc, SIGNAL( readyReadStandardError() ),
             this, SLOT( readStdError() ) );
}

/*!
    Destroys the assistant client object.
*/
QAssistantClient::~QAssistantClient()
{
    if ( proc->state() == QProcess::Running )
        proc->terminate();

    if( dpointers ) {
        QAssistantClientPrivate *d = (*dpointers)[ this ];
        if ( d ) {
            dpointers->remove(this);
            delete d;
            if( dpointers->isEmpty() ) {
                delete dpointers;
                dpointers = 0;
            }
        }
    }
}

/*!
    This function opens Qt Assistant, and sets up the client-server
    communiction between the application and Qt Assistant. If it is
    already open, this function does nothing. If an error occurs,
    error() is emitted.

    \sa assistantOpened()
*/
void QAssistantClient::openAssistant()
{
    if ( proc->state() == QProcess::Running )
        return;

    QStringList args;
    args.append("-server");
    if( !pageBuffer.isEmpty() ) {
        args.append( "-file" );
        args.append( pageBuffer );
    }

    QAssistantClientPrivate *d = data( this );
    if( d ) {
        QStringList::ConstIterator it = d->arguments.begin();
        while( it!=d->arguments.end() ) {
            args.append( *it );
            ++it;
        }
    }

    connect( proc, SIGNAL( readyReadStandardOutput() ),
        this, SLOT( readPort() ) );
    connect( proc, SIGNAL( error(QProcess::ProcessError) ),
        this, SLOT( procError(QProcess::ProcessError) ) );

    proc->start(assistantCommand, args);
}

void QAssistantClient::procError(QProcess::ProcessError err)
{
    switch (err)
    {
    case QProcess::FailedToStart:
        emit error( tr( "Failed to start Qt Assistant." ) );
        break;
    case QProcess::Crashed:
        emit error( tr( "Qt Assistant crashed." ) );
        break;
    default:
        emit error( tr( "Error while running Qt Assistant." ) );
    }
}

void QAssistantClient::readPort()
{
    QString p = proc->readAllStandardOutput();
    quint16 port = p.toUShort();
    if ( port == 0 ) {
        emit error( tr( "Cannot connect to Qt Assistant." ) );
        return;
    }
    socket->connectToHost( host, port );
    disconnect( proc, SIGNAL( readyReadStandardOutput() ),
                this, SLOT( readPort() ) );
}

/*!
    Use this function to close Qt Assistant.

    \sa assistantClosed()
*/
void QAssistantClient::closeAssistant()
{
    if ( !opened )
        return;
    proc->terminate();
}

/*!
    Call this function to make Qt Assistant show a particular \a page.
    The \a page is a filename (e.g. \c myhelpfile.html). See "Adding
    Documentation to Qt Assistant" in the \link assistant.book Qt
    Assistant manual\endlink for further information.

    If Qt Assistant hasn't been \link openAssistant() opened\endlink
    yet, this function will do nothing. You can use isOpen() to
    determine whether Qt Assistant is up and running, or you can
    connect to the asssistantOpened() signal.

    \sa isOpen(), assistantOpened()
*/
void QAssistantClient::showPage( const QString &page )
{
    if ( !opened ) {
        pageBuffer = page;
        openAssistant();
        pageBuffer = QString::null;
        return;
    }
    QTextStream os( socket );
    os << page << "\n";
}

/*!
    \property QAssistantClient::open
    \brief whether Qt Assistant is open

*/
bool QAssistantClient::isOpen() const
{
    return opened;
}

void QAssistantClient::socketConnected()
{
    opened = true;
    if ( !pageBuffer.isEmpty() )
        showPage( pageBuffer );
    emit assistantOpened();
}

void QAssistantClient::socketConnectionClosed()
{
    opened = false;
    emit assistantClosed();
}

void QAssistantClient::socketError(QTcpSocket::SocketError err)
{
    if (err == QTcpSocket::ConnectionRefusedError)
        emit error( tr( "Could not connect to Assistant: Connection refused" ) );
    else if (err == QTcpSocket::HostNotFoundError)
        emit error( tr( "Could not connect to Assistant: Host not found" ) );
    else
        emit error( tr( "Communication error" ) );
}

void QAssistantClient::readStdError()
{
    QString errmsg = proc->readAllStandardError();

    if (!errmsg.isEmpty())
        emit error( errmsg.simplified() );
}

/*!
    \fn void QAssistantClient::setArguments(const QStringList &arguments)

    Sets the command line \a arguments used when Qt Assistant is
    started.
*/
void QAssistantClient::setArguments( const QStringList &args )
{
    QAssistantClientPrivate *d = data( this, true );
    d->arguments = args;
}

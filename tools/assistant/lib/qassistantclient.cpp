/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of the QAssistantClient library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qsocket.h>
#include <qtextstream.h>
#include <qprocess.h>
#include <qtimer.h>

#include "qassistantclient.h"

/*!
    \class QAssistantClient qassistantclient.h
    \brief The QAssistantClient class provides a means of using Qt
    Assistant as an application's help tool.

    Using Qt Assistant is simple: Create a QAssistantClient instance,
    then call showPage() as often as necessary to show your help
    pages. When you call showPage(), Qt Assistant will be launched if
    it isn't already running.

    The QAssistantClient instance can open (openAssistant()) or close
    (closeAssistant()) Qt Assistant whenever required. If Qt Assistant
    is open, isOpen() returns TRUE.

    One QAssistantClient instance interacts with one Qt Assistant
    instance, so every time you call openAssistant(), showPage() or
    closeAssistant() they are applied to the particular Qt Assistant
    instance associated with the QAssistantClient.

    When you call openAssistant() the assistantOpened() signal is
    emitted. Similarly when closeAssistant() is called,
    assistantClosed() is emitted. In either case, if an error occurs,
    error() is emitted.

    This class is not included in the Qt library itself. To use it you
    must link against \c libqassistantclient.so (Unix) or \c
    qassistantclient.lib (Windows), which is built into \c INSTALL/lib
    if you built the Qt tools (\c INSTALL is the directory where Qt is
    installed).

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
    closed. This happens when the user exits Qt Assistant, or when an
    error in the server or client occurs, or if closeAssistant() is
    called.
*/

/*!
    \fn void QAssistantClient::error( const QString &msg )

    This signal is emitted if Qt Assistant cannot be started or if an
    error occurs during the initialisation of the connection between
    Qt Assistant and the calling application. The \a msg provides an
    explanation of the error.
*/

/*!
    Constructs an assistant client object. The \a path specifies the
    path to the Qt Assistant executable. If the \a path is an empty
    string the system path (\c{%PATH%} or \c $PATH) is used.

    The assistant client object is a child of \a parent and is called
    \a name.
*/
QAssistantClient::QAssistantClient( const QString &path, QObject *parent, const char *name )
    : QObject( parent, name ), host ( "localhost" )
{
    if ( path.isEmpty() )
	assistantCommand = "assistant";
    else
	assistantCommand = path + "/assistant";

#if defined(Q_OS_MACX)
    assistantCommand += ".app/Contents/MacOS/assistant";
#endif

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

/*!
    Destroys the assistant client object and frees up all allocated
    resources.
*/
QAssistantClient::~QAssistantClient()
{
    if ( proc ) {
	proc->tryTerminate();
	proc->kill();
    }
}

/*!
    This function opens Qt Assistant and sets up the client-server
    communiction between the application and Qt Assistant. If it is
    already open, this function does nothing. If an error occurs,
    error() is emitted.

    \sa assistantOpened()
*/
void QAssistantClient::openAssistant()
{
    if ( proc )
	return;
    proc = new QProcess( this );
    proc->addArgument( assistantCommand );
    proc->addArgument( "-server" );
    if ( !proc->launch( QString::null ) ) {
	delete proc;
	proc = 0;
	emit error( tr( "Cannot start Qt Assistant" ) );
	return;
    }
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

/*!
    Use this function to close Qt Assistant.

    \sa assistantClosed()
*/
void QAssistantClient::closeAssistant()
{
    if ( !opened )
	return;
    proc->tryTerminate();
    proc->kill();
}

/*!
    Call this function to make Qt Assistant show a particular \a page.
    The \a page is a filename (e.g. \c myhelpfile.html). See "Adding
    Documentation to Qt Assistant" in the \link assistant.book Qt
    Assistant manual\endlink for further information.

    If Qt Assistant hasn't been \link openAssistant() opened\endlink,
    this function will open it.
*/
void QAssistantClient::showPage( const QString &page )
{
    if ( !opened ) {
	pageBuffer = page;
	openAssistant();
	return;
    }
    QTextStream os( socket );
    os << page << "\n";
}

/*!
    Returns TRUE is Qt Assistant is open; otherwise returns FALSE.
*/
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

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

  \brief The QAssistantClient class provides an easy way
  to use Qt Assistant from any application as help tool.

  In order to use Qt Assistant form your own application,
  the first step you have to do, is to create an instance
  of QAssistantClient. Afterwards call the function openAssistant()
  for actually opening the Qt Assistant. Each instance of
  QAssistantClient can only open/handle one Qt Assistant, no
  matter how often openAssistant() is called.

  To tell Qt Assistant which page should be displayed, use
  showPage().

  This class is not included in the Qt library itself. To
  use it you must link against libqassistantclient.so (Unix)
  or qassistantclient.lib (Windows), which is built into INSTALL/lib
  if you built the Qt tools (INSTALL is the directory where
  Qt is installed ).
*/

/*! \fn void QAssistantClient::assistantOpened()

  This signal is emmited when the Qt Assistant is open
  and the client-server communication is set up.
*/

/*! \fn void QAssistantClient::assistantClosed()

  This signal is emmited when the connection to the Qt
  Assistant is closed. This happens either when the Qt
  Assistant is quit manually, an error in the server or
  client occurs, or if the function closeAssistant() is
  called.
*/

/*! \fn void QAssistantClient::error( const QString &msg )

  This signal is emmited if the Qt Assistant cannot be started
  or an error occurs during the initialisation of the connection
  between Qt Assistant and the calling application.
*/

/*!
  Constructs a assistantclient object. The argument path
  specifies the path (respectively the directory) in the
  file system where the executable of the Qt Assistant is
  located. If the path is empty, QAssistantClient takes the
  Qt Assistant executable from the path system variable.

  The assistantclient object is a child of \a parent and is
  called \a name.
*/
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

/*!
  Destroys the assistantclient object and frees up all allocated
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
  This function opens the Qt Assistant and sets up the
  client-server communiction between an application and
  Qt Assistant.
  If it is already open, this function does nothing. If an
  error occurs, the signal error() is emmited.
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
  Use this function to close the Qt Assistant.
*/
void QAssistantClient::closeAssistant()
{
    if ( !opened )
	return;
    proc->tryTerminate();
    proc->kill();
}

/*!
  This function tells the Qt Assistant which page to display,
  \a page is the name of the file.
*/
void QAssistantClient::showPage( const QString &page )
{
    if ( !opened ) {
	pageBuffer = page;
	return;
    }
    QTextStream os( socket );
    os << page << "\n";
}

/*!
  Returns TRUE is the Qt Assistant is opened, otherwise FALSE.
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

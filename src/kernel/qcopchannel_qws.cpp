/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_qws.cpp#8 $
**
** Implementation of QCOP protocol
**
** Created : 000101
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcopchannel_qws.h"

#ifndef QT_NO_COP

#include "qwsdisplay_qws.h"
#include "qwscommand_qws.h"
#include "qwindowsystem_qws.h"
#include "qlist.h"
#include "qmap.h"

typedef QMap<QString, QList<QWSClient> > QCopServerMap;
static QCopServerMap *qcopServerMap = 0;

typedef QMap<QString, QList<QCopChannel> > QCopClientMap;
static QCopClientMap *qcopClientMap = 0;

class QCopChannel::Private
{
public:
    QCString channel;
};

/*! \class QCopChannel qcopchannel_qws.h

  \brief This class provides communication capabilities between several
  clients.

  The Qt Cop (QCOP) is a COmmunication Protocol, allowing clients to
  communicate inside of the same address space or between different processes.

  Currently, this facility is only available on Qt/Embedded as on X11
  and Windows we are exploring the use of existing standard such as
  DCOP and COM.

  QCopChannel contains important functions like send() and isRegistered()
  which are static and therefore usable without an object.

  In order to \e listen to the traffic on the channel, you should either
  subclass from QCopChannel and provide an re-implementation for receive(),
  or you should connect() to the received() signal.
 */

/*!
  Constructs a QCop channel and registers it with the server under the name
  \a channel. The standard \a parent and \a name arguments are passed on
  to the QObject constructor.
 */

QCopChannel::QCopChannel( const QCString& channel, QObject* parent, const char* name ) :
    QObject( parent, name )
{
    d = new QCopChannel::Private;
    d->channel = channel;

    if ( !qt_fbdpy ) {
	qFatal( "QCopChannel: Must construct a QApplication "
		"before QCopChannel" );
	return;
    }

    if ( !qcopClientMap )
	qcopClientMap = new QCopClientMap;

    // do we need a new channel list ?
    QCopClientMap::Iterator it = qcopClientMap->find( channel );
    if ( it != qcopClientMap->end() ) {
	it.data().append( this );
	return;
    }

    it = qcopClientMap->insert( channel, QList<QCopChannel>() );
    it.data().append( this );

    // inform server about this channel
    qt_fbdpy->registerChannel( channel );
}

/*!
  Destructs the client's side end of the channel and notifies the server
  about the closing. The server itself keeps the channel open until the
  last registered client detaches.
*/

QCopChannel::~QCopChannel()
{
    QCopClientMap::Iterator it = qcopClientMap->find( d->channel );
    Q_ASSERT( it != qcopClientMap->end() );
    it.data().removeRef( this );
    // still any clients connected locally ?
    if ( it.data().isEmpty() ) {
	QByteArray data;
	QDataStream s( data, IO_WriteOnly );
	s << d->channel;
	if ( qt_fbdpy )
	    send( "", "detach()", data );
	qcopClientMap->remove( d->channel );
    }

    delete d;
}

/*!
  Returns the name of the channel.
 */

QCString QCopChannel::channel() const
{
    return d->channel;
}

/*!
  This virtual function allows subclasses of QCopChannel to
  process data received from their channel.

  The default implementation emits the received() signal.

  Note that the format of \a data has to be well defined in order to
  demarshall the contained information.
  \sa send()
 */
void QCopChannel::receive( const QCString &msg, const QByteArray &data )
{
    emit received( msg, data );
}

/*!
  \fn void QCopChannel::received( const QCString &msg, const QByteArray &data )

  This signal is emitted whenever the receive() function gets incoming
  data.
*/

/*!
  Queries the server for the existance of \a channel.

  Returns TRUE if \a channel is registered.
 */

bool QCopChannel::isRegistered( const QCString& channel )
{
    QByteArray data;
    QDataStream s( data, IO_WriteOnly );
    s << channel;
    if ( !send( "", "isRegistered()", data ) )
	return FALSE;

    QWSQCopMessageEvent *e = qt_fbdpy->waitForQCopResponse();
    bool known = e->message == "known";
    delete e;
    return known;
}

/*!
  Send the message \a msg on \a channel. The message will be distributed
  to all clients subscribed to the channel.

  \sa receive()
 */

bool QCopChannel::send(const QCString &channel, const QCString &msg )
{
    QByteArray data;
    return send( channel, msg, data );
}

/*!
  Same as above function except the additional \a data parameter.
  QDataStream provides a convenient way to fill the byte array with
  auxiliary data.
 */

bool QCopChannel::send(const QCString &channel, const QCString &msg,
		       const QByteArray &data )
{
    if ( !qt_fbdpy ) {
	qFatal( "QCopChannel::send: Must construct a QApplication "
		"before using QCopChannel" );
	return FALSE;
    }

    qt_fbdpy->sendMessage( channel, msg, data );

    return TRUE;
}

/*!
  \internal
  Server side: subscribe client \a cl on channel \a ch.
 */

void QCopChannel::registerChannel( const QString &ch, const QWSClient *cl )
{
    if ( !qcopServerMap )
	qcopServerMap = new QCopServerMap;

    // do we need a new channel list ?
    QCopServerMap::Iterator it = qcopServerMap->find( ch );
    if ( it == qcopServerMap->end() )
	it = qcopServerMap->insert( ch, QList<QWSClient>() );

    it.data().append( cl );
}

/*!
  \internal
  Server side: unsubscribe \a cl from all channels.
 */

void QCopChannel::detach( const QWSClient *cl )
{
    if ( !qcopServerMap )
	return;

    QCopServerMap::Iterator it = qcopServerMap->begin();
    for ( ; it != qcopServerMap->end(); it++ )
	it.data().removeRef( cl );
}

/*!
  \internal
  Server side: transmit the message to all clients registered to the
  specified channel.
 */

void QCopChannel::answer( QWSClient *cl, const QCString &ch,
			  const QCString &msg, const QByteArray &data )
{
    // internal commands
    if ( ch.isEmpty() ) {
	if ( msg == "isRegistered()" ) {
	    QCString c;
	    QDataStream s( data, IO_ReadOnly );
	    s >> c;
	    bool known = qcopServerMap && qcopServerMap->contains( c );
	    QCString ans = known ? "known" : "unkown";
	    QWSServer::sendQCopEvent( cl, "", ans, data, TRUE );
	    return;
	} else if ( msg == "detach()" ) {
	    QCString c;
	    QDataStream s( data, IO_ReadOnly );
	    s >> c;
	    Q_ASSERT( qcopServerMap );
	    QCopServerMap::Iterator it = qcopServerMap->find( c );
	    if ( it != qcopServerMap->end() ) {
		Q_ASSERT( it.data().contains( cl ) );
		it.data().remove( cl );
		if ( it.data().isEmpty() )
		    qcopServerMap->remove( it );
	    }
	    return;
	}
	qWarning( "QCopChannel: unknown internal command %s", msg.data() );
	QWSServer::sendQCopEvent( cl, "", "bad", data );
	return;
    }

    QList<QWSClient> clist = (*qcopServerMap)[ ch ];
    if ( clist.isEmpty() ) {
	qWarning( "QCopChannel: no client registered for channel %s", ch.data() );
	return;
    }

    QWSClient *c = clist.first();
    for (; c != 0; c = clist.next() ) {
	QWSServer::sendQCopEvent( c, ch, msg, data );
    }
}

/*!
  \internal
  Client side: distribute received event to the QCop instance managing the
  channel.
 */
void QCopChannel::processEvent( const QCString &ch, const QCString &msg,
				const QByteArray &data )
{
    Q_ASSERT( qcopClientMap );

    // filter out internal events
    if ( ch.isEmpty() )
	return;

    // feed local clients with received data
    QList<QCopChannel> clients = (*qcopClientMap)[ ch ];
    for ( QCopChannel *p = clients.first(); p != 0; p = clients.next() )
	p->receive( msg, data );
}

#endif

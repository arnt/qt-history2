/****************************************************************************
** $Id$
**
** Implementation of QRemoteMessage class
**
** Created : 010301
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qremotemessage_p.h"
#include <assert.h>
#include <qsocket.h>
#include <qsocketdevice.h>

/*!
  \class QRemoteMessage qtestmessage.h
  \brief The QRemoteMessage class is a class that is used for exchanging messages between a
  Qt application and it's remote control. A typical use of such a remote functionality
  is testing. Hence the name QRemoteMessage.
  
	Detailed description

   <strong>Groups of functions:</strong>
  <ul>

  <li> Construction:
	QTestApplication(),
	~QTestApplication().

  </ul>
*/

uint QRemoteMessage::m_nextMsgId = 1;

/*!
    Constructs a default (empty) message.
*/

QRemoteMessage::QRemoteMessage()
{
    reset();
}

/*!
    Constructs a message based on event \a S and \a params.
*/

QRemoteMessage::QRemoteMessage(QString S, QString params)
{
    reset();

    // Prepare the struct
    m_msgId = m_nextMsgId++;
    m_event = S;
    m_params = params;
}

/*!
    Constructs a message based on event \a S, \a params and \a pixmap.
*/

QRemoteMessage::QRemoteMessage(QString S, QString params, const QPixmap *pixmap)
{
    reset();

    // Prepare the struct
    m_msgId = m_nextMsgId++;
    m_event = S;
    m_params = params;
    if (pixmap != 0) {
	m_pixmap = *pixmap;
    }
}

/*!
    Constructs a message based on event \a S, \a params and \a byteArray.
*/

QRemoteMessage::QRemoteMessage(QString S, QString params, const QByteArray *byteArray)
{
    reset();

    // Prepare the struct
    m_msgId = m_nextMsgId++;
    m_event = S;
    m_params = params;
    if (byteArray != 0) {
	m_byteArray = *byteArray;
    }
}

/*!
    Destroys the message.
*/

QRemoteMessage::~QRemoteMessage()
{
}

/*!
    Clears the message so that the same instance can be used for the reception of 
    a new message.
*/

void QRemoteMessage::reset()
{
    m_magicId = 0;
    m_size = 0;
    m_msgId = 0;
    m_isReply = FALSE;
    m_retValue = 0;
    m_event = "";
    m_params = "";
    m_pixmap.resize(0,0);
    m_byteArray.resize(0);
    primaryDataRead = FALSE;
}

/*!
    Returns a hardcoded value that is used for bitsynchronisation and validity check
    of the received data.
*/

uint QRemoteMessage::magicId()
{
    return 0xEDBA0000;
}

/*!
    Returns the smallest size of the message.
*/

uint QRemoteMessage::primarySize()
{
    // The size of the data part is the same as the size of the class minus 8 bytes
    // which are used to point to the VTABLE. This VTABLE pointer is hidden in the class
    // but it's always the first variable.
    return (sizeof(m_magicId) + sizeof(m_size));
}

/*!
    Sends the message over the \a socket connection.
*/

void QRemoteMessage::send(QSocket *socket)
{ 
    QDataStream	stream(socket);

    if ((socket->state() != QSocket::Connected) ||
	(socket->socketDevice()->error() != QSocketDevice::NoError)) {

	qDebug("Ignoring QRemoteMessage::send()... socket error!");
	return;
    }

    assert(m_event != "" && !m_event.isNull());

    m_magicId = magicId();

    stream << m_magicId;

    QByteArray dynaArray;
    QDataStream tmp(dynaArray,IO_WriteOnly);
    
    tmp << m_msgId;
    tmp << m_isReply;
    tmp << m_retValue;
    tmp << m_event;
    tmp << m_params;

    Q_UINT8 hasPixmap;
    if (!m_pixmap.isNull())
	hasPixmap = 1;
    else
	hasPixmap = 0;
    tmp << hasPixmap;
    if (hasPixmap != 0)
	tmp << m_pixmap;
    
    Q_UINT8 hasbyteArray;
    if (!m_byteArray.isNull())
	hasbyteArray = 1;
    else
	hasbyteArray = 0;
    tmp << hasbyteArray;
    if (hasbyteArray != 0)
	tmp << m_byteArray;

    tmp << m_magicId;
    
    m_size = dynaArray.count();

    stream << m_size;
    stream << dynaArray;
}

/*!
    Sends a reply message over the \a socket connection using the current message id 
    and the given \a retValue.
*/

void QRemoteMessage::reply(QSocket *socket, int retValue)
{
    m_pixmap.resize(0,0);
    m_event = "Reply";
    m_params = "";

    m_retValue = retValue;
    m_isReply = TRUE;
    send(socket);
}

/*!
    Fills the message with data received from the given \a socket connection.
    Returns TRUE if a complete and valid message was received.
*/

bool QRemoteMessage::receive(QSocket *socket)
{
    QDataStream	stream(socket);

    if (!primaryDataRead && (socket->size() >= (sizeof(m_magicId) + sizeof(m_size)))) {

	stream >> m_magicId;
	assert(m_magicId == magicId());
	stream >> m_size;

	primaryDataRead = TRUE;
    }

    if (primaryDataRead && (socket->size() >= m_size)) {

	QByteArray dynaArray;
	stream >> dynaArray;
	QDataStream tmp(dynaArray,IO_ReadOnly);

	tmp >> m_msgId;
	tmp >> m_isReply;
	assert((m_isReply >= 0) && (m_isReply <= 1));

	tmp >> m_retValue;
	tmp >> m_event;
	tmp >> m_params;

	Q_UINT8 hasPixmap;
	tmp >> hasPixmap;
	assert((hasPixmap >= 0) && (hasPixmap <= 1));
	m_pixmap.resize(0,0);
	if (hasPixmap == 1) 
		tmp >> m_pixmap;
	
	Q_UINT8 hasbyteArray;
	tmp >> hasbyteArray;
	assert((hasbyteArray >= 0) && (hasbyteArray <= 1));
	m_byteArray.resize(0);
	if (hasbyteArray == 1) 
	    tmp >> m_byteArray;
	tmp >> m_magicId;
	assert(magicId() == m_magicId);

	return TRUE;
    }

    return FALSE;
}

/*!
    Returns TRUE if the message contains a valid pixmap.
*/

bool QRemoteMessage::hasPixmap()
{
    return !m_pixmap.isNull();
}

/*!
    Returns the \a pixmap contained in the message.
    Returns TRUE if the message contains a valid pixmap.
*/

bool QRemoteMessage::getPixmap(QPixmap *&pixmap)
{
    if (!m_pixmap.isNull()) {

	pixmap = &m_pixmap;
	return TRUE;
    } else {

	return FALSE;
    }
}

/*!
    Returns TRUE if the message contains a valid ByteArray.
*/

bool QRemoteMessage::hasByteArray()
{
    return !m_byteArray.isNull();
}

/*!
    Returns the \a byteArray contained in the message.
    Returns TRUE if the byteArray is valid.
*/

bool QRemoteMessage::getByteArray(QByteArray *&byteArray)
{
    if (!m_byteArray.isNull()) {

	byteArray = &m_byteArray;
	return TRUE;
    } else {

	return FALSE;
    }
}

/*!
    Returns the param string contained in the message.
*/

QString QRemoteMessage::params()
{
    return m_params;
}

/*!
    Returns the id of the message.
    Each message transferred from the client to the remote control gets a unique id.
    If a remote controller replies to a message the same id is used.
    This way question and answer can be correlated.
*/

uint QRemoteMessage::msgId() 
{ 
    return m_msgId;
}

/*!
    Returns TRUE if the message is a reply message.
*/

bool QRemoteMessage::isReply() 
{ 
    return m_isReply != 0;
}

/*!
    Returns the returnValue from the reply message.
    If the message is not a reply the return value is undefined.
*/

int QRemoteMessage::retValue() 
{ 
    return m_retValue;
}

/*!
    Returns the event name of the message.
*/

QString QRemoteMessage::event() 
{ 
    return m_event;
}

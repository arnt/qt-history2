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

#ifndef QT_NO_REMOTE

#include <qsocket.h>
#include <qsocketdevice.h>

/*!
  \class QRemoteMessage qremotemessage.h
  \brief The QRemoteMessage class is used for exchanging messages between a
  Qt application and it's remote control.

  \internal

  \mainclass
  \ingroup io
*/

uint QRemoteMessage::next_msg_id = 1;

/*!
   \internal

    Constructs a default (empty) message.
*/

QRemoteMessage::QRemoteMessage()
{
    reset();
}

/*!
   \internal

    Constructs a message based on msg_event \a event and \a message.
*/

QRemoteMessage::QRemoteMessage(const QString &event, const QString &message)
{
    reset();

    // Prepare the struct
    msg_id = next_msg_id++;
    msg_event = event;
    msg = message;
}

/*
   \internal

    Constructs a message based on msg_event \a event, \a message and \a pixmap.
*/
/*
QRemoteMessage::QRemoteMessage(const QString &event, const QString &message, const QPixmap *pixmap)
{
    reset();

    // Prepare the struct
    msg_id = next_msg_id++;
    msg_event = event;
    msg = message;
    if (pixmap != 0) {
	internal_pixmap = *pixmap;
    }
}
*/

/*!
   \internal

    Constructs a message based on msg_event \a event, \a message and \a data.
*/

QRemoteMessage::QRemoteMessage(const QString &event, const QString &message, const QByteArray *data)
{
    reset();

    // Prepare the struct
    msg_id = next_msg_id++;
    msg_event = event;
    msg = message;
    if (data != 0) {
	internal_bytearray = *data;
    }
}

/*!
   \internal

    Destroys the message.
*/

QRemoteMessage::~QRemoteMessage()
{
}

/*!
   \internal

    Clears the message so that the same instance can be used for the reception of
    a new message.
*/

void QRemoteMessage::reset()
{
    magic_id = 0;
    size = 0;
    msg_id = 0;
    is_reply = FALSE;
    retvalue = "";
    msg_event = "";
    msg = "";
//    internal_pixmap.resize(0,0);
    internal_bytearray.resize(0);
    primary_data_read = FALSE;
}

/*!
   \internal

    Returns a hardcoded value that is used for bitsynchronisation and validity check
    of the received data.
*/

uint QRemoteMessage::magicId() const
{
    return 0xEDBA0000;
}

/*!
   \internal

    Returns the smallest size of the message.
*/

uint QRemoteMessage::primarySize()
{
    // The size of the data part is the same as the size of the class minus 8 bytes
    // which are used to point to the VTABLE. This VTABLE pointer is hidden in the class
    // but it's always the first variable.
    return (sizeof(magic_id) + sizeof(size));
}

/*!
   \internal

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

    Q_ASSERT(msg_event != "" && !msg_event.isNull());

    magic_id = magicId();

    stream << magic_id;

    QByteArray dynaArray;
    QDataStream tmp(dynaArray,IO_WriteOnly);

    tmp << msg_id;
    tmp << is_reply;
    tmp << retvalue;
    tmp << msg_event;
    tmp << msg;

/*
    Q_UINT8 hasPixmap;
    if (!internal_pixmap.isNull())
	hasPixmap = 1;
    else
	hasPixmap = 0;
    tmp << hasPixmap;
    if (hasPixmap != 0)
	tmp << internal_pixmap;
*/

    Q_UINT8 hasbyteArray;
    if (!internal_bytearray.isNull())
	hasbyteArray = 1;
    else
	hasbyteArray = 0;
    tmp << hasbyteArray;
    if (hasbyteArray != 0)
	tmp << internal_bytearray;

    tmp << magic_id;

    size = dynaArray.count();

    stream << size;
    stream << dynaArray;
}

/*!
   \internal

    Sends a reply message over the \a socket connection using the current message id
    and the given \a result.
*/

void QRemoteMessage::reply(QSocket *socket, const QString &result)
{
//    internal_pixmap.resize(0,0);
    msg_event = "Reply";
    msg = "";

    retvalue = result;
    is_reply = TRUE;
    send(socket);
}

/*!
   \internal

    Fills the message with data received from the given \a socket connection.
    Returns TRUE if a complete and valid message was received.
*/

bool QRemoteMessage::receive(QSocket *socket)
{
    QDataStream	stream(socket);

    if (!primary_data_read && (socket->size() >= (sizeof(magic_id) + sizeof(size)))) {

	stream >> magic_id;
	Q_ASSERT(magic_id == magicId());
	stream >> size;

	primary_data_read = TRUE;
    }

    if (primary_data_read && (socket->size() >= size)) {

	QByteArray dynaArray;
	stream >> dynaArray;
	QDataStream tmp(dynaArray,IO_ReadOnly);

	tmp >> msg_id;
	tmp >> is_reply;
	Q_ASSERT((is_reply >= 0) && (is_reply <= 1));

	tmp >> retvalue;
	tmp >> msg_event;
	tmp >> msg;

/*
	Q_UINT8 hasPixmap;
	tmp >> hasPixmap;
	internal_pixmap.resize(0,0);
	if (hasPixmap == 1)
		tmp >> internal_pixmap;
*/

	Q_UINT8 hasbyteArray;
	tmp >> hasbyteArray;
	internal_bytearray.resize(0);
	if (hasbyteArray == 1)
	    tmp >> internal_bytearray;
	tmp >> magic_id;
	Q_ASSERT(magicId() == magic_id);

	return TRUE;
    }

    return FALSE;
}

/*
   \internal

    Returns TRUE if the message contains a valid pixmap; otherwise
    returns FALSE.
*/
/*
bool QRemoteMessage::hasPixmap() const
{
    return !internal_pixmap.isNull();
}
*/
/*
    Returns the \a pixmap contained in the message.
    Returns TRUE if the message contains a valid pixmap.
*/
/*
bool QRemoteMessage::getPixmap(QPixmap *&pixmap)
{
    if (!internal_pixmap.isNull()) {

	pixmap = &internal_pixmap;
	return TRUE;
    } else {

	return FALSE;
    }
}
*/

/*!
   \internal

    Returns TRUE if the message contains a valid ByteArray; otherwise
    returns FALSE.
*/

bool QRemoteMessage::hasData() const
{
    return !internal_bytearray.isNull();
}

/*!
  \internal

    Returns the \a data contained in the message.
    Returns TRUE if the data is valid; otherwise returns FALSE.
*/

bool QRemoteMessage::getData(QByteArray *&data)
{
    if (!internal_bytearray.isNull()) {

	data = &internal_bytearray;
	return TRUE;
    } else {

	return FALSE;
    }
}

/*!
  \internal

    Returns the message.
*/

QString QRemoteMessage::message() const
{
    return msg;
}

/*!
  \internal

    Returns the id of the message.
    Each message transferred from the client to the remote control gets a unique id.
    If a remote controller replies to a message the same id is used.
    This way question and answer can be correlated.
*/

uint QRemoteMessage::messageId()  const
{
    return msg_id;
}

/*!
  \internal

    Returns TRUE if the message is a reply message; otherwise returns FALSE.
*/

bool QRemoteMessage::isReply() const
{
    return is_reply != 0;
}

/*!
  \internal

    Returns the returnValue from the reply message.
    If the message is not a reply the return value is undefined.
*/

QString QRemoteMessage::result() const
{
    return retvalue;
}

/*!
  \internal

    Returns the event description of the message.
*/

QString QRemoteMessage::event() const
{
    return msg_event;
}

#endif //QT_NO_REMOTE

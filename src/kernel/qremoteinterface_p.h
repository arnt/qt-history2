/****************************************************************************
** $Id$
**
** Definition of QRemoteInterface class
**
** Created : 010301
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
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

#ifndef QREMOTEINTERFACE_P_H
#define QREMOTEINTERFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Remote Control. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include <private/qcom_p.h>
#endif // QT_H

#ifndef QT_NO_REMOTE
#ifndef QT_NO_COMPONENT

#include <qobject.h>
class QString;
class QEvent;
class QRemoteClient;
class QPixmap;
class QSocket;


// {70EC01D4-2AB0-406F-B279-5BC348768C61}
#ifndef IID_QRemoteInterface
#define IID_QRemoteInterface QUuid( 0x70ec01d4, 0x2ab0, 0x406f, 0xb2, 0x79, 0x5b, 0xc3, 0x48, 0x76, 0x8c, 0x61)
#endif

// {3AC6E129-654A-4B7E-B1BA-85ECF379E182} 
#ifndef IID_QRemoteFactory
#define IID_QRemoteFactory QUuid( 0x3ac6e129, 0x654a, 0x4b7e, 0xb1, 0xba, 0x85, 0xec, 0xf3, 0x79, 0xe1, 0x82)
#endif


class Q_EXPORT QRemoteInterface : public QObject// public QUnknownInterface
{
    Q_OBJECT

public:
/*!
    Opens a connection to the remote controller (host). The connection is anticipated to
    be a socket connection, hence the parameters \a hostName and \a port.
    QRemoteInterface only defines the interface. The actual connect functionality must be
    implemented in a derived class.
*/
    virtual void open(const QString& hostname, int port) = 0;

/*!
    Returns TRUE if a connection has been set up to a remote control.
*/
    virtual bool isOpen() = 0;

/*!
    Closes the connection to the remote controller (host).
    QRemoteInterface only defines the interface. The actual close functionality must be
    implemented in a derived class.
*/
    virtual void close() = 0;

/*!
    Extends the functionality of QApp::notify() by appending remote functionality.
    Depending on the information provided by \a receiver and event \e certain actions
    may be taken.
    The function returns TRUE if the event has been handled completely, i.e. doesn't need
    to nor should be handled any more by QApplication::notify() itself.
    QRemoteInterface only defines the interface. The actual handleNotification functionality
    must be implemented in a derived class.
*/
    virtual bool handleNotification(QObject *receiver, QEvent * e) = 0;

/*!
    Saves a reference to the specified \a receiver.
    The reference can be used to communicate with the remote client code.
*/
    virtual void setRemoteClient(QRemoteClient *receiver) = 0;

/*!
    Posts (e.g. non blocking) an \a event and \a message to the remote controller (host).
    QRemoteInterface only defines the interface. The actual postObject functionality must be
    implemented in a derived class.
*/
    virtual void postObject(const QString &event, const QString &message) = 0;

/*!
    \Overload.
    The additional parameter \a data can be used to transfer a QByteArray to the remote
    controller (host).
*/
    virtual void postObject(const QString &event, const QString &message, const QByteArray *data) = 0;

/*!
    Sends (e.g. blocking) an \a event, \a message and \a data to the remote controller
    (host) and waits for a \a result. You can use \a timeout to specify the max wait time
    for the reply. If \a timeout == -1 the function waits forever.
    The \a result parameter returns the answer from the receiver of this message.
    QRemoteInterface only defines the interface. The actual sendObject functionality must be
    implemented in a derived class.
*/
    virtual bool sendObject(const QString &event, const QString &message, const QByteArray *data, QString &result, int timeout = -1) = 0;
};


struct Q_EXPORT QRemoteFactoryInterface : public QFeatureListInterface
{
    virtual QRemoteInterface* create( const QString& type ) = 0;
};

#endif //QT_NO_COMPONENT
#endif //QT_NO_REMOTE

#endif //QREMOTEINTERFACE_P_H

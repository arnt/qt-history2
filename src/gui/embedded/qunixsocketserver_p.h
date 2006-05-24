/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QUNIXSOCKETSERVER_H_
#define _QUNIXSOCKETSERVER_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>

class QUnixSocketServerPrivate;
class Q_GUI_EXPORT QUnixSocketServer : public QObject
{
    Q_OBJECT
public:
    enum ServerError { NoError, InvalidPath, ResourceError, BindError,
                       ListenError };

    QUnixSocketServer(QObject *parent=0);
    virtual ~QUnixSocketServer();

    void close();

    ServerError serverError() const;

    bool isListening() const;
    bool listen(const QByteArray & path);

    int socketDescriptor() const;
    QByteArray serverAddress() const;
   
    int maxPendingConnections() const; 
    void setMaxPendingConnections(int numConnections);

protected:
    virtual void incomingConnection(int socketDescriptor) = 0;

private:
    QUnixSocketServer(const QUnixSocketServer &);
    QUnixSocketServer & operator=(const QUnixSocketServer &);

    friend class QUnixSocketServerPrivate;
    QUnixSocketServerPrivate * d;
};


#endif // _QUNIXSOCKETSERVER_H_


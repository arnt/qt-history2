/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIP_H
#define QIP_H

#include <qnetworkprotocol.h>

class QSocket;

static const Q_UINT16 infoPort = 42417;


class Qip : public QNetworkProtocol
{
    Q_OBJECT

public:
    Qip();
    virtual int supportedOperations() const;

protected:
    virtual void operationListChildren( QNetworkOperation *op ); 
    virtual void operationGet( QNetworkOperation *op );
    virtual bool checkConnection( QNetworkOperation *op );

private slots:
    void socketConnected();
    void socketReadyRead();
    void socketConnectionClosed();
    void socketError( int code );

private:
    QSocket *socket;
    enum State { Start, List, Data } state;
    void error( int code, const QString& msg );
};



#endif // QIP_H


/****************************************************************************
** $Id: $
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#include <qnetworkprotocol.h>
#include <qurloperator.h>

#include "clientbase.h"

class QSocket;
class QTextEdit;
class QLineEdit;
class QListBox;
class QLabel;

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
};



class ClientInfo : public ClientInfoBase
{
    Q_OBJECT

public:
    ClientInfo();

private slots:
    void downloadFile();
    void newData( const QByteArray &ba );

private:
    QUrlOperator op;
    QString getOpenFileName();
};

#endif // CLIENT_H
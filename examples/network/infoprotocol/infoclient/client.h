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

#include "clientbase.h"

class QSocket;
class QTextEdit;
class QLineEdit;
class QListBox;
class QLabel;

static const Q_UINT16 infoPort = 42417;

class ClientInfo : public ClientInfoBase
{
    Q_OBJECT
public:
    ClientInfo( const QString &host, Q_UINT16 port );

private slots:
    void selectItem( const QString& item );
    void stepBack();
    void sendToServer( const QString& line );
    void socketConnected();
    void socketReadyRead();
    void socketConnectionClosed();
    void socketError( int code );

private:
    QSocket *socket;
};

#endif // CLIENT_H
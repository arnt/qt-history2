/****************************************************************************
** $Id: $
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <qsocket.h>
#include <qserversocket.h>
#include <qvbox.h>
#include <qdict.h>

#include "serverbase.h"

class QTextView;

static const Q_UINT16 infoPort = 42417;

// The InfoData class manages data, organized in tree structure.
class InfoData
{
public:
    InfoData();
    QStringList* list( const QString& path ) const;
    QStringList* get( const QString& path ) const;

private:
    QDict< QStringList > nodes;
    QDict< QStringList > data;
};


/*
  The ClientSocket class provides a socket that is connected with a client.
  For every client that connects to the server, the server creates a new
  instance of this class.
*/
class ClientSocket : public QSocket
{
    Q_OBJECT
public:
    ClientSocket( int sock, InfoData *i, QObject *parent = 0, const char *name = 0 );

private slots:
    void readClient();
    void connectionClosed();

private:
    bool processCommand( const QString& command, QStringList *answer );
    InfoData *info;
};


/*
  The SimpleServer class handles new connections to the server. For every
  client that connects, it creates a new ClientSocket -- that instance is now
  responsible for the communication with that client.
*/
class SimpleServer : public QServerSocket
{
    Q_OBJECT
public:
    SimpleServer( InfoData *i, Q_UINT16 port = infoPort, QObject* parent = 0 );
    void newConnection( int socket );

signals:
    void newConnect();

private:
    InfoData *info;
};


/*
  The ServerInfo class provides a small GUI for the server. It also creates the
  SimpleServer and as a result the server.
*/
class ServerInfo : public ServerInfoBase
{
    Q_OBJECT
public:
    ServerInfo();

private slots:
    void newConnect();

private:
    InfoData info;
};


#endif //SERVER_H
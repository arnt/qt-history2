/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef NNTP_H
#define NNTP_H

#include <qsocket.h>
#include <qnetworkprotocol.h>

class Nntp : public QNetworkProtocol
{
    Q_OBJECT

public:
    Nntp();
    virtual ~Nntp();
    virtual int supportedOperations() const;

protected:
    virtual void operationListChildren( QNetworkOperation *op );
    virtual void operationGet( QNetworkOperation *op );

    QSocket *commandSocket;
    bool connectionReady;
    bool readGroups;
    bool readHead;
    bool readBody;
    
private:
    bool checkConnection( QNetworkOperation *op );
    void close();
    void parseGroups();
    void readArticle();
    
protected slots:
    void hostFound();
    void connected();
    void closed();
    void readyRead();
    
};

#endif

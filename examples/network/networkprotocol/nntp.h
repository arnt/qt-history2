/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    bool readArticle;

private:
    bool checkConnection( QNetworkOperation *op );
    void close();
    void parseGroups();
    void parseArticle();

protected slots:
    void hostFound();
    void connected();
    void closed();
    void readyRead();
    void error( int );

};

#endif

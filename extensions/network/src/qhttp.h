/****************************************************************************
** $Id$
**
** Implementation of Network Extension Library
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QHTTP_H
#define QHTTP_H

#include "qsocket.h"
#include "qapplication.h"
#include "qstring.h"
#include "qurlinfo.h"
#include "qnetworkprotocol.h"
#include "qurloperator.h"

class QHttp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QHttp();
    virtual ~QHttp();
    virtual int supportedOperations() const;

protected:
    virtual void operationGet( QNetworkOperation *op );
    virtual void operationPut( QNetworkOperation *op );

    QSocket *commandSocket;
    bool connectionReady, passiveMode;

private:
    bool checkConnection( QNetworkOperation *op );
    void close();

protected slots:
    void hostFound();
    void connected();
    void closed();
    void readyRead();

};

#endif

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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFTP_H
#define QFTP_H

#ifndef QT_H
#include "qsocket.h"
#include "qapplication.h"
#include "qstring.h"
#include "qstringlist.h"

#include "qurlinfo.h"
#include "qnetworkprotocol.h"
#endif // QT_H

#ifdef QT_FEATURE_NETWORKPROTOCOL_FTP

class QFtp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QFtp();
    virtual ~QFtp();
    virtual int supportedOperations() const;

protected:
    void parseDir( const QString &buffer, QUrlInfo &info );
    virtual void operationListChildren( QNetworkOperation *op );
    virtual void operationMkDir( QNetworkOperation *op );
    virtual void operationRemove( QNetworkOperation *op );
    virtual void operationRename( QNetworkOperation *op );
    virtual void operationGet( QNetworkOperation *op );
    virtual void operationPut( QNetworkOperation *op );

    QSocket *commandSocket, *dataSocket;
    bool connectionReady, passiveMode;
    int getTotalSize, getDoneSize;
    bool startGetOnFail;
    int putToWrite, putWritten, putOffset;
    bool errorInListChildren;

private:
    bool checkConnection( QNetworkOperation *op );
    void close();
    void reinitCommandSocket();
    void okButTryLater( int code, const QCString &data );
    void okGoOn( int code, const QCString &data );
    void okButNeedMoreInfo( int code, const QCString &data );
    void errorForNow( int code, const QCString &data );
    void errorForgetIt( int code, const QCString &data );

protected slots:
    void hostFound();
    void connected();
    void closed();
    void readyRead();
    void dataHostFound();
    void dataConnected();
    void dataClosed();
    void dataReadyRead();
    void dataBytesWritten( int nbytes );
    void error( int );

};

#endif // QT_FEATURE_NETWORKPROTOCOL_FTP

#endif // QFTP_H

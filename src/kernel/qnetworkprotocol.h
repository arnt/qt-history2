/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetworkprotocol.h#26 $
**
** Implementation of QNetworkProtocol class
**
** Created : 950429
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

#ifndef QNETWORKPROTOCOL_H
#define QNETWORKPROTOCOL_H

#include "qurlinfo.h"

#include <qstring.h>
#include <qdict.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qqueue.h>

class QNetworkProtocol;
class QNetworkOperation;
class QTimer;
struct QNetworkProtocolPrivate;
class QUrlOperator;

class Q_EXPORT QNetworkProtocolFactoryBase
{
public:
   virtual QNetworkProtocol *createObject() = 0;

};

template< class Protocol >
class QNetworkProtocolFactory : public QNetworkProtocolFactoryBase
{
public:
    QNetworkProtocol *createObject() {
	return new Protocol;
    }

};

typedef QDict< QNetworkProtocolFactoryBase > QNetworkProtocolDict;
extern Q_EXPORT QNetworkProtocolDict *qNetworkProtocolRegister;

class Q_EXPORT QNetworkProtocol : public QObject
{
    Q_OBJECT

public:
    enum State {
	StWaiting = 0,
	StInProgress,
	StDone,
	StFailed
    };

    enum Operation {
	OpListChildren = 1,
	OpMkdir = 2,
	OpRemove = 4,
	OpRename = 8,
	OpGet = 32,
	OpPut = 64
    };

    enum ConnectionState {
	ConHostFound,
	ConConnected,
	ConClosed
    };

    enum Error {
	// no error
	NoError = 0,
	// general errors
	ErrValid = -1,
	ErrUnknownProtocol = -2,
	ErrUnsupported = -3,
	ErrParse = -4,
	// errors on connect
	ErrLoginIncorrect = -5,
	ErrHostNotFound = -6,
	// protocol errors
	ErrListChlidren = -7,
	ErrMkdir = -8,
	ErrRemove = -9,
	ErrRename = -10,
	ErrGet = -11,
	ErrPut = -12
    };

    QNetworkProtocol();
    virtual ~QNetworkProtocol();

    virtual void setUrl( QUrlOperator *u );

    virtual void setAutoDelete( bool b, int i = 10000 );
    bool autoDelete() const;

    virtual const QNetworkOperation *listChildren();
    virtual const QNetworkOperation *mkdir( const QString &dirname );
    virtual const QNetworkOperation *remove( const QString &filename );
    virtual const QNetworkOperation *rename( const QString &oldname, const QString &newname );
    virtual const QNetworkOperation *get();
    virtual const QNetworkOperation *put( const QByteArray &data );

    static void registerNetworkProtocol( const QString &protocol,
					 QNetworkProtocolFactoryBase *protocolFactory );
    static QNetworkProtocol *getNetworkProtocol( const QString &protocol );
    static bool hasOnlyLocalFileSystem();

    virtual int supportedOperations() const;
    virtual void addOperation( QNetworkOperation *op );

    QUrlOperator *url() const;
    QNetworkOperation *operationInProgress() const;
    void clearOperationQueue();

signals:
    void data( const QByteArray &, QNetworkOperation *res );
    void connectionStateChanged( int state, const QString &data );
    void finished( QNetworkOperation *res );
    void start( QNetworkOperation *res );
    void newChild( const QUrlInfo &, QNetworkOperation *res );
    void createdDirectory( const QUrlInfo &, QNetworkOperation *res );
    void removed( QNetworkOperation *res );
    void itemChanged( QNetworkOperation *res );

protected:
    virtual void processOperation( QNetworkOperation *op );
    virtual void operationListChildren( QNetworkOperation *op );
    virtual void operationMkDir( QNetworkOperation *op );
    virtual void operationRemove( QNetworkOperation *op );
    virtual void operationRename( QNetworkOperation *op );
    virtual void operationGet( QNetworkOperation *op );
    virtual void operationPut( QNetworkOperation *op );
    virtual bool checkConnection( QNetworkOperation *op );

private:
    QNetworkProtocolPrivate *d;

private slots:
    void processNextOperation( QNetworkOperation *old );
    void startOps();

    void emitNewChild( const QUrlInfo &, QNetworkOperation *res );
    void emitFinished( QNetworkOperation *res );
    void emitStart( QNetworkOperation *res );
    void emitCreatedDirectory( const QUrlInfo &, QNetworkOperation *res );
    void emitRemoved( QNetworkOperation *res );
    void emitItemChanged( QNetworkOperation *res );
    void emitData( const QByteArray &, QNetworkOperation *res );
    void removeMe();

};

struct QNetworkOperationPrivate;
class Q_EXPORT QNetworkOperation
{
public:
    QNetworkOperation( QNetworkProtocol::Operation operation,
		    const QString &arg1, const QString &arg2,
		    const QString &arg3 );
    QNetworkOperation( QNetworkProtocol::Operation operation,
		    const QByteArray &arg1, const QByteArray &arg2,
		    const QByteArray &arg3 );
    ~QNetworkOperation();

    void setState( QNetworkProtocol::State state );
    void setProtocolDetail( const QString &detail );
    void setErrorCode( QNetworkProtocol::Error ec );
    void setArg1( const QString &arg );
    void setArg2( const QString &arg );
    void setArg3( const QString &arg );
    void setRawArg1( const QByteArray &arg );
    void setRawArg2( const QByteArray &arg );
    void setRawArg3( const QByteArray &arg );

    QNetworkProtocol::Operation operation() const;
    QNetworkProtocol::State state() const;
    QString arg1() const;
    QString arg2() const;
    QString arg3() const;
    QByteArray rawArg1() const;
    QByteArray rawArg2() const;
    QByteArray rawArg3() const;
    QString protocolDetail() const;
    QNetworkProtocol::Error errorCode() const;

private:
    QNetworkOperationPrivate *d;

};

#endif

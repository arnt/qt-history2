/****************************************************************************
** $Id: $
**
** Definition of Q3NetworkProtocol class
**
** Created : 950429
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QNETWORKPROTOCOL_H
#define QNETWORKPROTOCOL_H

#ifndef QT_H
#include "qurlinfo.h"
#include "qstring.h"
#include "q3dict.h"
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_NETWORKPROTOCOL

class Q3NetworkProtocol;
class Q3NetworkOperation;
class QTimer;
class Q3UrlOperator;
class Q3NetworkProtocolPrivate;
template <class T> class Q3ValueList;

class Q_COMPAT_EXPORT Q3NetworkProtocolFactoryBase
{
public:
   virtual Q3NetworkProtocol *createObject() = 0;

};

template< class Protocol >
class Q3NetworkProtocolFactory : public Q3NetworkProtocolFactoryBase
{
public:
    Q3NetworkProtocol *createObject() {
	return new Protocol;
    }

};

typedef Q3Dict< Q3NetworkProtocolFactoryBase > Q3NetworkProtocolDict;

class Q_COMPAT_EXPORT Q3NetworkProtocol : public QObject
{
    Q_OBJECT

public:
    enum State {
	StWaiting = 0,
	StInProgress,
	StDone,
	StFailed,
	StStopped
    };

    enum Operation {
	OpListChildren = 1,
	OpMkDir = 2,
	OpMkdir = OpMkDir, // ### remove in 4.0
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
	ErrValid,
	ErrUnknownProtocol,
	ErrUnsupported,
	ErrParse,
	// errors on connect
	ErrLoginIncorrect,
	ErrHostNotFound,
	// protocol errors
	ErrListChildren,
	ErrListChlidren = ErrListChildren, // ### remove in 4.0
	ErrMkDir,
	ErrMkdir = ErrMkDir, // ### remove in 4.0
	ErrRemove,
	ErrRename,
	ErrGet,
	ErrPut,
	ErrFileNotExisting,
	ErrPermissionDenied
    };

    Q3NetworkProtocol();
    virtual ~Q3NetworkProtocol();

    virtual void setUrl( Q3UrlOperator *u );

    virtual void setAutoDelete( bool b, int i = 10000 );
    bool autoDelete() const;

    static void registerNetworkProtocol( const QString &protocol,
					 Q3NetworkProtocolFactoryBase *protocolFactory );
    static Q3NetworkProtocol *getNetworkProtocol( const QString &protocol );
    static bool hasOnlyLocalFileSystem();

    virtual int supportedOperations() const;
    virtual void addOperation( Q3NetworkOperation *op );

    Q3UrlOperator *url() const;
    Q3NetworkOperation *operationInProgress() const;
    virtual void clearOperationQueue();
    virtual void stop();

signals:
    void data( const QByteArray &, Q3NetworkOperation *res );
    void connectionStateChanged( int state, const QString &data );
    void finished( Q3NetworkOperation *res );
    void start( Q3NetworkOperation *res );
    void newChildren( const Q3ValueList<QUrlInfo> &, Q3NetworkOperation *res );
    void newChild( const QUrlInfo &, Q3NetworkOperation *res );
    void createdDirectory( const QUrlInfo &, Q3NetworkOperation *res );
    void removed( Q3NetworkOperation *res );
    void itemChanged( Q3NetworkOperation *res );
    void dataTransferProgress( int bytesDone, int bytesTotal, Q3NetworkOperation *res );

protected:
    virtual void processOperation( Q3NetworkOperation *op );
    virtual void operationListChildren( Q3NetworkOperation *op );
    virtual void operationMkDir( Q3NetworkOperation *op );
    virtual void operationRemove( Q3NetworkOperation *op );
    virtual void operationRename( Q3NetworkOperation *op );
    virtual void operationGet( Q3NetworkOperation *op );
    virtual void operationPut( Q3NetworkOperation *op );
    virtual void operationPutChunk( Q3NetworkOperation *op );
    virtual bool checkConnection( Q3NetworkOperation *op );

private:
    Q3NetworkProtocolPrivate *d;

private slots:
    void processNextOperation( Q3NetworkOperation *old );
    void startOps();
    void emitNewChildren( const QUrlInfo &i, Q3NetworkOperation *op );

    void removeMe();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3NetworkProtocol( const Q3NetworkProtocol & );
    Q3NetworkProtocol &operator=( const Q3NetworkProtocol & );
#endif
};

class Q3NetworkOperationPrivate;

class Q_COMPAT_EXPORT Q3NetworkOperation : public QObject
{
    Q_OBJECT
    friend class Q3UrlOperator;

public:
    Q3NetworkOperation( Q3NetworkProtocol::Operation operation,
		    const QString &arg0, const QString &arg1,
		    const QString &arg2 );
    Q3NetworkOperation( Q3NetworkProtocol::Operation operation,
		    const QByteArray &arg0, const QByteArray &arg1,
		    const QByteArray &arg2 );
    ~Q3NetworkOperation();

    void setState( Q3NetworkProtocol::State state );
    void setProtocolDetail( const QString &detail );
    void setErrorCode( int ec );
    void setArg( int num, const QString &arg );
    void setRawArg( int num, const QByteArray &arg );

    Q3NetworkProtocol::Operation operation() const;
    Q3NetworkProtocol::State state() const;
    QString arg( int num ) const;
    QByteArray rawArg( int num ) const;
    QString protocolDetail() const;
    int errorCode() const;

    void free();

private slots:
    void deleteMe();

private:
    QByteArray &raw( int num ) const;
    Q3NetworkOperationPrivate *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3NetworkOperation( const Q3NetworkOperation & );
    Q3NetworkOperation &operator=( const Q3NetworkOperation & );
#endif
};

#endif // QT_NO_NETWORKPROTOCOL

#endif // QNETWORKPROTOCOL_H

/****************************************************************************
**
** Definition of QNetworkProtocol class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QNETWORKPROTOCOL_H
#define QNETWORKPROTOCOL_H

#ifndef QT_H
#include "qurlinfo.h"
#include "qstring.h"
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_NETWORKPROTOCOL

class QNetworkProtocol;
class QNetworkOperation;
class QTimer;
class QUrlOperator;
class QNetworkProtocolPrivate;
template <class T> class QList;

class Q_COMPAT_EXPORT QNetworkProtocolFactoryBase
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

class Q_COMPAT_EXPORT QNetworkProtocol : public QObject
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
        OpMkdir = OpMkDir,
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
        ErrListChlidren = ErrListChildren,
        ErrMkDir,
        ErrMkdir = ErrMkDir,
        ErrRemove,
        ErrRename,
        ErrGet,
        ErrPut,
        ErrFileNotExisting,
        ErrPermissionDenied
    };

    QNetworkProtocol(QObject *parent = 0, const char *name = 0);
    virtual ~QNetworkProtocol();

    virtual void setUrl(QUrlOperator *u);

    virtual void setAutoDelete(bool b, int i = 10000);
    bool autoDelete() const;

    static void registerNetworkProtocol(const QString &protocol,
                                         QNetworkProtocolFactoryBase *protocolFactory);
    static QNetworkProtocol *getNetworkProtocol(const QString &protocol);
    static bool hasOnlyLocalFileSystem();

    virtual int supportedOperations() const;
    virtual void addOperation(QNetworkOperation *op);

    QUrlOperator *url() const;
    QNetworkOperation *operationInProgress() const;
    virtual void clearOperationQueue();
    virtual void stop();

signals:
    void data(const QByteArray &, QNetworkOperation *res);
    void connectionStateChanged(int state, const QString &data);
    void finished(QNetworkOperation *res);
    void start(QNetworkOperation *res);
    void newChildren(const QList<QUrlInfo> &, QNetworkOperation *res);
    void newChild(const QUrlInfo &, QNetworkOperation *res);
    void createdDirectory(const QUrlInfo &, QNetworkOperation *res);
    void removed(QNetworkOperation *res);
    void itemChanged(QNetworkOperation *res);
    void dataTransferProgress(int bytesDone, int bytesTotal, QNetworkOperation *res);

protected:
    virtual void processOperation(QNetworkOperation *op);
    virtual void operationListChildren(QNetworkOperation *op);
    virtual void operationMkDir(QNetworkOperation *op);
    virtual void operationRemove(QNetworkOperation *op);
    virtual void operationRename(QNetworkOperation *op);
    virtual void operationGet(QNetworkOperation *op);
    virtual void operationPut(QNetworkOperation *op);
    virtual void operationPutChunk(QNetworkOperation *op);
    virtual bool checkConnection(QNetworkOperation *op);

private:
    QNetworkProtocolPrivate *d;

private slots:
    void processNextOperation(QNetworkOperation *old);
    void startOps();
    void emitNewChildren(const QUrlInfo &i, QNetworkOperation *op);

    void removeMe();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QNetworkProtocol(const QNetworkProtocol &);
    QNetworkProtocol &operator=(const QNetworkProtocol &);
#endif
};

class QNetworkOperationPrivate;

class Q_COMPAT_EXPORT QNetworkOperation : public QObject
{
    Q_OBJECT
    friend class QUrlOperator;

public:
    QNetworkOperation(QNetworkProtocol::Operation operation,
                    const QString &arg0, const QString &arg1,
                    const QString &arg2);
    QNetworkOperation(QNetworkProtocol::Operation operation,
                    const QByteArray &arg0, const QByteArray &arg1,
                    const QByteArray &arg2);
    ~QNetworkOperation();

    void setState(QNetworkProtocol::State state);
    void setProtocolDetail(const QString &detail);
    void setErrorCode(int ec);
    void setArg(int num, const QString &arg);
    void setRawArg(int num, const QByteArray &arg);

    QNetworkProtocol::Operation operation() const;
    QNetworkProtocol::State state() const;
    QString arg(int num) const;
    QByteArray rawArg(int num) const;
    QString protocolDetail() const;
    int errorCode() const;

    void free();

private slots:
    void deleteMe();

private:
    QByteArray &raw(int num) const;
    QNetworkOperationPrivate *d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QNetworkOperation(const QNetworkOperation &);
    QNetworkOperation &operator=(const QNetworkOperation &);
#endif
};

#endif // QT_NO_NETWORKPROTOCOL

#endif // QNETWORKPROTOCOL_H

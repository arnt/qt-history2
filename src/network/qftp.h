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

#ifndef QFTP_H
#define QFTP_H

#include "QtCore/qstring.h" // char*->QString conversion
#include "QtNetwork/qurlinfo.h"
#include "QtCore/qobject.h"

#ifndef QT_NO_NETWORKPROTOCOL_FTP

class QFtpPrivate;

class Q_NETWORK_EXPORT QFtp : public QObject
{
    Q_OBJECT

public:
    QFtp(QObject *parent = 0);
    virtual ~QFtp();

    enum State {
        Unconnected,
        HostLookup,
        Connecting,
        Connected,
        LoggedIn,
        Closing
    };
    enum Error {
        NoError,
        UnknownError,
        HostNotFound,
        ConnectionRefused,
        NotConnected
    };
    enum Command {
        None,
        SetTransferMode,
        SetProxy,
        ConnectToHost,
        Login,
        Close,
        List,
        Cd,
        Get,
        Put,
        Remove,
        Mkdir,
        Rmdir,
        Rename,
        RawCommand
    };
    enum TransferMode {
        Active,
        Passive
    };

    int setProxy(const QString &host, Q_UINT16 port);
    int connectToHost(const QString &host, Q_UINT16 port=21);
    int login(const QString &user = QString(), const QString &password = QString());
    int close();
    int setTransferMode(TransferMode mode);
    int list(const QString &dir = QString());
    int cd(const QString &dir);
    int get(const QString &file, QIODevice *dev=0);
    int put(const QByteArray &data, const QString &file);
    int put(QIODevice *dev, const QString &file);
    int remove(const QString &file);
    int mkdir(const QString &dir);
    int rmdir(const QString &dir);
    int rename(const QString &oldname, const QString &newname);

    int rawCommand(const QString &command);

    Q_LONGLONG bytesAvailable() const;
#ifdef QT_COMPAT
    inline QT_COMPAT Q_LONG readBlock(char *data, Q_ULONG maxlen)
    { return Q_LONG(read(data, Q_LONGLONG(maxlen))); }
#endif
    Q_LONGLONG read(char *data, Q_LONGLONG maxlen);
    QByteArray readAll();

    int currentId() const;
    QIODevice* currentDevice() const;
    Command currentCommand() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    State state() const;

    Error error() const;
    QString errorString() const;

public slots:
    void abort();

signals:
    void stateChanged(int);
    void listInfo(const QUrlInfo&);
    void readyRead();
    void dataTransferProgress(Q_LONGLONG, Q_LONGLONG);
    void rawCommandReply(int, const QString&);

    void commandStarted(int);
    void commandFinished(int, bool);
    void done(bool);

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QFtp(QObject *parent, const char *name);
#endif

private:
    Q_DISABLE_COPY(QFtp)
    Q_DECLARE_PRIVATE(QFtp)

    Q_PRIVATE_SLOT(d, void startNextCommand())
    Q_PRIVATE_SLOT(d, void piFinished(const QString&))
    Q_PRIVATE_SLOT(d, void piError(int, const QString&))
    Q_PRIVATE_SLOT(d, void piConnectState(int))
    Q_PRIVATE_SLOT(d, void piFtpReply(int, const QString&))
};

#endif // QT_NO_NETWORKPROTOCOL_FTP
#endif

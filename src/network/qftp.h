/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFTP_H
#define QFTP_H

#include <QtCore/qstring.h>
#include <QtNetwork/qurlinfo.h>
#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

#ifndef QT_NO_FTP

class QFtpPrivate;

class Q_NETWORK_EXPORT QFtp : public QObject
{
    Q_OBJECT

public:
    explicit QFtp(QObject *parent = 0);
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
    enum TransferType {
        Binary,
        Ascii
    };

    int setProxy(const QString &host, quint16 port);
    int connectToHost(const QString &host, quint16 port=21);
    int login(const QString &user = QString(), const QString &password = QString());
    int close();
    int setTransferMode(TransferMode mode);
    int list(const QString &dir = QString());
    int cd(const QString &dir);
    int get(const QString &file, QIODevice *dev=0, TransferType type = Binary);
    int put(const QByteArray &data, const QString &file, TransferType type = Binary);
    int put(QIODevice *dev, const QString &file, TransferType type = Binary);
    int remove(const QString &file);
    int mkdir(const QString &dir);
    int rmdir(const QString &dir);
    int rename(const QString &oldname, const QString &newname);

    int rawCommand(const QString &command);

    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT qint64 readBlock(char *data, quint64 maxlen)
    { return read(data, qint64(maxlen)); }
#endif
    QByteArray readAll();

    int currentId() const;
    QIODevice* currentDevice() const;
    Command currentCommand() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    State state() const;

    Error error() const;
    QString errorString() const;

public Q_SLOTS:
    void abort();

Q_SIGNALS:
    void stateChanged(int);
    void listInfo(const QUrlInfo&);
    void readyRead();
    void dataTransferProgress(qint64, qint64);
    void rawCommandReply(int, const QString&);

    void commandStarted(int);
    void commandFinished(int, bool);
    void done(bool);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QFtp(QObject *parent, const char *name);
#endif

private:
    Q_DISABLE_COPY(QFtp)
    Q_DECLARE_PRIVATE(QFtp)

    Q_PRIVATE_SLOT(d_func(), void _q_startNextCommand())
    Q_PRIVATE_SLOT(d_func(), void _q_piFinished(const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_piError(int, const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_piConnectState(int))
    Q_PRIVATE_SLOT(d_func(), void _q_piFtpReply(int, const QString&))
};

#endif // QT_NO_FTP

QT_END_HEADER

#endif // QFTP_H

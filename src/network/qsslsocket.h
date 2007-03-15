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

/****************************************************************************
**
** In addition, as a special exception, Trolltech gives permission to link
** the code of its release of Qt with the OpenSSL project's "OpenSSL" library
** (or modified versions of the "OpenSSL" library that use the same license
** as the original version), and distribute the linked executables.
**
** You must comply with the GNU General Public License version 2 in all
** respects for all of the code used other than the "OpenSSL" code.  If you
** modify this file, you may extend this exception to your version of the file,
** but you are not obligated to do so.  If you do not wish to do so, delete
** this exception statement from your version of this file.
**
****************************************************************************/

#ifndef QSSLSOCKET_H
#define QSSLSOCKET_H

#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

#ifndef QT_NO_OPENSSL

#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qsslerror.h>

class QDir;
class QSslCipher;
class QSslCertificate;

Q_DECLARE_METATYPE(QList<QSslError>)

class QSslSocketPrivate;
class Q_NETWORK_EXPORT QSslSocket : public QTcpSocket
{
    Q_OBJECT
public:
    enum Mode {
        PlainMode,
        SslClientMode,
        SslServerMode
    };

    enum Protocol {
        SslV3,
        SslV2,
        TlsV1,
        Compat
    };

    QSslSocket(QObject *parent = 0);
    ~QSslSocket();

    // Autostarting the SSL client handshake.
    void connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode mode = ReadWrite);
    bool setSocketDescriptor(int socketDescriptor, SocketState state, OpenMode openMode);

    Mode mode() const;
    bool isEncrypted() const;

    Protocol protocol() const;
    void setProtocol(Protocol protocol);

    // From QIODevice
    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool canReadLine() const;
    void close();
    bool atEnd() const;
    bool flush();
    void abort();

    // Certificate & cipher accessors.
    void setLocalCertificate(const QSslCertificate &certificate);
    QSslCertificate localCertificate() const;
    QSslCertificate peerCertificate() const;
    QList<QSslCertificate> peerCertificateChain() const;
    QSslCipher currentCipher() const;

    // Private keys, for server sockets.
    void setPrivateKey(const QSslKey &key);
    QSslKey privateKey() const;

    // Cipher settings.
    QList<QSslCipher> ciphers() const;
    void resetCiphers();
    void setCiphers(const QList<QSslCipher> &ciphers);
    static void setGlobalCiphers(const QList<QSslCipher> &ciphers);
    static void resetGlobalCiphers();
    static QList<QSslCipher> globalCiphers();
    static QList<QSslCipher> supportedCiphers();

    // CA settings.
    bool addCaCertificates(const QString &path);
    void addCaCertificate(const QSslCertificate &certificate);
    void addCaCertificates(const QList<QSslCertificate> &certificates);
    void setCaCertificates(const QList<QSslCertificate> &certificates);
    void resetCaCertificates();
    QList<QSslCertificate> caCertificates() const;
    static bool addGlobalCaCertificates(const QString &path);
    static void addGlobalCaCertificate(const QSslCertificate &certificate);
    static void addGlobalCaCertificates(const QList<QSslCertificate> &certificates);
    static void setGlobalCaCertificates(const QList<QSslCertificate> &certificates);
    static QList<QSslCertificate> globalCaCertificates();
    static QList<QSslCertificate> systemCaCertificates();

    bool waitForConnected(int msecs = 30000);
    bool waitForEncrypted(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000);
    bool waitForBytesWritten(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

    static bool supportsSsl();

public Q_SLOTS:
    void startClientHandShake();
    void startServerHandShake();
    void ignoreSslErrors();

Q_SIGNALS:
    void encrypted();
    void sslErrors(const QList<QSslError> &errors);
    void modeChanged(QSslSocket::Mode newMode);

protected Q_SLOTS:
    void connectToHostImplementation(const QString &hostName, quint16 port,
                                     OpenMode openMode);
    void disconnectFromHostImplementation();

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    Q_DECLARE_PRIVATE(QSslSocket)
    Q_DISABLE_COPY(QSslSocket)
    Q_PRIVATE_SLOT(d_func(), void _q_connectedSlot())
    Q_PRIVATE_SLOT(d_func(), void _q_hostFoundSlot())
    Q_PRIVATE_SLOT(d_func(), void _q_disconnectedSlot())
    Q_PRIVATE_SLOT(d_func(), void _q_stateChangedSlot(QAbstractSocket::SocketState))
    Q_PRIVATE_SLOT(d_func(), void _q_errorSlot(QAbstractSocket::SocketError))
    Q_PRIVATE_SLOT(d_func(), void _q_readyReadSlot())
    Q_PRIVATE_SLOT(d_func(), void _q_bytesWrittenSlot(qint64))
    friend class QSslSocketBackendPrivate;
};

#endif // QT_NO_OPENSSL

QT_END_HEADER

#endif

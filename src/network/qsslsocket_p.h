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

#ifndef QSSLSOCKET_P_H
#define QSSLSOCKET_P_H

#include "qsslsocket.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <private/qtcpsocket_p.h>
#include "qsslkey.h"

#include <QtCore/qstringlist.h>

#include <private/qringbuffer_p.h>

class QSslSocketPrivate : public QTcpSocketPrivate
{
    Q_DECLARE_PUBLIC(QSslSocket)
public:
    QSslSocketPrivate();
    virtual ~QSslSocketPrivate();

    void init();

    QSslSocket::Mode mode;
    QSslSocket::Protocol protocol;
    bool autoStartHandShake;
    bool connectionEncrypted;
    bool ignoreSslErrors;

    QRingBuffer readBuffer;
    QRingBuffer writeBuffer;

    QSslCertificate peerCertificate;
    QList<QSslCertificate> peerCertificateChain;
    QSslCertificate localCertificate;

    static QList<QSslCipher> globalCiphers();
    static QList<QSslCipher> supportedCiphers();
    static void setGlobalCiphers(const QList<QSslCipher> &ciphers);
    static void setGlobalSupportedCiphers(const QList<QSslCipher> &ciphers);
    static void resetGlobalCiphers();

    static QList<QSslCertificate> globalCaCertificates();
    static QList<QSslCertificate> systemCaCertificates();
    static void setGlobalCaCertificates(const QList<QSslCertificate> &certs);
    static bool addGlobalCaCertificates(const QString &path);
    static void addGlobalCaCertificate(const QSslCertificate &cert);
    static void addGlobalCaCertificates(const QList<QSslCertificate> &certs);
    static QList<QSslCertificate> certificatesFromPath(const QString &path);

    QSslKey privateKey;

    QList<QSslCipher> ciphers;
    QList<QSslCertificate> localCaCertificates;

    // The socket itself, including private slots.
    QTcpSocket *plainSocket;
    void createPlainSocket(QIODevice::OpenMode openMode);
    void _q_connectedSlot();
    void _q_hostFoundSlot();
    void _q_disconnectedSlot();
    void _q_stateChangedSlot(QAbstractSocket::SocketState);
    void _q_errorSlot(QAbstractSocket::SocketError);
    void _q_readyReadSlot();
    void _q_bytesWrittenSlot(qint64);

    // Platform specific functions
    virtual void startClientHandShake() = 0;
    virtual void startServerHandShake() = 0;
    virtual void transmit() = 0;
    virtual void disconnectFromHost() = 0;
    virtual void disconnected() = 0;
    virtual QSslCipher currentCipher() const = 0;
};

#endif

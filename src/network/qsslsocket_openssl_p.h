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

#ifndef QSSLSOCKET_OPENSSL_P_H
#define QSSLSOCKET_OPENSSL_P_H

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

#include "qsslsocket_p.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/pkcs7.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/stack.h>
#include <openssl/x509.h>

class QSslSocketBackendPrivate : public QSslSocketPrivate
{
    Q_DECLARE_PUBLIC(QSslSocket)
public:
    QSslSocketBackendPrivate();
    virtual ~QSslSocketBackendPrivate();

    // Platform specific functions
    void startClientHandShake();
    void startServerHandShake();
    void transmit();
    bool testConnection();
    void disconnectFromHost();
    void disconnected();
    QSslCipher currentCipher() const;

    // SSL context
    static bool resolveSsl();
    bool initOpenSsl();
    bool initialized;
    SSL *ssl;
    SSL_CTX *ctx;
    BIO *readBio;
    BIO *writeBio;
    SSL_SESSION *session;

    static QSslCipher QSslCipher_from_SSL_CIPHER(SSL_CIPHER *cipher);
    static QByteArray X509_to_QByteArray(X509 *x509, bool pemEncoded);
    static X509 *QSslCertificate_to_X509(const QSslCertificate &certificate);
    static QSslCertificate X509_to_QSslCertificate(X509 *x509);
    static QList<QSslCertificate> STACKOFX509_to_QSslCertificates(STACK_OF(X509) *x509);
    static QSslCertificate QByteArray_to_QSslCertificate(const QByteArray &array);
    static QList<QSslCertificate> QByteArray_to_QSslCertificates(const QByteArray &array);
};

#endif

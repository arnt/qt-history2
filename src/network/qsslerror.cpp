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

#include "qsslerror.h"
#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
#endif

class QSslErrorPrivate
{
public:
    QSslError::SslError error;
    QSslCertificate certificate;
};

QSslError::QSslError(SslError error, const QSslCertificate &certificate)
    : d(new QSslErrorPrivate)
{
    d->error = error;
    d->certificate = certificate;
}

QSslError::QSslError(const QSslError &other)
    : d(new QSslErrorPrivate)
{
    *d = *other.d;
}

QSslError::~QSslError()
{
    delete d;
}

QSslError::SslError QSslError::error() const
{
    return d->error;
}

QString QSslError::errorString() const
{
    QString errStr;
    switch (d->error) {
    case NoError:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "No error"));
        break;
    case UnableToGetIssuerCertificate:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The issuer certificate could not be found"));
        break;
    case UnableToDecryptCertificateSignature:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The certificate signature could not be decrypted"));
        break;
    case UnableToDecodeIssuerPublicKey:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The public key in the certificate could not be read"));
        break;
    case CertificateSignatureFailed:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The signature of the certificate is invalid"));
        break;
    case CertificateNotYetValid:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The certificate is not yet valid"));
        break;
    case CertificateExpired:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The certificate has expired"));
        break;
    case InvalidNotBeforeField:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The certificate's notBefore field contains an invalid time"));
        break;
    case InvalidNotAfterField:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The certificate's notAfter field contains an invalid time"));
        break;
    case SelfSignedCertificate:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The certificate is self-signed, and untrusted"));
        break;
    case SelfSignedCertificateInChain:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The root certificate of the certificate chain is self-signed, and untrusted"));
        break;
    case UnableToGetLocalIssuerCertificate:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The issuer certificate of a locally looked up certificate could not be found"));
        break;
    case UnableToVerifyFirstCertificate:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "No certificates could be verified"));
        break;
    case InvalidCaCertificate:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "One of the CA certificates is invalid"));
        break;
    case PathLengthExceeded:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The basicConstraints pathlength parameter has been exceeded"));
        break;
    case InvalidPurpose:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The supplied certificate is unsuited for this purpose"));
        break;
    case CertificateUntrusted:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The root CA certificate is not trusted for this purpose"));
        break;
    case CertificateRejected:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The root CA certificate is marked to reject the specified purpose"));
        break;
    case SubjectIssuerMismatch: // hostname mismatch
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError,
                                               "The current candidate issuer certificate was rejected because its"
                                               " subject name did not match the issuer name of the current certificate"));
        break;
    case AuthorityIssuerSerialNumberMismatch:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The current candidate issuer certificate was rejected because"
                                               " its issuer name and serial number was present and did not match the"
                                               " authority key identifier of the current certificate"));
        break;
    case NoPeerCertificate:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "The peer did not present any certificate"));
        break;
    case HostNameMismatch:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError,
                                               "The host name did not match any of the valid hosts"
                                               " for this certificate"));
        break;
    case NoSslSupport:
        break;
    default:
        errStr = QObject::tr(QT_TRANSLATE_NOOP(QSslError, "Unknown error"));
        break;
    }

    return errStr;
}

QSslCertificate QSslError::certificate() const
{
    return d->certificate;
}

#ifndef QT_NO_DEBUG_STREAM
//class QDebug;
QDebug operator<<(QDebug debug, const QSslError &error)
{
    debug << error.errorString();
    return debug;
}
QDebug operator<<(QDebug debug, const QSslError::SslError &error)
{
    debug << QSslError(error).errorString();
    return debug;
}
#endif

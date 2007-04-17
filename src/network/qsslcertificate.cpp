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

/*!
    \class QSslCertificate
    \brief The QSslCertificate class provides a convenient API for an X509 certificate.
    \since 4.3

    \reentrant
    \ingroup io
    \module network

    QSslCertificate stores an X509 certificate, and is commonly used to verify
    the identity and store information about the local host, a remotely
    connected peer, or a trusted third party Certificate Authority.

    There are many ways to construct a QSslCertificate. The most common way is
    to call QSslSocket::peerCertificate() or
    QSslSocket::peerCertificateChain(), which both return QSslCertificate
    objects. You can also create load certificates from a binary or PEM   // ### create/load, DER
    encoded bundle, typically stored as one or more local files, or in a Qt
    Resource.

    You can call isNull() to check if your certificate is null or not; by
    default, QSslCertificate constructs a null certificate. To check if the
    certificate is valid, call isValid(). Null certificates are always also
    invalid. If you want to reset all contents in a certificate, call clear().

    After loading a certificate, you can find information about the
    certificate, its subject and its issuer by calling one of the many
    accessor functions, including version(), serialNumber(), issuerInfo() and
    subjectInfo(). You can call notValidBefore() and notValidAfter() to check
    when the certificate was issued, and when it expires. The publicKey()
    function returns the certificate subject's public key as a QSslKey. You
    can call issuerInfo() or subjectInfo() to get detailed information about
    the certificate issuer and its subject.

    Internally, QSslCertificate is stored as an X509 structure. You can access
    this handle by calling handle(), but the results are likely to not be
    portable.

    \sa QSslSocket, QSslKey, QSslCipher, QSslError
*/

/*!
    \enum QSslCertificate::SubjectInfo

    Describes keys that you can pass to QSslCertificate::issuerInfo() or
    QSslCertificate::subjectInfo() to get information about the certificate
    issuer or subject.

    \value Organization "O" The name of the organization.

    \value CommonName "CN" The common name; most often this is used to store
    the host name.

    \value LocalityName "L" The locality.

    \value OrganizationalUnitName "OU" The organizational unit name.

    \value CountryName "C" The country.

    \value StateOrProvinceName "ST" The state or province.
*/

#include "qsslsocket_openssl_symbols_p.h"
#include "qsslcertificate.h"
#include "qsslcertificate_p.h"
#include "qsslkey.h"
#include "qsslkey_p.h"

#include <QtCore/qatomic.h>
#include <QtCore/qdatetime.h>
#ifndef QT_NO_DEBUG
#include <QtCore/qdebug.h>
#endif
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

/*!
    Constructs a QSslCertificate, and reads from \a device to find the first
    available certificate. You can later call isNull() to see if \a device
    contained a certificate, and this certificate was loaded successfully.
*/
QSslCertificate::QSslCertificate(QIODevice *device, QSsl::EncodingFormat format)
    : d(new QSslCertificatePrivate)
{
    if (device)
        d->init(device->readAll(), format);
}

/*!
    Constructs a QSslCertificate, and parses \a data to find the first
    available certificate. You can later call isNull() to see if \a data
    contained a certificate, and this certificate was loaded successfully.
*/
QSslCertificate::QSslCertificate(const QByteArray &data, QSsl::EncodingFormat format)
    : d(new QSslCertificatePrivate)
{
    d->init(data, format);
}

/*!
    Constructs an identical copy of \a other.
*/
QSslCertificate::QSslCertificate(const QSslCertificate &other) : d(other.d)
{
    d->ref.ref();
}

/*!
    Destroys the QSslCertificate.
*/
QSslCertificate::~QSslCertificate()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    Copies the contents of \a other into this certificate, making the two
    certificates identical.
*/
QSslCertificate &QSslCertificate::operator=(const QSslCertificate &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Returns true if this certificate is the same as \a other; otherwise
    returns false.
*/
bool QSslCertificate::operator==(const QSslCertificate &other) const
{
    if (d == other.d)
        return true;
    if (d->null && other.d->null)
        return true;
    if (d->x509 && other.d->x509)
        return q_X509_cmp(d->x509, other.d->x509) == 0;
    return false;
}

/*!
    \fn bool QSslCertificate::operator!=(const QSslCertificate &other) const

    Returns true if this certificate is not the same as \a other; otherwise
    returns false.
*/

/*!
    Returns true if this is a null certificate (i.e., a certificate with no
    contents); otherwise returns false.

    By default, QSslCertificate constructs a null certificate.

    \sa isValid(), clear()
*/
bool QSslCertificate::isNull() const
{
    return d->null;
}

/*!
    Returns true if this certificate is valid; otherwise returns false.

    Note: Currently, this function always returns false.

    \sa isNull()
*/
bool QSslCertificate::isValid() const
{
    const QDateTime currentTime = QDateTime::currentDateTime();
    return currentTime >= d->notValidBefore && currentTime <= d->notValidAfter;
}

/*!
    Clears the contents of this certificate, making it a null certificate.

    \sa isNull()
*/
void QSslCertificate::clear()
{
    if (!d->ref.deref()) {
        delete d;
        d = new QSslCertificatePrivate;
    }
}

/*!
    Returns the certificate's version string.
*/
QByteArray QSslCertificate::version() const
{
    return d->versionString;
}

/*!
    Returns the certificate's serial number string.
*/
QByteArray QSslCertificate::serialNumber() const
{
    return d->serialNumberString;
}

/*!
    Returns a cryptographic digest of this certificate. By default, and MD5
    digest will be generated, but you can also specify a custom \a algorithm.
*/
QByteArray QSslCertificate::digest(QCryptographicHash::Algorithm algorithm) const
{
    return QCryptographicHash::hash(toDer(), algorithm);
}

static QString _q_SubjectInfoToString(QSslCertificate::SubjectInfo info)
{
    QString str;
    switch (info) {
    case QSslCertificate::Organization: str = QLatin1String("O"); break;
    case QSslCertificate::CommonName: str = QLatin1String("CN"); break;
    case QSslCertificate::LocalityName: str = QLatin1String("L"); break;
    case QSslCertificate::OrganizationalUnitName: str = QLatin1String("OU"); break;
    case QSslCertificate::CountryName: str = QLatin1String("C"); break;
    case QSslCertificate::StateOrProvinceName: str = QLatin1String("ST"); break;
    }
    return str;
}

/*!
    Returns the issuer info for \a info, or an empty string if there is no
    information for \a info in the certificate.

    \sa subjectInfo()
*/
QString QSslCertificate::issuerInfo(SubjectInfo info) const
{
    return d->issuerInfo.value(_q_SubjectInfoToString(info));
}

/*!
    Returns the issuer info for \a tag, or an empty string if there is no
    information for \a tag in the certificate.

    \sa subjectInfo()
*/
QString QSslCertificate::issuerInfo(const QByteArray &tag) const
{
    // ### Use a QByteArray for the keys in the map
    return d->issuerInfo.value(QString::fromLatin1(tag));
}

/*!
    Returns the subject info for \a info, or an empty string if there is no
    information for \a info in the certificate.

    \sa issuerInfo()
*/
QString QSslCertificate::subjectInfo(SubjectInfo info) const
{
    return d->subjectInfo.value(_q_SubjectInfoToString(info));
}

/*!
    Returns the subject info for \a tag, or an empty string if there is no
    information for \a tag in the certificate.

    \sa issuerInfo()
*/
QString QSslCertificate::subjectInfo(const QByteArray &tag) const
{
    // ### Use a QByteArray for the keys in the map
    return d->subjectInfo.value(QString::fromLatin1(tag));
}

/*!
    Returns the list of alternative subject names for this certificate. The
    alternate subject names typically contain hostnames, optionally with
    wildcards, that are valid for this certificate.

    These names are tested against the connected peer's host name if the
    subject info for \l CommonName either does not define a valid host name,
    or if the subject info name doesn't match the peer's host name.

    \sa subjectInfo()
*/
QStringList QSslCertificate::alternateSubjectNames() const
{
    // ### unimplemented
    return QStringList();
}

/*!
    Returns the date that the certificate becomes valid, or an empty QDateTime
    if this is a null certificate.

    \sa notValidAfter()
*/
QDateTime QSslCertificate::notValidBefore() const
{
    return d->notValidBefore;
}

/*!
    Returns the date that the certificate expires, or an empty QDateTime if
    this is a null certificate.

    \sa notValidBefore()
*/
QDateTime QSslCertificate::notValidAfter() const
{
    return d->notValidAfter;
}

/*!
    Returns a pointer to the native certificate handle, if this is available;
    otherwise a null pointer is returned.

    You can use this handle together with native API to access extended
    information about the certificate.

    \warning Use of this function has a high probability of being
    non-portable, and its return value may vary between platforms, and between
    minor Qt releases.
*/
Qt::HANDLE QSslCertificate::handle() const
{
    return Qt::HANDLE(d->x509);
}

/*!
    Returns the certificate subject's public key.
*/
QSslKey QSslCertificate::publicKey() const
{
    if (!d->x509)
        return QSslKey();

    QSslKey key;

    key.d->type = QSsl::PublicKey;
    X509_PUBKEY *xkey = d->x509->cert_info->key;
    EVP_PKEY *pkey = q_X509_PUBKEY_get(xkey);
    Q_ASSERT(pkey);

    if (q_EVP_PKEY_type(pkey->type) == EVP_PKEY_RSA) {
        key.d->rsa = q_EVP_PKEY_get1_RSA(pkey);
        key.d->algorithm = QSsl::Rsa;
        key.d->isNull = false;
    } else if (q_EVP_PKEY_type(pkey->type) == EVP_PKEY_DSA) {
        key.d->dsa = q_EVP_PKEY_get1_DSA(pkey);
        key.d->algorithm = QSsl::Dsa;
        key.d->isNull = false;
    } else if (q_EVP_PKEY_type(pkey->type) == EVP_PKEY_DH) {
        // DH unsupported
    } else {
        // error?
    }

    q_EVP_PKEY_free(pkey);
    return key;
}

/*!
    Returns this certificate converted to a PEM (Base64) encoded
    representation.
*/
QByteArray QSslCertificate::toPem() const
{
    if (!d->x509)
        return QByteArray();
    return d->QByteArray_from_X509(d->x509, QSsl::Pem);
}

/*!
    Returns this certificate converted to a DER (binary) encoded
    representation.
*/
QByteArray QSslCertificate::toDer() const
{
    if (!d->x509)
        return QByteArray();
    return d->QByteArray_from_X509(d->x509, QSsl::Der);
}

/*!
    Searches for and parses all certificates in all files in \a path, and
    returns the list of certificates. \a path can be a file or a path, and it
    can also contain wildcards.

    \sa fromData()
*/
QList<QSslCertificate> QSslCertificate::fromPath(const QString &path, QSsl::EncodingFormat format)
{
    QStringList entryList;
    if (QFileInfo(path).isDir()) {
        entryList = QDir(path).entryList();
    } else {
        entryList << path;
    }

    QList<QSslCertificate> certs;
    foreach (QString path, entryList) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            certs << QSslCertificate(file.readAll(), format);
    }
    return certs;
}

/*!
    Searches for and parses all certificates in \a device, and returns the
    list of certificates.

    \sa fromData()
*/
QList<QSslCertificate> QSslCertificate::fromDevice(QIODevice *device, QSsl::EncodingFormat format)
{
    return fromData(device->readAll(), format);
}

/*!
    Searches for and parses all certificates in \a data, and returns the
    list of certificates.

    \sa fromDevice()
*/
QList<QSslCertificate> QSslCertificate::fromData(
    const QByteArray &data, QSsl::EncodingFormat format)
{
    return (format == QSsl::Pem)
        ? QSslCertificatePrivate::certificatesFromPem(data)
        : QSslCertificatePrivate::certificatesFromDer(data);
}

void QSslCertificatePrivate::init(const QByteArray &data, QSsl::EncodingFormat format)
{
    if (!data.isEmpty()) {
        QList<QSslCertificate> certs = (format == QSsl::Pem)
            ? certificatesFromPem(data, 1)
            : certificatesFromDer(data, 1);
        if (!certs.isEmpty()) {
            *this = *certs.first().d;
            if (x509)
                x509 = q_X509_dup(x509);
        }
    }
}

static const char BeginCertString[] = "-----BEGIN CERTIFICATE-----\n";
static const char EndCertString[] = "-----END CERTIFICATE-----\n";

// ### refactor against QSsl::pemFromDer() etc. (to avoid redundant implementations)
QByteArray QSslCertificatePrivate::QByteArray_from_X509(X509 *x509, QSsl::EncodingFormat format)
{
    if (!x509) {
        qWarning("QSslSocketBackendPrivate::X509_to_QByteArray: null X509");
        return QByteArray();
    }

    // Use i2d_X509 to convert the X509 to an array.
    int length = q_i2d_X509(x509, 0);
    QByteArray array;
    array.resize(length);
    char *data = array.data();
    char **dataP = &data;
    unsigned char **dataPu = (unsigned char **)dataP;
    if (q_i2d_X509(x509, dataPu) < 0)
        return QByteArray();

    if (format == QSsl::Der)
        return array;

    // Convert to Base64 - wrap at 64 characters.
    array = array.toBase64();
    QByteArray tmp;
    for (int i = 0; i < array.size() - 64; i += 64) {
        tmp += QByteArray::fromRawData(array.data() + i, 64);
        tmp += "\n";
    }
    if (int remainder = array.size() % 64) {
        tmp += QByteArray::fromRawData(array.data() + array.size() - remainder, remainder);
        tmp += "\n";
    }

    return BeginCertString + tmp + EndCertString;
}

static QMap<QString, QString> _q_mapFromOnelineName(char *name)
{
    QMap<QString, QString> info;
    QString infoStr = QString::fromLocal8Bit(name);
    q_CRYPTO_free(name);

    // ### The right-hand encoding seems to allow hex (Regulierungsbeh\xC8orde)
    //entry.replace(QLatin1String("\\x"), QLatin1String("%"));
    //entry = QUrl::fromPercentEncoding(entry.toLatin1());
    // ### See RFC-4630 for more details!

    QRegExp rx(QLatin1String("/([A-Za-z]+)=(.+)"));

    int pos = 0;
    while ((pos = rx.indexIn(infoStr, pos)) != -1) {
        const QString name = rx.cap(1);

        QString value = rx.cap(2);
        const int valuePos = rx.pos(2);

        const int next = rx.indexIn(value);
        if (next == -1) {
            info.insert(name, value);
            break;
        }

        value = value.left(next);
        info.insert(name, value);
        pos = valuePos + value.length();
    }

    return info;
}

QSslCertificate QSslCertificatePrivate::QSslCertificate_from_X509(X509 *x509)
{
    QSslCertificate certificate;
    if (!x509 || !QSslSocket::supportsSsl())
        return certificate;

    certificate.d->issuerInfo =
        _q_mapFromOnelineName(q_X509_NAME_oneline(q_X509_get_issuer_name(x509), 0, 0));
    certificate.d->subjectInfo =
        _q_mapFromOnelineName(q_X509_NAME_oneline(q_X509_get_subject_name(x509), 0, 0));

    ASN1_TIME *nbef = X509_get_notBefore(x509); // ### convert to q_X509_get_notBefore
    ASN1_TIME *naft = X509_get_notAfter(x509);  // ### convert to q_X509_get_notAfter
    certificate.d->notValidBefore.setTime_t(q_getTimeFromASN1(nbef));
    certificate.d->notValidAfter.setTime_t(q_getTimeFromASN1(naft));
    certificate.d->null = false;
    certificate.d->x509 = q_X509_dup(x509);

    return certificate;
}

QList<QSslCertificate> QSslCertificatePrivate::certificatesFromPem(
    const QByteArray &pem, int count)
{
    QList<QSslCertificate> certificates;

    int offset = 0;
    while (count == -1 || certificates.size() < count) {
        int startPos = pem.indexOf(BeginCertString, offset);
        if (startPos == -1)
            break;
        startPos += sizeof(BeginCertString) - 1;
        
        int endPos = pem.indexOf(EndCertString, startPos);
        if (endPos == -1)
            break;

        offset = endPos + sizeof(EndCertString) - 1;

        QByteArray decoded = QByteArray::fromBase64(
            QByteArray::fromRawData(pem.data() + startPos, endPos - startPos));
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
        const unsigned char *data = (const unsigned char *)decoded.data();
#else
        unsigned char *data = (unsigned char *)decoded.data();
#endif
        
        if (X509 *x509 = q_d2i_X509(0, &data, decoded.size())) {
            certificates << QSslCertificate_from_X509(x509);
            q_X509_free(x509);
        }
    }

    return certificates;
}

QList<QSslCertificate> QSslCertificatePrivate::certificatesFromDer(
    const QByteArray &der, int count)
{
    QList<QSslCertificate> certificates;

#if OPENSSL_VERSION_NUMBER >= 0x00908000L
        const unsigned char *data = (const unsigned char *)der.data();
#else
        unsigned char *data = (unsigned char *)der.data();
#endif
    int size = der.size();

    while (count == -1 || certificates.size() < count) {
        if (X509 *x509 = q_d2i_X509(0, &data, size)) {
            certificates << QSslCertificate_from_X509(x509);
            q_X509_free(x509);
        } else {
            break;
        }
        size -= ((char *)data - der.data());
    }

    return certificates;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug debug, const QSslCertificate &certificate)
{
    debug << "QSslCertificate("
          << certificate.version()
          << "," << certificate.serialNumber()
          << "," << certificate.digest()
          << "," << certificate.issuerInfo(QSslCertificate::Organization)
          << "," << certificate.subjectInfo(QSslCertificate::Organization)
          << "," << certificate.alternateSubjectNames()
          << "," << certificate.notValidBefore()
          << "," << certificate.notValidAfter()
          << ")";
    return debug;
}
QDebug operator<<(QDebug debug, QSslCertificate::SubjectInfo info)
{
    switch (info) {
    case QSslCertificate::Organization: debug << "Organization"; break;
    case QSslCertificate::CommonName: debug << "CommonName"; break;
    case QSslCertificate::CountryName: debug << "CountryName"; break;
    case QSslCertificate::LocalityName: debug << "LocalityName"; break;
    case QSslCertificate::OrganizationalUnitName: debug << "OrganizationalUnitName"; break;
    case QSslCertificate::StateOrProvinceName: debug << "StateOrProvinceName"; break;
    }
    return debug;
}
#endif

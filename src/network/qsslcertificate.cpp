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

#include "qsslsocket_openssl_symbols_p.h"
#include "qsslcertificate.h"
#include "qsslcertificate_p.h"
#include "qsslkey.h"
#include "qsslkey_p.h"

#include <QtCore/qatomic.h>
#include <QtCore/qdatetime.h>
#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
#endif
#include <QtCore/qdir.h>
#include <QtCore/qdiriterator.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <QtCore/qdebug.h>

QSslCertificate::QSslCertificate(QIODevice *device, QSsl::EncodingFormat format)
    : d(new QSslCertificatePrivate)
{
    QSslSocketPrivate::ensureInitialized();
    if (device)
        d->init(device->readAll(), format);
}

QSslCertificate::QSslCertificate(const QByteArray &data, QSsl::EncodingFormat format)
    : d(new QSslCertificatePrivate)
{
    QSslSocketPrivate::ensureInitialized();
    d->init(data, format);
}

QSslCertificate::QSslCertificate(const QSslCertificate &other) : d(other.d)
{
    d->ref.ref();
}

QSslCertificate::~QSslCertificate()
{
    if (!d->ref.deref())
        delete d;
}

QSslCertificate &QSslCertificate::operator=(const QSslCertificate &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

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

bool QSslCertificate::isNull() const
{
    return d->null;
}

bool QSslCertificate::isValid() const
{
    const QDateTime currentTime = QDateTime::currentDateTime();
    return currentTime >= d->notValidBefore && currentTime <= d->notValidAfter;
}

void QSslCertificate::clear()
{
    if (!d->ref.deref()) {
        delete d;
        d = new QSslCertificatePrivate;
    }
}

QByteArray QSslCertificate::version() const
{
    return d->versionString;
}

QByteArray QSslCertificate::serialNumber() const
{
    return d->serialNumberString;
}

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

QString QSslCertificate::issuerInfo(SubjectInfo info) const
{
    return d->issuerInfo.value(_q_SubjectInfoToString(info));
}

QString QSslCertificate::issuerInfo(const QByteArray &tag) const
{
    // ### Use a QByteArray for the keys in the map
    return d->issuerInfo.value(QString::fromLatin1(tag));
}

QString QSslCertificate::subjectInfo(SubjectInfo info) const
{
    return d->subjectInfo.value(_q_SubjectInfoToString(info));
}

QString QSslCertificate::subjectInfo(const QByteArray &tag) const
{
    // ### Use a QByteArray for the keys in the map
    return d->subjectInfo.value(QString::fromLatin1(tag));
}

QMultiMap<QSsl::AlternateNameEntryType, QString>
QSslCertificate::alternateSubjectNames() const
{
    QMultiMap<QSsl::AlternateNameEntryType, QString> result;

    STACK *altNames = (STACK *)q_X509_get_ext_d2i(d->x509, NID_subject_alt_name, 0, 0);

    if (altNames) {
        for (int i = 0; i < q_sk_GENERAL_NAME_num(altNames); ++i) {
            const GENERAL_NAME *genName = q_sk_GENERAL_NAME_value(altNames, i);
            const QString altName = QLatin1String(
                    QByteArray(reinterpret_cast<const char *>(q_ASN1_STRING_data(genName->d.ia5)),
                    q_ASN1_STRING_length(genName->d.ia5)));
            if (genName->type == GEN_DNS)
                result.insert(QSsl::DnsEntry, altName);
            else if (genName->type == GEN_EMAIL)
                result.insert(QSsl::EmailEntry, altName);
        }
        q_sk_free(altNames);
    }

    return result;
}

QDateTime QSslCertificate::effectiveDate() const
{
    return d->notValidBefore;
}

QDateTime QSslCertificate::expiryDate() const
{
    return d->notValidAfter;
}

Qt::HANDLE QSslCertificate::handle() const
{
    return Qt::HANDLE(d->x509);
}

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

QByteArray QSslCertificate::toPem() const
{
    if (!d->x509)
        return QByteArray();
    return d->QByteArray_from_X509(d->x509, QSsl::Pem);
}

QByteArray QSslCertificate::toDer() const
{
    if (!d->x509)
        return QByteArray();
    return d->QByteArray_from_X509(d->x509, QSsl::Der);
}

QList<QSslCertificate>
QSslCertificate::fromPath(const QString &path,
			  QSsl::EncodingFormat format,
			  QRegExp::PatternSyntax syntax)
{
    if (syntax == QRegExp::FixedString) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            return QSslCertificate::fromData(file.readAll(),format);
        return QList<QSslCertificate>();
    }

    QList<QSslCertificate> certs;
    QRegExp pattern(path, Qt::CaseSensitive, syntax);

    QDirIterator it(path);
    while (it.hasNext()) {
        if (!pattern.exactMatch(path))
            continue;

        QFile file(it.filePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            certs += QSslCertificate::fromData(file.readAll(),format);
    }
    return certs;
}

QList<QSslCertificate>
QSslCertificate::fromDevice(QIODevice *device, QSsl::EncodingFormat format)
{
    if (!device) {
        qWarning("QSslCertificate::fromDevice: cannot read from a null device");
        return QList<QSslCertificate>();
    }
    return fromData(device->readAll(), format);
}

QList<QSslCertificate>
QSslCertificate::fromData(const QByteArray &data, QSsl::EncodingFormat format)
{
    return (format == QSsl::Pem)
        ? QSslCertificatePrivate::certificatesFromPem(data)
        : QSslCertificatePrivate::certificatesFromDer(data);
}

void QSslCertificatePrivate::init(const QByteArray &data,
				  QSsl::EncodingFormat format)
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

    ASN1_TIME *nbef = q_X509_get_notBefore(x509);
    ASN1_TIME *naft = q_X509_get_notAfter(x509);
    certificate.d->notValidBefore.setTime_t(q_getTimeFromASN1(nbef));
    certificate.d->notValidAfter.setTime_t(q_getTimeFromASN1(naft));
    certificate.d->null = false;
    certificate.d->x509 = q_X509_dup(x509);

    return certificate;
}

QList<QSslCertificate>
QSslCertificatePrivate::certificatesFromPem(const QByteArray &pem, int count)
{
    QList<QSslCertificate> certificates;
    QSslSocketPrivate::ensureInitialized();

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

QList<QSslCertificate>
QSslCertificatePrivate::certificatesFromDer(const QByteArray &der, int count)
{
    QList<QSslCertificate> certificates;
    QSslSocketPrivate::ensureInitialized();


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

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const QSslCertificate &certificate)
{
    debug << "QSslCertificate("
          << certificate.version()
          << "," << certificate.serialNumber()
          << "," << certificate.digest()
          << "," << certificate.issuerInfo(QSslCertificate::Organization)
          << "," << certificate.subjectInfo(QSslCertificate::Organization)
          << "," << certificate.alternateSubjectNames()
#ifndef QT_NO_TEXTSTREAM
          << "," << certificate.effectiveDate()
          << "," << certificate.expiryDate()
#endif
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

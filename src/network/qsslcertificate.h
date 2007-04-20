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

#ifndef QSSLCERTIFICATE_H
#define QSSLCERTIFICATE_H

#include <QtCore/qnamespace.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qcryptographichash.h>
#include <QtCore/qregexp.h>
#include <QtNetwork/qssl.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

#ifndef QT_NO_OPENSSL

class QDateTime;
class QIODevice;
class QSslKey;
class QStringList;
template <typename T, typename U> class QMap;

typedef struct x509_st X509; // ### check if this works

class QSslCertificatePrivate;
class Q_NETWORK_EXPORT QSslCertificate
{
public:
    enum SubjectInfo {
        Organization,
        CommonName,
        LocalityName,
        OrganizationalUnitName,
        CountryName,
        StateOrProvinceName
    };

    QSslCertificate(QIODevice *device, QSsl::EncodingFormat format = QSsl::Pem);
    QSslCertificate( // ### s/encoded/data (to be consistent with signature in .cpp file) ?
        const QByteArray &encoded = QByteArray(), QSsl::EncodingFormat format = QSsl::Pem);
    QSslCertificate(const QSslCertificate &other);
    ~QSslCertificate();
    QSslCertificate &operator=(const QSslCertificate &other);
    bool operator==(const QSslCertificate &other) const;
    inline bool operator!=(const QSslCertificate &other) const { return !operator==(other); }

    bool isNull() const;
    bool isValid() const;
    void clear();

    // Certificate info
    QByteArray version() const;
    QByteArray serialNumber() const;
    QByteArray digest(QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5) const;
    QString issuerInfo(SubjectInfo info) const;
    QString issuerInfo(const QByteArray &tag) const;
    QString subjectInfo(SubjectInfo info) const;
    QString subjectInfo(const QByteArray &tag) const;
    QMultiMap<QSsl::AlternateNameEntry, QString> alternateSubjectNames() const;
    QDateTime notValidBefore() const;
    QDateTime notValidAfter() const;
    QSslKey publicKey() const;

    QByteArray toPem() const;
    QByteArray toDer() const;

    static QList<QSslCertificate> fromPath(
        const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
        QRegExp::PatternSyntax syntax = QRegExp::FixedString);
    static QList<QSslCertificate> fromDevice(
        QIODevice *device, QSsl::EncodingFormat format = QSsl::Pem);
    static QList<QSslCertificate> fromData(
        const QByteArray &data, QSsl::EncodingFormat format = QSsl::Pem);

    Qt::HANDLE handle() const;

private:
    QSslCertificatePrivate *d;
    friend class QSslCertificatePrivate;
    friend class QSslSocketBackendPrivate;
};

#ifndef QT_NO_DEBUG
class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslCertificate &certificate);
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QSslCertificate::SubjectInfo info);
#endif

#endif // QT_NO_OPENSSL

QT_END_HEADER

#endif

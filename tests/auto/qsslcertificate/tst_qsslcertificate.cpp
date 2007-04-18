/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsslcertificate.h>
#include <qsslkey.h>
#include <qsslsocket.h>

#include <QtNetwork/qhostaddress.h> // ### needed?
#include <QtNetwork/qnetworkproxy.h> // ### needed?

class tst_QSslCertificate : public QObject
{
    Q_OBJECT

    struct CertInfo {
        QFileInfo fileInfo;
        QFileInfo fileInfo_digest_md5;
        QFileInfo fileInfo_digest_sha1;
        QSsl::EncodingFormat format;
        CertInfo(const QFileInfo &fileInfo, QSsl::EncodingFormat format)
            : fileInfo(fileInfo), format(format) {}
    };

    QList<CertInfo> certInfoList;
    QMap<QString, QString> pubkeyMap;
    QMap<QString, QString> md5Map;
    QMap<QString, QString> sha1Map;

    void createTestRows();
#ifndef QT_NO_OPENSSL
    void compareCertificates(const QSslCertificate & cert1, const QSslCertificate & cert2);
#endif

public:
    tst_QSslCertificate();
    virtual ~tst_QSslCertificate();

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL
private slots:
    void emptyConstructor();
    void constructor_data();
    void constructor();
    void constructingGarbage();
    void copyAndAssign_data();
    void copyAndAssign();
    void digest_data();
    void digest();
    void publicKey_data();
    void publicKey();
    void toPemOrDer_data();
    void toPemOrDer();

// ### add tests for certificate bundles (multiple certificates concatenated into a single
//     structure); both PEM and DER formatted
#endif
};

tst_QSslCertificate::tst_QSslCertificate()
{
    QDir dir(qApp->applicationDirPath() + QLatin1String("/certificates"));
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Readable);

    // register all certificates
    QRegExp rx(QLatin1String("^.+\\.(pem|der)$"));
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (rx.indexIn(fileInfo.fileName()) >= 0)
            certInfoList
                << CertInfo(fileInfo, rx.cap(1) == QLatin1String("pem") ? QSsl::Pem : QSsl::Der);
    }

    // register all public keys
    rx = QRegExp(QLatin1String("^(.+\\.(?:pem|der))\\.pubkey$"));
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (rx.indexIn(fileInfo.fileName()) >= 0) {
            QString key = rx.cap(1);
            pubkeyMap.insert(key, fileInfo.absoluteFilePath());
        }
    }

    // register all digests (of entire certificates)
    rx = QRegExp(QLatin1String("^(.+\\.(?:pem|der))\\.digest-(md5|sha1)$"));
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (rx.indexIn(fileInfo.fileName()) >= 0) {
            QString key = rx.cap(1);
            if (rx.cap(2) == QLatin1String("md5"))
                md5Map.insert(key, fileInfo.absoluteFilePath());
            else
                sha1Map.insert(key, fileInfo.absoluteFilePath());
        }
    }
}

tst_QSslCertificate::~tst_QSslCertificate()
{
}

void tst_QSslCertificate::initTestCase_data()
{
}

void tst_QSslCertificate::init()
{
}

void tst_QSslCertificate::cleanup()
{
}

static QByteArray readFile(const QString &absFilePath)
{
    QFile file(absFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QWARN("failed to open file");
        return QByteArray();
    }
    return file.readAll();
}

#ifndef QT_NO_OPENSSL

void tst_QSslCertificate::emptyConstructor()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslCertificate certificate;
    QVERIFY(certificate.isNull());
}

Q_DECLARE_METATYPE(QSsl::EncodingFormat);

void tst_QSslCertificate::createTestRows()
{
    QTest::addColumn<QString>("absFilePath");
    QTest::addColumn<QSsl::EncodingFormat>("format");
    foreach (CertInfo certInfo, certInfoList) {
        QTest::newRow(certInfo.fileInfo.fileName().toLatin1())
            << certInfo.fileInfo.absoluteFilePath() << certInfo.format;
    }
}

void tst_QSslCertificate::constructor_data()
{
    createTestRows();
}

void tst_QSslCertificate::constructor()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::EncodingFormat, format);

    QByteArray encoded = readFile(absFilePath);
    QSslCertificate certificate(encoded, format);
    QVERIFY(!certificate.isNull());
}

void tst_QSslCertificate::constructingGarbage()
{
    if (!QSslSocket::supportsSsl())
        return;

    QByteArray garbage("garbage");
    QSslCertificate certificate(garbage);
    QVERIFY(certificate.isNull());
}

void tst_QSslCertificate::copyAndAssign_data()
{
    createTestRows();
}

void tst_QSslCertificate::compareCertificates(
    const QSslCertificate & cert1, const QSslCertificate & cert2)
{
    QCOMPARE(cert1.isNull(), cert2.isNull());
    // Note: in theory, the next line could fail even if the certificates are identical!
    QCOMPARE(cert1.isValid(), cert2.isValid());
    QCOMPARE(cert1.version(), cert2.version());
    QCOMPARE(cert1.serialNumber(), cert2.serialNumber());
    QCOMPARE(cert1.digest(), cert2.digest());
    QCOMPARE(cert1.toPem(), cert2.toPem());
    QCOMPARE(cert1.toDer(), cert2.toDer());
    for (int info = QSslCertificate::Organization;
         info <= QSslCertificate::StateOrProvinceName; info++) {
        const QSslCertificate::SubjectInfo subjectInfo = (QSslCertificate::SubjectInfo)info;
        QCOMPARE(cert1.issuerInfo(subjectInfo), cert2.issuerInfo(subjectInfo));
        QCOMPARE(cert1.subjectInfo(subjectInfo), cert2.subjectInfo(subjectInfo));
    }
    QCOMPARE(cert1.alternateSubjectNames(), cert2.alternateSubjectNames());
    QCOMPARE(cert1.notValidBefore(), cert2.notValidBefore());
    QCOMPARE(cert1.notValidAfter(), cert2.notValidAfter());
    // ### add more functions here ...
}

void tst_QSslCertificate::copyAndAssign()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::EncodingFormat, format);

    QByteArray encoded = readFile(absFilePath);
    QSslCertificate certificate(encoded, format);

    QSslCertificate copied(certificate);
    compareCertificates(certificate, copied);

    QSslCertificate assigned = certificate;
    compareCertificates(certificate, assigned);
}

void tst_QSslCertificate::digest_data()
{
    QTest::addColumn<QString>("absFilePath");
    QTest::addColumn<QSsl::EncodingFormat>("format");
    QTest::addColumn<QString>("absFilePath_digest_md5");
    QTest::addColumn<QString>("absFilePath_digest_sha1");
    foreach (CertInfo certInfo, certInfoList) {
        QString certName = certInfo.fileInfo.fileName();
        QTest::newRow(certName.toLatin1())
            << certInfo.fileInfo.absoluteFilePath()
            << certInfo.format
            << md5Map.value(certName)
            << sha1Map.value(certName);
    }
}

// Converts a digest of the form '{MD5|SHA1} Fingerprint=AB:B8:32...' to binary format.
static QByteArray convertDigest(const QByteArray &input)
{
    QByteArray result;
    QRegExp rx(QLatin1String("(?:=|:)([0-9A-Fa-f]{2})"));
    int pos = 0;
    while ((pos = rx.indexIn(input, pos)) != -1) {
        result.append(rx.cap(1).toLatin1());
        pos += rx.matchedLength();
    }
    return QByteArray::fromHex(result);
}

void tst_QSslCertificate::digest()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::EncodingFormat, format);
    QFETCH(QString, absFilePath_digest_md5);
    QFETCH(QString, absFilePath_digest_sha1);

    QByteArray encoded = readFile(absFilePath);
    QSslCertificate certificate(encoded, format);
    QVERIFY(!certificate.isNull());

    if (!absFilePath_digest_md5.isEmpty())
        QCOMPARE(convertDigest(readFile(absFilePath_digest_md5)),
                 certificate.digest(QCryptographicHash::Md5));

    if (!absFilePath_digest_sha1.isEmpty())
        QCOMPARE(convertDigest(readFile(absFilePath_digest_sha1)),
                 certificate.digest(QCryptographicHash::Sha1));
}

void tst_QSslCertificate::publicKey_data()
{
    QTest::addColumn<QString>("certFilePath");
    QTest::addColumn<QSsl::EncodingFormat>("format");
    QTest::addColumn<QString>("pubkeyFilePath");

    foreach (CertInfo certInfo, certInfoList) {
        QString certName = certInfo.fileInfo.fileName();
        if (pubkeyMap.contains(certName))
            QTest::newRow(certName.toLatin1())
                << certInfo.fileInfo.absoluteFilePath()
                << certInfo.format
                << pubkeyMap.value(certName);
    }
}

void tst_QSslCertificate::publicKey()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, certFilePath);
    QFETCH(QSsl::EncodingFormat, format);
    QFETCH(QString, pubkeyFilePath);

    QByteArray encodedCert = readFile(certFilePath);
    QSslCertificate certificate(encodedCert, format);
    QVERIFY(!certificate.isNull());

    QByteArray encodedPubkey = readFile(pubkeyFilePath);
    QSslKey pubkey(encodedPubkey, QSsl::Rsa, format, QSsl::PublicKey); // ### support DSA as well!
    QVERIFY(!pubkey.isNull());

    QCOMPARE(certificate.publicKey(), pubkey);
}

void tst_QSslCertificate::toPemOrDer_data()
{
    createTestRows();
}

static const char BeginCertString[] = "-----BEGIN CERTIFICATE-----\n";
static const char EndCertString[] = "-----END CERTIFICATE-----\n";

// Returns, in Pem-format, the first certificate found in a Pem-formatted block
// (Note that such a block may contain e.g. a private key at the end).
static QByteArray firstPemCertificateFromPem(const QByteArray &pem)
{
    int startPos = pem.indexOf(BeginCertString);
    int endPos = pem.indexOf(EndCertString);
    if (startPos == -1 || endPos == -1)
        return QByteArray();
    return pem.mid(startPos, endPos + sizeof(EndCertString) - startPos - 1);
}

void tst_QSslCertificate::toPemOrDer()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::EncodingFormat, format);

    QByteArray encoded = readFile(absFilePath);
    QSslCertificate certificate(encoded, format);
    QVERIFY(!certificate.isNull());
    if (format == QSsl::Pem)
        QCOMPARE(certificate.toPem(), firstPemCertificateFromPem(encoded));
    else
        // ### for now, we assume that DER-encoded certificates don't contain bundled stuff
        QCOMPARE(certificate.toDer(), encoded);
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslCertificate)
#include "tst_qsslcertificate.moc"

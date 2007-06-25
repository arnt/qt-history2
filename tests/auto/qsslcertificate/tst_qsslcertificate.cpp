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
    QMap<QString, QString> subjAltNameMap;
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
    void alternateSubjectNames_data();
    void alternateSubjectNames();
    void publicKey_data();
    void publicKey();
    void toPemOrDer_data();
    void toPemOrDer();
    void fromDevice();

// ### add tests for certificate bundles (multiple certificates concatenated into a single
//     structure); both PEM and DER formatted
#endif
};

tst_QSslCertificate::tst_QSslCertificate()
{
#ifdef Q_WS_MAC
    // applicationDirPath() points to a path inside the app bundle on Mac.
    QDir dir(qApp->applicationDirPath() + QLatin1String("/../../../certificates"));
#elif defined(Q_OS_WIN32)
    QDir dir(QDir::currentPath() + QLatin1String("/certificates"));
#else
    QDir dir(qApp->applicationDirPath() + QLatin1String("/certificates"));
#endif
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Readable);
    QRegExp rxCert(QLatin1String("^.+\\.(pem|der)$"));
    QRegExp rxSan(QLatin1String("^(.+\\.(?:pem|der))\\.san$"));
    QRegExp rxPubKey(QLatin1String("^(.+\\.(?:pem|der))\\.pubkey$"));
    QRegExp rxDigest(QLatin1String("^(.+\\.(?:pem|der))\\.digest-(md5|sha1)$"));
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (rxCert.indexIn(fileInfo.fileName()) >= 0)
            certInfoList <<
                CertInfo(fileInfo,
                         rxCert.cap(1) == QLatin1String("pem") ? QSsl::Pem : QSsl::Der);
        if (rxSan.indexIn(fileInfo.fileName()) >= 0)
            subjAltNameMap.insert(rxSan.cap(1), fileInfo.absoluteFilePath());
        if (rxPubKey.indexIn(fileInfo.fileName()) >= 0)
            pubkeyMap.insert(rxPubKey.cap(1), fileInfo.absoluteFilePath());
        if (rxDigest.indexIn(fileInfo.fileName()) >= 0) {
            if (rxDigest.cap(2) == QLatin1String("md5"))
                md5Map.insert(rxDigest.cap(1), fileInfo.absoluteFilePath());
            else
                sha1Map.insert(rxDigest.cap(1), fileInfo.absoluteFilePath());
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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
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
    QCOMPARE(cert1.effectiveDate(), cert2.effectiveDate());
    QCOMPARE(cert1.expiryDate(), cert2.expiryDate());
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

    QVERIFY(!certificate.isNull());

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

void tst_QSslCertificate::alternateSubjectNames_data()
{
    QTest::addColumn<QString>("certFilePath");
    QTest::addColumn<QSsl::EncodingFormat>("format");
    QTest::addColumn<QString>("subjAltNameFilePath");

    foreach (CertInfo certInfo, certInfoList) {
        QString certName = certInfo.fileInfo.fileName();
        if (subjAltNameMap.contains(certName))
            QTest::newRow(certName.toLatin1())
                << certInfo.fileInfo.absoluteFilePath()
                << certInfo.format
                << subjAltNameMap.value(certName);
    }
}

void tst_QSslCertificate::alternateSubjectNames()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, certFilePath);
    QFETCH(QSsl::EncodingFormat, format);
    QFETCH(QString, subjAltNameFilePath);

    QByteArray encodedCert = readFile(certFilePath);
    QSslCertificate certificate(encodedCert, format);
    QVERIFY(!certificate.isNull());

    QByteArray fileContents = readFile(subjAltNameFilePath);

    const QMultiMap<QSsl::AlternateNameEntryType, QString> altSubjectNames =
        certificate.alternateSubjectNames();

    // verify that each entry in subjAltNames is present in fileContents
    QMapIterator<QSsl::AlternateNameEntryType, QString> it(altSubjectNames);
    while (it.hasNext()) {
        it.next();
        QString type;
        if (it.key() == QSsl::EmailEntry)
            type = QLatin1String("email");
        else if (it.key() == QSsl::DnsEntry)
            type = QLatin1String("DNS");
        else
            QFAIL("unsupported alternative name type");
        QString entry = QString("%1:%2").arg(type).arg(it.value());
        QVERIFY(fileContents.contains(entry.toAscii()));
    }

    // verify that each entry in fileContents is present in subjAltNames
    QRegExp rx(QLatin1String("(email|DNS):([^,\\n]+)"));
    for (int pos = 0; (pos = rx.indexIn(fileContents, pos)) != -1; pos += rx.matchedLength()) {
        QSsl::AlternateNameEntryType key;
        if (rx.cap(1) == QLatin1String("email"))
            key = QSsl::EmailEntry;
        else if (rx.cap(1) == QLatin1String("DNS"))
            key = QSsl::DnsEntry;
        else
            QFAIL("unsupported alternative name type");
        QVERIFY(altSubjectNames.contains(key, rx.cap(2)));
    }
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

void tst_QSslCertificate::fromDevice()
{
    QTest::ignoreMessage(QtWarningMsg, "QSslCertificate::fromDevice: cannot read from a null device");
    QList<QSslCertificate> certs = QSslCertificate::fromDevice(0); // don't crash
    QVERIFY(certs.isEmpty());
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslCertificate)
#include "tst_qsslcertificate.moc"

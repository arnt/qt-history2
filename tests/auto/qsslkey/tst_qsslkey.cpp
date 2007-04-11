/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsslkey.h>
#include <qsslsocket.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>

class tst_QSslKey : public QObject
{
    Q_OBJECT

    struct KeyInfo {
        QFileInfo fileInfo;
        QSsl::Algorithm algorithm;
        QSsl::KeyType type;
        int length;
        QSsl::EncodingFormat format;
        KeyInfo(
            const QFileInfo &fileInfo, QSsl::Algorithm algorithm, QSsl::KeyType type,
            int length, QSsl::EncodingFormat format)
            : fileInfo(fileInfo), algorithm(algorithm), type(type), length(length)
            , format(format) {}
    };

    QList<KeyInfo> keyInfoList;

    void createPlainTestRows();

public:
    tst_QSslKey();
    virtual ~tst_QSslKey();

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL

private slots:
    void emptyConstructor();
    void constructor_data();
    void constructor();
    void copyAndAssign_data();
    void copyAndAssign();
    void length_data();
    void length();
    void toPemOrDer_data();
    void toPemOrDer();
    void toEncryptedPemOrDer_data();
    void toEncryptedPemOrDer();

#endif
};

tst_QSslKey::tst_QSslKey()
{
#ifdef Q_WS_MAC
    // applicationDirPath() points to a path inside the app bundle on Mac.
    QDir dir(qApp->applicationDirPath() + QLatin1String("/../../../keys"));
#else
    QDir dir(qApp->applicationDirPath() + QLatin1String("/keys"));
#endif
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Readable);
    QRegExp rx(QLatin1String("^(rsa|dsa)-(pub|pri)-(\\d+)\\.(pem|der)$"));
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (rx.indexIn(fileInfo.fileName()) >= 0)
            keyInfoList << KeyInfo(
                fileInfo,
                rx.cap(1) == QLatin1String("rsa") ? QSsl::Rsa : QSsl::Dsa,
                rx.cap(2) == QLatin1String("pub") ? QSsl::PublicKey : QSsl::PrivateKey,
                rx.cap(3).toInt(),
                rx.cap(4) == QLatin1String("pem") ? QSsl::Pem : QSsl::Der);
    }
}

tst_QSslKey::~tst_QSslKey()
{
}

void tst_QSslKey::initTestCase_data()
{
}

void tst_QSslKey::init()
{
}

void tst_QSslKey::cleanup()
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

void tst_QSslKey::emptyConstructor()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslKey key;
    QVERIFY(key.isNull());
    QVERIFY(key.length() < 0);
}

Q_DECLARE_METATYPE(QSsl::Algorithm);
Q_DECLARE_METATYPE(QSsl::KeyType);
Q_DECLARE_METATYPE(QSsl::EncodingFormat);

void tst_QSslKey::createPlainTestRows()
{
    QTest::addColumn<QString>("absFilePath");
    QTest::addColumn<QSsl::Algorithm>("algorithm");
    QTest::addColumn<QSsl::KeyType>("type");
    QTest::addColumn<int>("length");
    QTest::addColumn<QSsl::EncodingFormat>("format");
    foreach (KeyInfo keyInfo, keyInfoList) {
        QTest::newRow(keyInfo.fileInfo.fileName().toLatin1())
            << keyInfo.fileInfo.absoluteFilePath() << keyInfo.algorithm << keyInfo.type
            << keyInfo.length << keyInfo.format;
    }
}

void tst_QSslKey::constructor_data()
{
    createPlainTestRows();
}

void tst_QSslKey::constructor()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::Algorithm, algorithm);
    QFETCH(QSsl::KeyType, type);
    QFETCH(QSsl::EncodingFormat, format);

    QByteArray encoded = readFile(absFilePath);
    QSslKey key(encoded, algorithm, format, type);
    QVERIFY(!key.isNull());
}

void tst_QSslKey::copyAndAssign_data()
{
    createPlainTestRows();
}

void tst_QSslKey::copyAndAssign()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::Algorithm, algorithm);
    QFETCH(QSsl::KeyType, type);
    QFETCH(QSsl::EncodingFormat, format);

    QByteArray encoded = readFile(absFilePath);
    QSslKey key(encoded, algorithm, format, type);

    QSslKey copied(key);
    QCOMPARE(key.algorithm(), copied.algorithm());
    QCOMPARE(key.type(), copied.type());
    QCOMPARE(key.length(), copied.length());
    QCOMPARE(key.toPem(), copied.toPem());
    QCOMPARE(key.toDer(), copied.toDer());

    QSslKey assigned = key;
    QCOMPARE(key.algorithm(), assigned.algorithm());
    QCOMPARE(key.type(), assigned.type());
    QCOMPARE(key.length(), assigned.length());
    QCOMPARE(key.toPem(), assigned.toPem());
    QCOMPARE(key.toDer(), assigned.toDer());
}

void tst_QSslKey::length_data()
{
    createPlainTestRows();
}

void tst_QSslKey::length()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::Algorithm, algorithm);
    QFETCH(QSsl::KeyType, type);
    QFETCH(int, length);
    QFETCH(QSsl::EncodingFormat, format);

    QByteArray encoded = readFile(absFilePath);
    QSslKey key(encoded, algorithm, format, type);
    QVERIFY(!key.isNull());
    QCOMPARE(key.length(), length);
}

void tst_QSslKey::toPemOrDer_data()
{
    createPlainTestRows();
}

void tst_QSslKey::toPemOrDer()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::Algorithm, algorithm);
    QFETCH(QSsl::KeyType, type);
    QFETCH(QSsl::EncodingFormat, format);

    QByteArray encoded = readFile(absFilePath);
    QSslKey key(encoded, algorithm, format, type);
    QVERIFY(!key.isNull());
    QCOMPARE(format == QSsl::Pem ? key.toPem() : key.toDer(), encoded);
}

void tst_QSslKey::toEncryptedPemOrDer_data()
{
    QTest::addColumn<QString>("absFilePath");
    QTest::addColumn<QSsl::Algorithm>("algorithm");
    QTest::addColumn<QSsl::KeyType>("type");
    QTest::addColumn<QSsl::EncodingFormat>("format");
    QTest::addColumn<QString>("password");

    QStringList passwords;
//     passwords << "" << " " << "foobar" << "foo bar"
//               << "aAzZ`1234567890-=~!@#$%^&*()_+[]{}\\|;:'\",.<>/?";
    passwords << "foobar";
    foreach (KeyInfo keyInfo, keyInfoList) {
        foreach (QString password, passwords) {
            QString testName = QString("%1-%2-%3-%4").arg(keyInfo.fileInfo.fileName())
                .arg(keyInfo.algorithm == QSsl::Rsa ? "RSA" : "DSA")
                .arg(keyInfo.type == QSsl::PrivateKey ? "PrivateKey" : "PublicKey")
                .arg(keyInfo.format == QSsl::Pem ? "PEM" : "DER");
            QTest::newRow(testName.toLatin1())
                << keyInfo.fileInfo.absoluteFilePath() << keyInfo.algorithm << keyInfo.type
                << keyInfo.format << password;
        }
    }
}

void tst_QSslKey::toEncryptedPemOrDer()
{
    if (!QSslSocket::supportsSsl())
        return;

    QFETCH(QString, absFilePath);
    QFETCH(QSsl::Algorithm, algorithm);
    QFETCH(QSsl::KeyType, type);
    QFETCH(QSsl::EncodingFormat, format);
    QFETCH(QString, password);

    QByteArray plain = readFile(absFilePath);
    QSslKey key(plain, algorithm, format, type);
    QVERIFY(!key.isNull());

    QByteArray pwBytes(password.toLatin1());

    if (type == QSsl::PrivateKey) {
        QByteArray encryptedPem = key.toPem(pwBytes);
        QVERIFY(!encryptedPem.isEmpty());
        QSslKey keyPem(encryptedPem, algorithm, QSsl::Pem, type, pwBytes);
        QVERIFY(!keyPem.isNull());
        QCOMPARE(keyPem.toPem(), key.toPem());
    } else {
        // verify that public keys are never encrypted by toPem()
        QByteArray encryptedPem = key.toPem(pwBytes);
        QVERIFY(!encryptedPem.isEmpty());
        QByteArray plainPem = key.toPem();
        QVERIFY(!plainPem.isEmpty());
        QCOMPARE(encryptedPem, plainPem);
    }

    if (type == QSsl::PrivateKey) {
        QByteArray encryptedDer = key.toDer(pwBytes);
        // ### at this point, encryptedDer is invalid, hence the below QEXPECT_FAILs
        QVERIFY(!encryptedDer.isEmpty());
        QSslKey keyDer(encryptedDer, algorithm, QSsl::Der, type, pwBytes);
        if (type == QSsl::PrivateKey)
            QEXPECT_FAIL(
                QTest::currentDataTag(), "We're not able to decrypt these yet...", Continue);
        QVERIFY(!keyDer.isNull());
        if (type == QSsl::PrivateKey)
            QEXPECT_FAIL(
                QTest::currentDataTag(), "We're not able to decrypt these yet...", Continue);
        QCOMPARE(keyDer.toPem(), key.toPem());
    } else {
        // verify that public keys are never encrypted by toDer()
        QByteArray encryptedDer = key.toDer(pwBytes);
        QVERIFY(!encryptedDer.isEmpty());
        QByteArray plainDer = key.toDer();
        QVERIFY(!plainDer.isEmpty());
        QCOMPARE(encryptedDer, plainDer);
    }

    // ### add a test to verify that public keys are _decrypted_ correctly (by the ctor)
}

#endif

QTEST_MAIN(tst_QSslKey)
#include "tst_qsslkey.moc"

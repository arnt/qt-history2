#include <QtCore/QtCore>
#include <QtTest/QtTest>

class tst_Headers: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void licenseCheck_data() { allHeadersData(); }
    void licenseCheck();

    void privateSlots_data() { allHeadersData(); }
    void privateSlots();

    void macros_data() { allHeadersData(); }
    void macros();

private:
    void allHeadersData();
    QStringList headers;
};

QStringList getHeaders(const QString &path)
{
    QStringList headers;

    QDir dir(path);
    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (QString subdir, dirs)
        headers += getHeaders(path + "/" + subdir);

    QStringList entries = dir.entryList(QStringList("*.h"), QDir::Files);
    QRegExp reg("^(?!ui_)");
    entries = entries.filter(reg);
    foreach (QString entry, entries)
        headers += path + "/" + entry;

    return headers;
}

void tst_Headers::initTestCase()
{
    headers = getHeaders(QString::fromLocal8Bit(qgetenv("QTDIR")) + "/src");
    if (headers.isEmpty())
        QSKIP("can't find any headers in your $QTDIR/src", SkipAll);
}

void tst_Headers::allHeadersData()
{
    QTest::addColumn<QString>("header");

    foreach (QString hdr, headers) {
        if (hdr.contains("/3rdparty/") || hdr.endsWith("/qclass_lib_map.h"))
            continue;

        QTest::newRow(qPrintable(hdr)) << hdr;
    }
}

void tst_Headers::licenseCheck()
{
    QFETCH(QString, header);

    if (header.endsWith("/qgifhandler.h") || header.endsWith("/qconfig.h"))
        return;

    QFile f(header);
    QVERIFY(f.open(QIODevice::ReadOnly));

    QStringList content = QString::fromLocal8Bit(f.readAll()).split("\n");
    QString licenseType = content.at(6).contains("$TROLLTECH_")
        ? content.at(6).mid(14, content.at(6).count() - 9 - 14)
        : content.at(4).mid(14, content.at(4).count() - 9 - 14);

    int i = 0;

    QCOMPARE(content.at(i++), QString("/****************************************************************************"));
    if (licenseType != "3RDPARTY") {
        QCOMPARE(content.at(i++), QString("**"));
        QCOMPARE(content.at(i++), QString("** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved."));
    }
    QCOMPARE(content.at(i++), QString("**"));
    QCOMPARE(content.at(i++), QString("** This file is part of the $MODULE$ of the Qt Toolkit."));
    QCOMPARE(content.at(i++), QString("**"));
    QVERIFY(content.at(i).startsWith("** $TROLLTECH_"));
    QVERIFY(licenseType == "DUAL" || licenseType == "COMMERCIAL" || licenseType == "INTERNAL"
            || licenseType == "3RDPARTY");
    QVERIFY(content.at(i++).endsWith("_LICENSE$"));
    QCOMPARE(content.at(i++), QString("**"));
    QCOMPARE(content.at(i++), QString("** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE"));
    QCOMPARE(content.at(i++), QString("** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."));
    QCOMPARE(content.at(i++), QString("**"));
    QCOMPARE(content.at(i++), QString("****************************************************************************/"));
}

void tst_Headers::privateSlots()
{
    QFETCH(QString, header);

    if (header.endsWith("/qobjectdefs.h"))
        return;

    QFile f(header);
    QVERIFY(f.open(QIODevice::ReadOnly));

    QStringList content = QString::fromLocal8Bit(f.readAll()).split("\n");
    foreach (QString line, content) {
        if (line.contains("Q_PRIVATE_SLOT("))
            QVERIFY(line.contains("_q_"));
    }
}

void tst_Headers::macros()
{
    QFETCH(QString, header);

    if (header.endsWith("_p.h") || header.endsWith("_pch.h")
        || header.contains("global/qconfig-") || header.endsWith("/qconfig.h")
        || header.contains("/src/tools/") || header.contains("/src/plugins/")
        || header.endsWith("/qiconset.h") || header.endsWith("/qfeatures.h"))
        return;

    QFile f(header);
    QVERIFY(f.open(QIODevice::ReadOnly));

    QStringList content = QString::fromLocal8Bit(f.readAll()).split("\n");

    int beginHeader = content.indexOf("QT_BEGIN_HEADER");
    int endHeader = content.indexOf("QT_END_HEADER");

    QVERIFY(beginHeader >= 0);
    QVERIFY(endHeader >= 0);
    QVERIFY(beginHeader < endHeader);

    QVERIFY(content.indexOf(QRegExp("\\bslots\\s*:")) == -1);
    QVERIFY(content.indexOf(QRegExp("\\bsignals\\s*:")) == -1);

    if (header.contains("/sql/drivers/") || header.contains("/arch/qatomic")
        || header.endsWith("qglobal.h") || header.endsWith("qt_windows.h")
        || header.endsWith("qwindowdefs_win.h"))
        return;

    int qtmodule = content.indexOf(QRegExp("^QT_MODULE\\(.*\\)$"));
    QVERIFY(qtmodule != -1);
    QVERIFY(qtmodule > beginHeader && qtmodule < endHeader);
}

QTEST_MAIN(tst_Headers)
#include "tst_headers.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtTest/QtTest>
#include <QtXml/QXmlStreamReader>

class tst_Selftests: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void runSubTest_data();
    void runSubTest();
    void checkXML() const;
    void checkXML_data();

private:
    QStringList m_checkXMLBlacklist;
};

static QList<QByteArray> splitLines(QByteArray ba)
{
    ba.replace('\r', "");
    return ba.split('\n');
}

static QList<QByteArray> expectedResult(const QString &subdir)
{
    QFile file(":/expected_" + subdir + ".txt");
    if (!file.open(QIODevice::ReadOnly))
        return QList<QByteArray>();
    return splitLines(file.readAll());
}

void tst_Selftests::runSubTest_data()
{
    QTest::addColumn<QString>("subdir");
    QTest::newRow("subtest") << "subtest";
    QTest::newRow("warnings") << "warnings";
    QTest::newRow("maxwarnings") << "maxwarnings";
    QTest::newRow("cmptest") << "cmptest";
//    QTest::newRow("alive") << "alive"; // timer dependent
    QTest::newRow("globaldata") << "globaldata";
    QTest::newRow("skipglobal") << "skipglobal";
    QTest::newRow("skip") << "skip";
    QTest::newRow("strcmp") << "strcmp";
    QTest::newRow("expectfail") << "expectfail";
    QTest::newRow("sleep") << "sleep";
    QTest::newRow("fetchbogus") << "fetchbogus";
    QTest::newRow("crashes") << "crashes";
    QTest::newRow("multiexec") << "multiexec";
    QTest::newRow("failinit") << "failinit";
    QTest::newRow("failinitdata") << "failinitdata";
    QTest::newRow("skipinit") << "skipinit";
    QTest::newRow("skipinitdata") << "skipinitdata";
    QTest::newRow("datetime") << "datetime";
    QTest::newRow("singleskip") << "singleskip";

    //on windows assert does nothing in release mode and blocks execution with a popup window in debug mode
#if !defined(Q_OS_WIN) 
    QTest::newRow("assert") << "assert";
#endif

    QTest::newRow("waitwithoutgui") << "waitwithoutgui";
    QTest::newRow("differentexec") << "differentexec";
    QTest::newRow("exception") << "exception";
    QTest::newRow("qexecstringlist") << "qexecstringlist";
}

void tst_Selftests::runSubTest()
{
    QFETCH(QString, subdir);

    QProcess proc;
    proc.setEnvironment(QStringList(""));
    proc.start(subdir + "/tst_" + subdir);
    QVERIFY(proc.waitForFinished());

    const QByteArray out(proc.readAllStandardOutput());
    const QByteArray err(proc.readAllStandardError());

#ifdef Q_OS_LINUX
    QVERIFY2(err.isEmpty() || err.trimmed() == "Glib dispatcher checking for g_thread_init()", err.constData());
#else
    QVERIFY2(err.isEmpty(), err.constData());
#endif
             

    QList<QByteArray> res = splitLines(out);
    QList<QByteArray> exp = expectedResult(subdir);

    if (exp.count() == 0) {
        QList<QList<QByteArray> > expArr;
        int i = 1;
        do {
            exp = expectedResult(subdir + QString("_%1").arg(i++));
            if (exp.count())
            expArr += exp;
        } while(exp.count());
        
        for (int j = 0; j < expArr.count(); ++j) {
            if (res.count() == expArr.at(j).count()) {
                exp = expArr.at(j);
                break;
            }
        }
    } else {
        QCOMPARE(res.count(), exp.count());
    }

    for (int i = 0; i < res.count(); ++i) {
        QByteArray line = res.at(i);
        if (line.startsWith("Config: Using QTest"))
            continue;
        // the __FILE__ __LINE__ output is compiler dependent, skip it
        if (line.startsWith("   Loc: [") && line.endsWith(")]"))
            continue;
        if (line.endsWith(" : failure location"))
            continue;

        const QString output(QString::fromLatin1(line));
        const QString expected(QString::fromLatin1(exp.at(i)));

        if (line.contains("ASSERT") && output != expected)
            QEXPECT_FAIL("assert", "QTestLib prints out the absolute path.", Continue);

        /* On some platforms we compile without RTTI, and as a result we never throw an exception. */
        if(expected.startsWith(QLatin1String("tst_Exception::throwException() Caught unhandled exce")) && expected != output)
            QCOMPARE(output, QString::fromLatin1("tst_Exception::throwException()\n"));
        else
            QCOMPARE(output, expected);
    }
}

void tst_Selftests::initTestCase()
{
    m_checkXMLBlacklist.append("crashes"); // This test crashes
    m_checkXMLBlacklist.append("waitwithoutgui"); // This test is not a QTestLib test.

    /* Output from several tests is broken with the XML output method,
     * and it's quite heavy in the design. See task 155001. */
    m_checkXMLBlacklist.append("multiexec");
    m_checkXMLBlacklist.append("differentexec");
    m_checkXMLBlacklist.append("qexecstringlist");
}

void tst_Selftests::checkXML() const
{
    QFETCH(QString, subdir);

    if(m_checkXMLBlacklist.contains(subdir))
        return;

    QProcess proc;
    proc.setEnvironment(QStringList(""));
    proc.start(subdir + "/tst_" + subdir + " -xml");
    QVERIFY(proc.waitForFinished());

    QByteArray out(proc.readAllStandardOutput());
    QByteArray err(proc.readAllStandardError());

    /* Some platforms decides to output a message for uncaught exceptions. For instance,
     * this is what windows platforms says:
     * "This application has requested the Runtime to terminate it in an unusual way.
     * Please contact the application's support team for more information." */
    if(subdir != QLatin1String("exception"))
        QVERIFY2(err.isEmpty(), err.constData());

    QXmlStreamReader reader(out);

    while(!reader.atEnd())
        reader.readNext();

    QVERIFY(!reader.error());
}

void tst_Selftests::checkXML_data()
{
    runSubTest_data();
}

QTEST_MAIN(tst_Selftests)

#include "tst_selftests.moc"

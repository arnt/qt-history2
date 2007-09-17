/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QFile>
#include <QtTest/QtTest>

/*!
 \class tst_CLI
 \internal
 \short
 \since 4.4
 \brief Tests the command line interface, \c patternist, for the XQuery code.

 This test is not intended for testing the engine, but all the gluing the
 command line interface do: error reporting, query output, variable bindings, exit
 codes, and so on.

 In other words, if you have an engine bug; don't add it here because it won't be
 tested properly. Instead add it to the test suite.

 */
class tst_CLI : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void run() const;
    void run_data() const;
};

void tst_CLI::run() const
{
    QFETCH(int, expectedExitCode);
    QFETCH(QByteArray, expectedStandardOutput);
    QFETCH(QByteArray, expectedStandardError);
    QFETCH(QStringList, arguments);
    QFETCH(QString, cwd);

    QProcess process;
    process.setWorkingDirectory(QDir::current().absoluteFilePath(cwd));
    process.start(QLatin1String("patternist"), arguments);

    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QVERIFY(process.waitForFinished());

    QCOMPARE(process.exitCode(), expectedExitCode);
    QCOMPARE(process.readAllStandardOutput(), expectedStandardOutput);
    //QCOMPARE(process.readAllStandardError(), expectedStandardError); TODO Remove all the debug statements.
}

void tst_CLI::run_data() const
{
    /* Check one file for existsnce, to avoid possible false positives. */
    QVERIFY(QFile::exists(QLatin1String("queries/onePlusOne.xq")));

    QTest::addColumn<int>("expectedExitCode");
    QTest::addColumn<QByteArray>("expectedStandardOutput");
    QTest::addColumn<QByteArray>("expectedStandardError");
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<QString>("cwd");

    QTest::newRow("A simple math query")
        << 0
        << QByteArray("2")
        << QByteArray()
        << QStringList(QLatin1String("queries/onePlusOne.xq"))
        << QString();

    QTest::newRow("An unbound external variable")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/externalVariable.xq"))
        << QString();

    QTest::newRow("Bind an external variable")
        << 0
        << QByteArray("1 4<e>1</e>true")
        << QByteArray()
        << (QStringList() << QLatin1String("queries/externalVariable.xq")
                          << QLatin1String("externalVariable=1"))
        << QString();

    QTest::newRow("Use fn:doc")
        << 0
        << QByteArray("<e xmlns=\"http://example.com\" xmlns:p=\"http://example.com/P\" attr=\"1\" p:attr=\"\"><?target data?><!-- a comment --><e/>text <f/>text node</e>")
        << QByteArray()
        << QStringList(QLatin1String("queries/openDocument.xq"))
        << QString();

    QTest::newRow("Make sure query paths are resolved against CWD, not the location of the executable.")
        << 0
        << QByteArray("2")
        << QByteArray()
        << QStringList(QLatin1String("onePlusOne.xq"))
        << QString::fromLatin1("queries");

    QTest::newRow("Call fn:error()")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/errorFunction.xq"))
        << QString();

    QTest::newRow("Evaluate a library module")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/simpleLibraryModule.xq"))
        << QString();

    QFile globals(QLatin1String("baselines/globals.xml"));
    QVERIFY(globals.open(QIODevice::ReadOnly));

    QTest::newRow("Use an external variable multiple times.")
        << 0
        << globals.readAll()
        << QByteArray()
        << (QStringList() << QLatin1String("queries/reportGlobals.xq")
                          << QLatin1String("fileToOpen=globals.gccxml"))
        << QString();

    QTest::newRow("Trigger a static error.")
        << 1
        << globals.readAll()
        << QByteArray()
        << QStringList(QLatin1String("queries/staticError.xq"))
        << QString();

    QTest::newRow("Pass -help")
        << 0
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("-help"))
        << QString();

    QTest::newRow("Open an nonexistant file")
        << 4
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/ThisFileDoesNotExist.xq"))
        << QString();
            
    /* The following five tests exists to test the various
     * markup classes in the message. */
    QTest::newRow("XQuery-function")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/wrongArity.xq"))
        << QString();

    QTest::newRow("XQuery-type")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/typeError.xq"))
        << QString();

    QTest::newRow("XQuery-data & XQuery-keyword")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/zeroDivision.xq"))
        << QString();

    QTest::newRow("XQuery-uri")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/unsupportedCollation.xq"))
        << QString();

    QTest::newRow("XQuery-expression")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/invalidRegexp.xq"))
        << QString();
            
    /* The available flags are formatted in a complex way.  */
    QTest::newRow("Print a list of available regexp flags")
        << 1
        << QByteArray()
        << QByteArray()
        << QStringList(QLatin1String("queries/invalidRegexpFlag.xq"))
        << QString();
            

    // TODO pass external variables that allows space arround the equal sign.
    // TODO run fn:trace()
    // TODO Trigger warning
    // TODO what can we do with queries/nodeSequence.xq?
    // two query files on the command line.
}

QTEST_MAIN(tst_CLI)

#include "tst_cli.moc"
// vim: et:ts=4:sw=4:sts=4

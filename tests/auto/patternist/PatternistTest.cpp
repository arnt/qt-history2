/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
 */

#include <QtTest/QtTest>
#include <QStringList>
#include <QTemporaryFile>

#include "PatternistTest.h"
#include "PatternistTest.moc"

using namespace Patternist;

QTEST_MAIN(PatternistTest)

const static QLatin1String nonExistantFileWin     ("F://myDir\\example.com\\example.org\\doesNotExist.xq");
const static QLatin1String nonExistantFileUnix    ("/tmp/example.com/example.org/doesNotExist.xq");

PatternistTest::PatternistTest() : m_validQueryFile(new QTemporaryFile(this))
{
}

void PatternistTest::initTestCase()
{
    m_validQueryFile->open(QIODevice::ReadWrite);
    QTextStream out(m_validQueryFile);
    out << QLatin1String("1 + 1 eq 2");

    /* Check that the QTemporaryFile stuff didn't fail. */
    QVERIFY(QFile::exists(m_validQueryFile->fileName()));

    QVERIFY(!QFile::exists(nonExistantFileWin));
    QVERIFY(!QFile::exists(nonExistantFileUnix));
}

void PatternistTest::cleanupTestCase()
{
    m_validQueryFile->close();
}

void PatternistTest::shouldFail_data()
{
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<int>("expectedExitCode");

    QTest::newRow("No arguments is disallowed.")
        << QStringList()
        << 1;

    /*************************** --query *******************************/
    QTest::newRow("A URI must be specified after --query.")
        << (QStringList() << QLatin1String("--query"))
        << 2;

    QTest::newRow("Two --query options is invalid.")
        << (QStringList() << QLatin1String("--query") << m_validQueryFile->fileName()
                          << QLatin1String("--query") << m_validQueryFile->fileName())
        << 2;

    QTest::newRow("One --query option followed by an invalid --query option.")
        << (QStringList() << QLatin1String("--query") << m_validQueryFile->fileName()
                          << QLatin1String("--query"))
        << 2;

    QTest::newRow("One invalid --query option followed by a valid --query option.")
        << (QStringList() << QLatin1String("--query")
                          << QLatin1String("--query") << m_validQueryFile->fileName())
        << 2;

    QTest::newRow("One valid --query option followed by a URI.")
        << (QStringList() << QLatin1String("--query") << m_validQueryFile->fileName()
                          << m_validQueryFile->fileName())
        << 2;

    QTest::newRow("A URI followed by one valid --query option.")
        << (QStringList() << m_validQueryFile->fileName()
                          << QLatin1String("--query") << m_validQueryFile->fileName())
        << 2;


    QTest::newRow("A Windows-like URI is specified referencing non-existant file.")
        << (QStringList() << QLatin1String("--query") << nonExistantFileWin)
        << 2;

    QTest::newRow("A Unix-like URI is specified referencing non-existant file.")
        << (QStringList() << QLatin1String("--query") << nonExistantFileUnix)
        << 2;
    /*******************************************************************/




    /*********************** Invalid Options ***************************/
    QTest::newRow("No option by name --doesNotExist exists.")
        << (QStringList() << QLatin1String("--doesNotExist"))
        << 2;

            // Todo invalid URIs.

    QTest::newRow("No option by name -d exists.")
        << (QStringList() << QLatin1String("-d"))
        << 2;
    /*******************************************************************/
}

void PatternistTest::shouldFail()
{
    QFETCH(QStringList, arguments);
    QFETCH(int, expectedExitCode);

    QProcess process;
    process.start(QLatin1String("patternist"), arguments, QIODevice::NotOpen);

    const bool didNotTimeOut = process.waitForFinished();
    const int actualExitCode = process.exitCode();

    QVERIFY(didNotTimeOut);
    QVERIFY(process.exitStatus() == QProcess::NormalExit);
    QCOMPARE(expectedExitCode, actualExitCode);
}

// vim: et:ts=4:sw=4:sts=4

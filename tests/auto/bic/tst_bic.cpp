/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include "qbic.h"

#include <stdlib.h>

class tst_Bic: public QObject
{
    Q_OBJECT

public:
    tst_Bic();
    QBic::Info getCurrentInfo(const QString &libName);

private slots:
    void initTestCase_data();
    void initTestCase();

    void sizesAndVTables_data();
    void sizesAndVTables();

private:
    QBic bic;
};

typedef QPair<QString, QString> QStringPair;

tst_Bic::tst_Bic()
{
    bic.addBlacklistedClass(QLatin1String("std::*"));
    bic.addBlacklistedClass(QLatin1String("qIsNull*"));
    bic.addBlacklistedClass(QLatin1String("_*"));
    bic.addBlacklistedClass(QLatin1String("<anonymous*"));

    /* some system stuff we don't care for */
    bic.addBlacklistedClass(QLatin1String("timespec"));
    bic.addBlacklistedClass(QLatin1String("itimerspec"));
    bic.addBlacklistedClass(QLatin1String("sched_param"));
    bic.addBlacklistedClass(QLatin1String("timeval"));
    bic.addBlacklistedClass(QLatin1String("drand"));
    bic.addBlacklistedClass(QLatin1String("lconv"));
    bic.addBlacklistedClass(QLatin1String("random"));
    bic.addBlacklistedClass(QLatin1String("wait"));
    bic.addBlacklistedClass(QLatin1String("tm"));

    /* some bug in gcc also reported template instanciations */
    bic.addBlacklistedClass(QLatin1String("QTypeInfo<*>"));
    bic.addBlacklistedClass(QLatin1String("QMetaTypeId<*>"));
    bic.addBlacklistedClass(QLatin1String("QVector<QGradientStop>*"));

    /* this guy is never instanciated, just for compile-time checking */
    bic.addBlacklistedClass(QLatin1String("QMap<*>::PayloadNode"));

    /* QFileEngine was removed in 4.1 */
    bic.addBlacklistedClass(QLatin1String("QFileEngine"));
    bic.addBlacklistedClass(QLatin1String("QFileEngineHandler"));
    bic.addBlacklistedClass(QLatin1String("QFlags<QFileEngine::FileFlag>"));

    /* Private classes */
    bic.addBlacklistedClass(QLatin1String("QBrushData"));
}

void tst_Bic::initTestCase_data()
{
    QTest::addColumn<QString>("libName");

    QTest::newRow("QtCore") << "QtCore";
    QTest::newRow("QtGui") << "QtGui";
    QTest::newRow("QtSql") << "QtSql";
    QTest::newRow("QtSvg") << "QtSvg";
    QTest::newRow("QtNetwork") << "QtNetwork";
    QTest::newRow("QtOpenGL") << "QtOpenGL";
    QTest::newRow("QtXml") << "QtXml";
    QTest::newRow("Qt3Support") << "Qt3Support";
    QTest::newRow("QtTest") << "QtTest";
    QTest::newRow("QtDBus") << "QtDBus";
}

void tst_Bic::initTestCase()
{
    QString qtDir = QString::fromLocal8Bit(getenv("QTDIR"));
    QVERIFY2(!qtDir.isEmpty(), "This test needs $QTDIR");
}

void tst_Bic::sizesAndVTables_data()
{
#if !defined(Q_CC_GNU) || defined(Q_CC_INTEL)
    QSKIP("Test not implemented for this compiler/platform", SkipAll);
#else

    QString archFileName400;
    QString archFileName410;
    QString archFileName420;

#if defined Q_OS_LINUX && defined Q_WS_X11
# if defined(__powerpc__) && !defined(__powerpc64__)
    archFileName400 = "data/%1.4.0.0.linux-gcc-ppc32.txt";
    archFileName410 = "data/%1.4.1.0.linux-gcc-ppc32.txt";
    archFileName420 = "data/%1.4.2.0.linux-gcc-ppc32.txt";
# elif defined(__amd64__)
    archFileName400 = "data/%1.4.0.0.linux-gcc-amd64.txt";
# elif defined(__i386__)
    archFileName400 = "data/%1.4.0.0.linux-gcc-ia32.txt";
    archFileName410 = "data/%1.4.1.0.linux-gcc-ia32.txt";
    archFileName420 = "data/%1.4.2.0.linux-gcc-ia32.txt";
# endif
#elif defined Q_OS_AIX
    if (sizeof(void*) == 4)
        archFileName400 = "data/%1.4.0.0.aix-gcc-power32.txt";
#elif defined Q_OS_MAC && defined(__powerpc__)
    archFileName400 = "data/%1.4.0.0.macx-gcc-ppc32.txt";
    archFileName410 = "data/%1.4.1.0.macx-gcc-ppc32.txt";
    archFileName420 = "data/%1.4.2.0.macx-gcc-ppc32.txt";
#elif defined Q_OS_MAC && defined(__i386__)
    archFileName410 = "data/%1.4.1.0.macx-gcc-ia32.txt";
    archFileName420 = "data/%1.4.2.0.macx-gcc-ia32.txt";
#endif

    if (archFileName400.isEmpty() && archFileName410.isEmpty())
        QSKIP("No reference files found for this platform", SkipAll);

#if QT_VERSION >= 0x040100
    bool isPatchRelease400 = false;
#else
    bool isPatchRelease400 = true;
#endif

#if QT_VERSION >= 0x040200
    bool isPatchRelease410 = false;
#else
    bool isPatchRelease410 = true;
#endif

#if QT_VERSION >= 0x040300
    bool isPatchRelease420 = false;
#else
    bool isPatchRelease420 = true;
#endif

    QTest::addColumn<QString>("oldLib");
    QTest::addColumn<bool>("isPatchRelease");

    QTest::newRow("4.0") << archFileName400 << isPatchRelease400;
    QTest::newRow("4.1") << archFileName410 << isPatchRelease410;
    QTest::newRow("4.2") << archFileName420 << isPatchRelease420;
#endif
}

QBic::Info tst_Bic::getCurrentInfo(const QString &libName)
{
    QTemporaryFile tmpQFile;
    tmpQFile.open();
    QString tmpFileName = tmpQFile.fileName();

    QByteArray tmpFileContents = "#include<" + libName.toLatin1() + "/" + libName.toLatin1() + ">\n";
    tmpQFile.write(tmpFileContents);

    QString qtDir = QString::fromLocal8Bit(getenv("QTDIR"));
    QString compilerName = "g++";

    QStringList args;
    args << "-c"
         << "-I" + qtDir + "/include"
         << "-I/usr/X11R6/include/"
         << "-DQT_NO_STL" << "-DQT3_SUPPORT"
         << "-xc++"
#ifndef Q_OS_AIX
         << "-o" << "/dev/null"
#endif
         << "-fdump-class-hierarchy"
         << tmpFileName;

    QProcess proc;
    proc.start(compilerName, args, QIODevice::ReadOnly);
    if (!proc.waitForFinished(6000000)) {
        qWarning() << "gcc didn't finish" << proc.errorString();
        return QBic::Info();
    }

    QString errs = QString::fromLocal8Bit(proc.readAllStandardError().constData());
    if (!errs.isEmpty()) {
        qDebug() << "Arguments:" << args << "Warnings:" << errs;
        return QBic::Info();
    }

    QString resultFileName;
#if defined(Q_CC_GNU) && !defined(Q_CC_INTEL) && (__GNUC__ - 0 < 4)
    resultFileName = QFileInfo(tmpQFile).fileName() + ".class";
#else
    resultFileName = QFileInfo(tmpQFile).fileName() + ".t01.class";
#endif
    QBic::Info inf = bic.parseFile(resultFileName);

    QFile::remove(resultFileName);
    tmpQFile.close();

    return inf;
}

void tst_Bic::sizesAndVTables()
{
#if !defined(Q_CC_GNU) || defined(Q_CC_INTEL)
    QSKIP("Test not implemented for this compiler/platform", SkipAll);
#else

    QFETCH_GLOBAL(QString, libName);
    QFETCH(QString, oldLib);
    QFETCH(bool, isPatchRelease);

    bool isFailed = false;

    if (oldLib.isEmpty() || !QFile::exists(oldLib.arg(libName)))
        QSKIP("No platform spec found for this platform/version.", SkipSingle);

    const QBic::Info oldLibInfo = bic.parseFile(oldLib.arg(libName));
    QVERIFY(!oldLibInfo.classVTables.isEmpty());

    const QBic::Info currentLibInfo = getCurrentInfo(libName);
    QVERIFY(!currentLibInfo.classVTables.isEmpty());

    const QBic::VTableDiff diff = bic.diffVTables(oldLibInfo, currentLibInfo);
    if (!diff.removedVTables.isEmpty()) {
        qWarning() << "VTables for the following classes were removed" << diff.removedVTables;
        isFailed = true;
    }

    if (!diff.modifiedVTables.isEmpty()) {
        foreach(QStringPair entry, diff.modifiedVTables)
            qWarning() << "modified VTable:\n    Old: " << entry.first
                       << "\n    New: " << entry.second;
        isFailed = true;
    }

    if (isPatchRelease && !diff.addedVTables.isEmpty()) {
        qWarning() << "VTables for the following classes were added in a patch release:"
                   << diff.addedVTables;
        isFailed = true;
    }

    if (isPatchRelease && !diff.reimpMethods.isEmpty()) {
        foreach(QStringPair entry, diff.reimpMethods)
            qWarning() << "reimplemented virtual in patch release:\n    Old: " << entry.first
                       << "\n    New: " << entry.second;
        isFailed = true;
    }

    const QBic::SizeDiff sizeDiff = bic.diffSizes(oldLibInfo, currentLibInfo);
    if (!sizeDiff.mismatch.isEmpty()) {
        foreach (QString className, sizeDiff.mismatch)
            qWarning() << "size mismatch for" << className
                       << "old" << oldLibInfo.classSizes.value(className)
                       << "new" << currentLibInfo.classSizes.value(className);
        isFailed = true;
    }

    if (!sizeDiff.removed.isEmpty()) {
        qWarning() << "the following classes were removed:" << sizeDiff.removed;
        isFailed = true;
    }

    if (isPatchRelease && !sizeDiff.added.isEmpty()) {
        qWarning() << "the following classes were added in a patch release:" << sizeDiff.added;
        isFailed = true;
    }

    if (isFailed)
        QFAIL("Test failed, read warnings above.");
#endif
}

QTEST_APPLESS_MAIN(tst_Bic)

#include "tst_bic.moc"


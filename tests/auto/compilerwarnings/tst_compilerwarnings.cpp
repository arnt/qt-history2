/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qcoreapplication.h>
#include <qprocess.h>
#include <qtemporaryfile.h>
#include <qdebug.h>

#include <QtTest/QtTest>

#include <stdlib.h>

class tst_CompilerWarnings: public QObject
{
    Q_OBJECT

private slots:
    void warnings_data();
    void warnings();
};

/*
    Return list of all documented qfeatures (QT_NO_*)
 */
static QStringList getFeatures()
{
    QStringList srcDirs;
    srcDirs << QString::fromLocal8Bit(getenv("QTDIR"))
            << QString::fromLocal8Bit(getenv("QTSRCDIR"));

    QString featurefile;
    foreach (QString dir, srcDirs) {
        QString str = dir + "/src/corelib/global/qfeatures.txt";
        if (QFile::exists(str)) {
            featurefile = str;
            break;
        }
    }

    if (featurefile.isEmpty()) {
        qWarning("Unable to find qfeatures.txt");
        return QStringList();
    }

    QFile file(featurefile);
    if (!file.open(QIODevice::ReadOnly)) {
	qWarning("Unable to open feature file '%s'", qPrintable(featurefile));
	return QStringList();
    }

    QStringList features;
    QTextStream s(&file);
    QRegExp regexp("Feature:\\s+(\\w+)\\s*");
    for (QString line = s.readLine(); !s.atEnd(); line = s.readLine()) {
        if (regexp.exactMatch(line))
            features << regexp.cap(1);
    }

    return features;
}

void tst_CompilerWarnings::warnings_data()
{
    QTest::addColumn<QStringList>("cflags");

    QTest::newRow("standard") << QStringList();

#if 0
#ifdef Q_WS_QWS
    QStringList features = getFeatures();
    foreach (QString feature, features) {
        QStringList args;
        QString macro = QString("QT_NO_%1").arg(feature);
        args << (QString("-D%1").arg(macro));
        QTest::newRow(qPrintable(macro)) << args;
    }
#endif
#endif
}

void tst_CompilerWarnings::warnings()
{
    QFETCH(QStringList, cflags);

#if defined(Q_CC_GNU) && __GNUC__ == 3
    QSKIP("gcc 3.x outputs too many bogus warnings", Continue);
#endif

    static QString tmpFile;
    if (tmpFile.isEmpty()) {
        QTemporaryFile tmpQFile;
        tmpQFile.open();
        tmpFile = tmpQFile.fileName();
        tmpQFile.close();
    }

    QStringList args;
    QString compilerName;

    static QString qtDir = QString::fromLocal8Bit(getenv("QTDIR"));
    QVERIFY2(!qtDir.isEmpty(), "This test needs $QTDIR");

    args << cflags;
#if defined(Q_CC_GNU)
    compilerName = "g++";
    args << "-I" + qtDir + "/include";
    args << "-I/usr/X11R6/include/";
#ifdef Q_OS_HPUX
    args << "-I/usr/local/mesa/aCC-64/include";
#endif
    args << "-c";
    args << "-Wall" << "-Wold-style-cast" << "-Woverloaded-virtual" << "-pedantic" << "-ansi"
         << "-Wno-long-long" << "-Wshadow" << "-Wpacked" << "-Wunreachable-code"
         << "-Wundef" << "-Wchar-subscripts" << "-Wformat-nonliteral" << "-Wformat-security"
         << "-Wcast-align"
#if QT_VERSION >= 0x040100
         << "-Wfloat-equal"
#endif
         << "-o" << tmpFile
         << "test.cpp";
#elif defined(Q_CC_XLC)
    compilerName = "xlC_r";
    args << "-I" + qtDir + "/include"
# if QT_POINTER_SIZE == 8
         << "-q64"
# endif
         << "-c" << "-o" << tmpFile
         << "-info=all"
         << "test.cpp";
#elif defined(Q_CC_MSVC)
    compilerName = "cl";
    args << "-I" + qtDir + "/include"
         << "-nologo" << "-W3"
         << "-o" << tmpFile
         << "test.cpp";
#elif defined (Q_CC_SUN)
    compilerName = "CC";
    // +w or +w2 outputs too much bogus
    args << "-I" + qtDir + "/include"
# if QT_POINTER_SIZE == 8
         << "-xarch=v9"
# endif
         << "-o" << tmpFile
         << "test.cpp";
#elif defined (Q_CC_HPACC)
    compilerName = "aCC";
    args << "-I" + qtDir + "/include"
         << "-I/usr/local/mesa/aCC-64/include"
# if QT_POINTER_SIZE == 8
         << "+DA2.0W"
# endif
         << "-DQT_NO_STL" << "-c" << "+w" << "+W" << "392,655,818,887"
         << "-o" << tmpFile
         << "test.cpp";
#elif defined(Q_CC_MIPS)
    compilerName = "CC";
    args << "-I" + qtDir + "/include"
         << "-c"
         << "-woff" << "3303" // const qualifier on return
         << "-o" << tmpFile
         << "test.cpp";
#else
    QSKIP("Test not implemented for this compiler", SkipAll);
#endif

    QProcess proc;
    proc.start(compilerName, args, QIODevice::ReadOnly);
    QVERIFY2(proc.waitForFinished(6000000), proc.errorString().toLocal8Bit());

#ifdef Q_CC_MSVC
    QString errs = QString::fromLocal8Bit(proc.readAllStandardOutput().constData());
    if (errs.startsWith("test.cpp"))
        errs = errs.mid(10);
#else
    QString errs = QString::fromLocal8Bit(proc.readAllStandardError().constData());
#endif
    QStringList errList;
    if (!errs.isEmpty()) {
        errList = errs.split("\n");
        qDebug() << "Arguments:" << args;
        foreach (QString err, errList) {
            qDebug() << err;
        }
    }
    QCOMPARE(errList.count(), 0); // verbose info how many lines of errors in output
    QVERIFY(errs.isEmpty());
}

QTEST_APPLESS_MAIN(tst_CompilerWarnings)

#include "tst_compilerwarnings.moc"


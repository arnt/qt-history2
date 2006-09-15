/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qcoreapplication.h>
#include <qstring.h>
#include <qtemporaryfile.h>
#include <qfile.h>
#include <qdir.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/io/qtemporaryfile.h corelib/io/qtemporaryfile.cpp

class tst_QTemporaryFile : public QObject
{
    Q_OBJECT
public:
    tst_QTemporaryFile();
    virtual ~tst_QTemporaryFile();
public slots:
    void init();
    void cleanup();
private slots:
    void construction();
    void fileTemplate();
    void fileTemplate_data();
    void getSetCheck();
    void fileName();
    void autoRemove();
    void write();
    void openCloseOpenClose();
public:
};

void tst_QTemporaryFile::construction()
{
    QTemporaryFile file(0);
    QString tmp = QDir::tempPath();
    QCOMPARE(file.fileTemplate().left(tmp.size()), tmp);
    QCOMPARE(file.fileTemplate().at(tmp.size()), QChar('/'));
}

// Testing get/set functions
void tst_QTemporaryFile::getSetCheck()
{
    QTemporaryFile obj1;
    // bool QTemporaryFile::autoRemove()
    // void QTemporaryFile::setAutoRemove(bool)
    obj1.setAutoRemove(false);
    QCOMPARE(false, obj1.autoRemove());
    obj1.setAutoRemove(true);
    QCOMPARE(true, obj1.autoRemove());
}

tst_QTemporaryFile::tst_QTemporaryFile()
{
}

tst_QTemporaryFile::~tst_QTemporaryFile()
{

}

void tst_QTemporaryFile::init()
{
// TODO: Add initialization code here.
// This will be executed immediately before each test is run.
}

void tst_QTemporaryFile::cleanup()
{
// TODO: Add cleanup code here.
// This will be executed immediately after each test is run.
}

void tst_QTemporaryFile::fileTemplate_data()
{
    QTest::addColumn<QString>("constructorTemplate");
    QTest::addColumn<QString>("suffix");
    QTest::addColumn<QString>("fileTemplate");

    QTest::newRow("constructor default")          << "" << "" << "";
    QTest::newRow("constructor with xxx sufix") << "qt_XXXXXXxxx" << "xxx" << "";
    QTest::newRow("constructor with xXx sufix") << "qt_XXXXXXxXx" << "xXx" << "";
    QTest::newRow("constructor with no sufix") << "qt_XXXXXX" << "" << "";
    QTest::newRow("constructor with >6 X's and xxx suffix") << "qt_XXXXXXXXXXxxx" << "xxx" << "";
    QTest::newRow("constructor with >6 X's, no suffix") << "qt_XXXXXXXXXX" << "" << "";

    QTest::newRow("set template, no suffix") << "" << "" << "foo";
    QTest::newRow("set template, with lowercase XXXXXX") << "" << "xxxxxx" << "qt_XXXXXXxxxxxx";
    QTest::newRow("set template, with xxx") << "" << ".xxx" << "qt_XXXXXX.xxx";
    QTest::newRow("set template, with >6 X's") << "" << ".xxx" << "qt_XXXXXXXXXXXXXX.xxx";
    QTest::newRow("set template, with >6 X's, no suffix") << "" << "" << "qt_XXXXXXXXXXXXXX";
}

void tst_QTemporaryFile::fileTemplate()
{
#if QT_VERSION >= 0x040200
    QFETCH(QString, constructorTemplate);
    QFETCH(QString, suffix);
    QFETCH(QString, fileTemplate);

    QTemporaryFile file(constructorTemplate);
    if (!fileTemplate.isEmpty())
        file.setFileTemplate(fileTemplate);

    QCOMPARE(file.open(), true);

    QCOMPARE(file.fileName().right(suffix.length()), suffix);
    file.close();
#endif
}


/*
    This tests whether the temporary file really gets placed in QDir::tempPath
*/
void tst_QTemporaryFile::fileName()
{
    // Get QDir::tempPath and make an absolute path.
    QString tempPath = QDir::tempPath();
    QString absoluteTempPath = QDir(tempPath).absolutePath();
    QTemporaryFile file;
    file.setAutoRemove(true);
    file.open();
    QString fileName = file.fileName();
    QVERIFY(QFile::exists(fileName));
    // Get path to the temp file, whithout the file name.
    QString absoluteFilePath = QFileInfo(fileName).absolutePath();
#ifdef Q_OS_WIN
    absoluteFilePath = absoluteFilePath.toLower();
    absoluteTempPath = absoluteTempPath.toLower();
#endif
    QCOMPARE(absoluteFilePath, absoluteTempPath);
}

void tst_QTemporaryFile::autoRemove()
{
	// Test auto remove
	QString fileName;
	{
		QTemporaryFile file("tempXXXXXX");
		file.setAutoRemove(true);
		QVERIFY(file.open());
		fileName = file.fileName();
		file.close();
	}
	QVERIFY(!QFile::exists(fileName));

	// Test if disabling auto remove works.
	{
		QTemporaryFile file("tempXXXXXX");
		file.setAutoRemove(false);
		QVERIFY(file.open());
		fileName = file.fileName();
		file.close();
	}
	QVERIFY(QFile::exists(fileName));
	QVERIFY(QFile::remove(fileName));


	// Do not explicitly call setAutoRemove (tests if it really is the default as documented)
	{
		QTemporaryFile file("tempXXXXXX");
		QVERIFY(file.open());
		fileName = file.fileName();
		file.close();
	}
	QVERIFY(!QFile::exists(fileName));

}

void tst_QTemporaryFile::write()
{
    QByteArray data("OLE\nOLE\nOLE");
    QTemporaryFile file;
    QVERIFY(file.open());
    QCOMPARE((int)file.write(data), data.size());
    file.close();
}

void tst_QTemporaryFile::openCloseOpenClose()
{
#if QT_VERSION < 0x040101
    QSKIP("Until Qt 4.1.1, QTemporaryFile would create a new name every time open() was called.", SkipSingle);
#endif
    QString fileName;
    {
        // Create a temp file
        QTemporaryFile file("tempXXXXXX");
        file.setAutoRemove(true);
        QVERIFY(file.open());
        file.write("OLE");
        fileName = file.fileName();
        QVERIFY(QFile::exists(fileName));
        file.close();

        // Check that it still exists after being closed        
        QVERIFY(QFile::exists(fileName));
        QVERIFY(!file.isOpen());
        QVERIFY(file.open());
        QCOMPARE(file.readAll(), QByteArray("OLE"));
        // Check that it's still the same file after being opened again.
        QCOMPARE(file.fileName(), fileName);
    }
    QVERIFY(!QFile::exists(fileName));
}

QTEST_MAIN(tst_QTemporaryFile)
#include "tst_qtemporaryfile.moc"

#include <QtTest/QtTest>
#include "testlupdate.h"
#include <QtCore/QDir>
#include <QtCore/QFile>

class tst_lupdate : public QObject
{
    Q_OBJECT
public:
    tst_lupdate();

private slots:
    void parse_data();
    void parse();
    void merge_data();
    void merge();
    
private:
    TestLUpdate m_lupdate;
    QString m_basePath;
};

tst_lupdate::tst_lupdate()
{
    
    m_basePath = QDir::currentPath() + QLatin1String("/testdata/");
}

void tst_lupdate::parse_data()
{
    QTest::addColumn<QString>("inputprofile");
    QTest::addColumn<QString>("generatedtsfile");
    QTest::addColumn<QString>("expectedtsfile");
    
    QTest::newRow("Parse UI file") << QString("parseui/project.pro") << QString() << QString();
    QTest::newRow("Parse cpp file") << QString("parsecpp/project.pro") << QString() << QString();
}

void tst_lupdate::parse()
{

    QFETCH(QString, inputprofile);
    QFETCH(QString, generatedtsfile);
    QFETCH(QString, expectedtsfile);

    if (inputprofile.startsWith("/")) inputprofile.remove(0,1);
    QString inputpro = m_basePath + inputprofile;

    QString casePath = QFileInfo(inputpro).absolutePath();
    // If the testcase did not give a filename, assume that is uses the default filenames.
    if (generatedtsfile.isNull()) {
        generatedtsfile = casePath + QLatin1String("/project_no.ts");
    } else {
        if (generatedtsfile.startsWith("/")) generatedtsfile.remove(0,1);
        generatedtsfile = m_basePath + generatedtsfile;
    }

    if (expectedtsfile.isNull()) {
        QString base = QFileInfo(inputpro).absolutePath();
        expectedtsfile = base + QLatin1String("/project_no.ts.result");
    } else {
        if (expectedtsfile.startsWith("/")) expectedtsfile.remove(0,1);
        expectedtsfile = m_basePath + expectedtsfile;
    }

    // qmake will delete the previous one, to ensure that we don't do any merging....
    m_lupdate.setWorkingDirectory(casePath);

    m_lupdate.qmake();
    m_lupdate.updateProFile(inputpro);
    
    QFile file1(generatedtsfile);
    bool ok = file1.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    QByteArray data1 = file1.readAll();
    QString str1(data1);
    
    QFile file2(expectedtsfile);
    ok = file2.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    QByteArray data2 = file2.readAll();
    QString str2(data2);

    str1 = str1.replace(QLatin1String("\015\012"), QLatin1String("\n"));
    str2 = str2.replace(QLatin1String("\015\012"), QLatin1String("\n"));

    QCOMPARE(str1, str2);


}

void tst_lupdate::merge_data()
{
    QTest::addColumn<QString>("inputprofile");
    QTest::addColumn<QString>("generatedtsfile");
    QTest::addColumn<QString>("expectedtsfile");
    
    QTest::newRow("Merge UI file") << QString("mergeui/project.pro") << QString() << QString();
    QTest::newRow("Merge CPP file") << QString("mergecpp/project.pro") << QString() << QString();
    QTest::newRow("mergecpp, (Merge CPP file, obsolete the old and create a new one)") << QString("mergecpp_obsolete/project.pro") << QString() << QString();
    QTest::newRow("merge_versions. Make sure that we can merge an old ts with a new version ts file.") 
                    << QString("merge_versions/project.pro") << QString() << QString();
}

void tst_lupdate::merge()
{
    QFETCH(QString, inputprofile);
    QFETCH(QString, generatedtsfile);
    QFETCH(QString, expectedtsfile);

    if (inputprofile.startsWith("/")) inputprofile.remove(0,1);
    QString inputpro = m_basePath + inputprofile;
    QString casePath = QFileInfo(inputpro).absolutePath();

    // If the testcase did not give a filename, assume that is uses the default filenames.
    if (generatedtsfile.isNull()) {
        generatedtsfile = casePath + QLatin1String("/project_no.ts");
    } else {
        if (generatedtsfile.startsWith("/")) generatedtsfile.remove(0,1);
        generatedtsfile = m_basePath + generatedtsfile;
    }

    if (expectedtsfile.isNull()) {
        QString base = QFileInfo(inputpro).absolutePath();
        expectedtsfile = base + QLatin1String("/project_no.ts.result");
    } else {
        if (expectedtsfile.startsWith("/")) expectedtsfile.remove(0,1);
        expectedtsfile = m_basePath + expectedtsfile;
    }

    // copy it to a temporary file, since we can't write to a read-only file (usually, its not checked out editable from the depot)
    // This also enables us to run the test many times in a sequence, without changing the input data.
    QString tmpTSFile = generatedtsfile + QLatin1String(".tmp");

    m_lupdate.setWorkingDirectory(casePath);
    
    // qmake will ensure that the ts.tmp file is created.
    m_lupdate.qmake();
    m_lupdate.updateProFile(inputpro);
    
    QFile file1(tmpTSFile);
    bool ok = file1.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    QByteArray data1 = file1.readAll();
    QString str1(data1);
    file1.close();
    
    QFile file2(expectedtsfile);
    ok = file2.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    QByteArray data2 = file2.readAll();
    QString str2(data2);
    file2.close();

    str1 = str1.replace(QLatin1String("\015\012"), QLatin1String("\n"));
    str2 = str2.replace(QLatin1String("\015\012"), QLatin1String("\n"));

    QCOMPARE(str1, str2);


}


QTEST_MAIN(tst_lupdate)
#include "tst_lupdate.moc"

/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include "../../../src/gui/dialogs/qfilesystemmodel_p.h"
#include <QFileIconProvider>

//TESTED_CLASS=QFileSystemModel
//TESTED_FILES=qfilesystemmodel.h qfilesystemmodel.cpp

#define WAITTIME 500

class tst_QFileSystemModel : public QObject {
  Q_OBJECT

public:
    tst_QFileSystemModel();
    virtual ~tst_QFileSystemModel();

public Q_SLOTS:
    void init();
    void cleanup();

private slots:
    void rootPath();
    void naturalCompare_data();
    void naturalCompare();
    void readOnly();
    void iconProvider();

    void rowCount();

    void rowsInserted_data();
    void rowsInserted();

    void rowsRemoved_data();
    void rowsRemoved();

    void dataChanged_data();
    void dataChanged();

#ifdef Q_OS_UNIX
    // TODO just copy the test out of QDir
    void filters_data();
    void filters();
#endif

    void nameFilters();

protected:
    bool createFiles(const QString &test_path, const QStringList &initial_files, const QStringList &intial_dirs = QStringList(), const QString &baseDir = QDir::temp().absolutePath());

private:
    QFileSystemModel *model;
};

tst_QFileSystemModel::tst_QFileSystemModel() : model(0)
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

tst_QFileSystemModel::~tst_QFileSystemModel()
{
}

void tst_QFileSystemModel::init()
{
    cleanup();
    QCOMPARE(model, (QFileSystemModel*)0);
    model = new QFileSystemModel;
}

void tst_QFileSystemModel::cleanup()
{
    delete model;
    model = 0;
    QString tmp = QDir::temp().path() + QDir::separator() + QString("flatdirtest");
    QDir dir(tmp);
    if (dir.exists(tmp)) {
        QStringList list = dir.entryList(QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
        for (int i = 0; i < list.count(); ++i) {
            QFileInfo fi(dir.path() + QDir::separator() + list.at(i));
            if (fi.exists() && fi.isFile())
                QVERIFY(QFile::remove(tmp + QDir::separator() + list.at(i)));
            if (fi.exists() && fi.isDir())
                QVERIFY(dir.rmdir(list.at(i)));
        }
        list = dir.entryList(QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
        QVERIFY(list.count() == 0);
        QVERIFY(dir.rmdir(tmp));
    }
}

void tst_QFileSystemModel::rootPath()
{
    QCOMPARE(model->rootPath(), QString(QDir().path()));

    QSignalSpy rootChanged(model, SIGNAL(rootPathChanged(const QString &)));
    QModelIndex root = model->setRootPath(model->rootPath());
    root = model->setRootPath("this directory shouldn't exist");
    QCOMPARE(rootChanged.count(), 0);

    QString oldRootPath = model->rootPath();
    root = model->setRootPath(QDir::homePath());
    QTest::qWait(WAITTIME * 2);
    QCOMPARE(model->rootPath(), QString(QDir::homePath()));
    QCOMPARE(rootChanged.count(), oldRootPath == model->rootPath() ? 0 : 1);
    QVERIFY(model->rowCount(root) > 0);
    QCOMPARE(model->rootDirectory().absolutePath(), QDir::homePath());
}

void tst_QFileSystemModel::naturalCompare_data()
{
    QTest::addColumn<QString>("s1");
    QTest::addColumn<QString>("s2");
    QTest::addColumn<int>("caseSensitive");
    QTest::addColumn<int>("result");
    QTest::addColumn<int>("swap");
    for (int j = 0; j < 4; ++j) { // <- set a prefix and a postfix string (not numbers)
        QString prefix = (j == 0 || j == 1) ? "b" : "";
        QString postfix = (j == 1 || j == 2) ? "y" : "";

        for (int k = 0; k < 3; ++k) { // <- make 0 not a special case
            QString num = QString("%1").arg(k);
            QString nump = QString("%1").arg(k + 1);
            for (int i = 10; i < 12; ++i) { // <- swap s1 and s2 and reverse the result
                QTest::newRow("basic") << prefix + "0" + postfix << prefix + "0" + postfix << int(Qt::CaseInsensitive) << 0;

                // s1 should always be less then s2
                QTest::newRow("just text")    << prefix + "fred" + postfix     << prefix + "jane" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("just numbers") << prefix + num + postfix        << prefix + "9" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("zero")         << prefix + num + postfix        << prefix + "0" + nump + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("space b")      << prefix + num + postfix        << prefix + " " + nump + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("space a")      << prefix + num + postfix        << prefix + nump + " " + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("tab b")        << prefix + num + postfix        << prefix + "    " + nump + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("tab a")        << prefix + num + postfix        << prefix + nump + "   " + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("10 vs 2")      << prefix + num + postfix        << prefix + "10" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("diff len")     << prefix + num + postfix        << prefix + nump + postfix + "x" << int(Qt::CaseInsensitive) << i;
                QTest::newRow("01 before 1")  << prefix + "0" + num + postfix  << prefix + nump + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul nums 2nd") << prefix + "1-" + num + postfix << prefix + "1-" + nump + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul nums 2nd") << prefix + "10-" + num + postfix<< prefix + "10-10" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul nums 2nd") << prefix + "10-0"+ num + postfix<< prefix + "10-10" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul nums 2nd") << prefix + "10-" + num + postfix<< prefix + "10-010" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul nums big") << prefix + "10-" + num + postfix<< prefix + "20-0" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul nums big") << prefix + "2-" + num + postfix << prefix + "10-0" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul alphabet") << prefix + num + "-a" + postfix << prefix + num + "-c" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul alphabet2")<< prefix + num + "-a9" + postfix<< prefix + num + "-c0" + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("mul nums w\\0")<< prefix + num + "-"+ num + postfix<< prefix + num+"-0"+nump + postfix << int(Qt::CaseInsensitive) << i;
                QTest::newRow("num first")    << prefix + num + postfix  << prefix + "a" + postfix << int(Qt::CaseInsensitive) << i;
            }
        }
    }
}

void tst_QFileSystemModel::naturalCompare()
{
    QFETCH(QString, s1);
    QFETCH(QString, s2);
    QFETCH(int, caseSensitive);
    QFETCH(int, result);

    if (result == 10)
        QCOMPARE(QFileSystemModelPrivate::naturalCompare(s1, s2, Qt::CaseSensitivity(caseSensitive)), -1);
    else
        if (result == 11)
            QCOMPARE(QFileSystemModelPrivate::naturalCompare(s2, s1, Qt::CaseSensitivity(caseSensitive)), 1);
    else
        QCOMPARE(QFileSystemModelPrivate::naturalCompare(s2, s1, Qt::CaseSensitivity(caseSensitive)), result);
}

void tst_QFileSystemModel::readOnly()
{
    QModelIndex root = model->setRootPath(QDir::homePath());
    QTest::qWait(WAITTIME);
    QCOMPARE(model->isReadOnly(), true);
    QVERIFY(model->index(0, 0, root).isValid());
    QVERIFY(!(model->flags(model->index(0, 0, root)) & Qt::ItemIsEditable));
    model->setReadOnly(false);
    QCOMPARE(model->isReadOnly(), false);
    QVERIFY(model->flags(model->index(0, 0, root)) & Qt::ItemIsEditable);
}

void tst_QFileSystemModel::iconProvider()
{
    QVERIFY(model->iconProvider());
    QFileIconProvider *p = new QFileIconProvider();
    model->setIconProvider(p);
    QCOMPARE(model->iconProvider(), p);
}

bool tst_QFileSystemModel::createFiles(const QString &test_path, const QStringList &initial_files, const QStringList &initial_dirs, const QString &dir)
{
    QDir baseDir(dir);
    if (!baseDir.exists(test_path)) {
        if (!baseDir.mkdir(test_path) && false) {
            qDebug() << "failed to create dir" << test_path;
            return false;
        }
    }
    for (int i = 0; i < initial_dirs.count(); ++i) {
        QDir dir(test_path);
        dir.mkdir(initial_dirs.at(i));
    }
    for (int i = 0; i < initial_files.count(); ++i) {
        QFile file(test_path + QDir::separator() + initial_files.at(i));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            qDebug() << "failed to open file" << initial_files.at(i);
            return false;
        }
        if (!file.resize(1024 + file.size())) {
            qDebug() << "failed to resize file" << initial_files.at(i);
            return false;
        }
        if (!file.flush()) {
            qDebug() << "failed to flush file" << initial_files.at(i);
            return false;
        }
        file.close();
#ifdef Q_OS_WIN
        if (initial_files.at(i)[0] == '.')
            QProcess::execute(QString("attrib +h %1").arg(file.fileName()));
#endif
        //qDebug() << test_path + QDir::separator() + initial_files.at(i) << (QFile::exists(test_path + QDir::separator() + initial_files.at(i)));
    }

    return true;
}

void tst_QFileSystemModel::rowCount()
{
    QString tmp = QDir::temp().path() + QDir::separator() + QString("flatdirtest");
    QVERIFY(createFiles(tmp, QStringList()));

    QSignalSpy spy2(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy spy3(model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));

    QModelIndex root = model->setRootPath(tmp);
    QCOMPARE(model->rowCount(root), 0);
    QTest::qWait(WAITTIME);

    QCOMPARE(model->rowCount(root), 0);
    QString l = "b,d,f,h,j,.a,.c,.e,.g";
    QStringList files = l.split(",");
    QVERIFY(createFiles(tmp, files));
    QTest::qWait(WAITTIME);
    if (model->rowCount(root) != 5) {
        qDebug() << "dumping next" << model->rowCount(root);
        for (int i = 0; i < model->rowCount(root); ++i)
            qDebug() << model->index(i, 0, root).data().toString();
    }
    QCOMPARE(model->rowCount(root), 5);
    QVERIFY(spy2.count() > 0);
    QVERIFY(spy3.count() > 0);
}

void tst_QFileSystemModel::rowsInserted_data()
{
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("assending");
    for (int i = 0;i < 4;++i) {
        QTest::newRow(QString("Qt::AscendingOrder %1").arg(i).toLocal8Bit().constData())  << i << (int)Qt::AscendingOrder;
        QTest::newRow(QString("Qt::DescendingOrder %1").arg(i).toLocal8Bit().constData()) << i << (int)Qt::DescendingOrder;
    }
}

void tst_QFileSystemModel::rowsInserted()
{
    QString tmp = QDir::temp().path() + QDir::separator() + QString("flatdirtest");
    rowCount();
    QModelIndex root = model->index(model->rootPath());

    QFETCH(int, assending);
    QFETCH(int, count);
    model->sort(0, (Qt::SortOrder)assending);
    QTest::qWait(WAITTIME);

    QSignalSpy spy0(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy spy1(model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));
    int oldCount = model->rowCount(root);
    for (int i = 0; i < count; ++i)
        QVERIFY(createFiles(tmp, QStringList(QString("c%1").arg(i))));
    QCOMPARE(spy0.count(), 0);
    QTest::qWait(WAITTIME);
    QCOMPARE(model->rowCount(root), oldCount + count);
    if (assending == (Qt::SortOrder)Qt::AscendingOrder) {
        QString letter = model->index(model->rowCount(root) - 1, 0, root).data().toString();
        QCOMPARE(letter, QString("j"));
    } else {
        QCOMPARE(model->index(model->rowCount(root) - 1, 0, root).data().toString(), QString("b"));
    }
    if (spy0.count() > 0)
    if (count == 0) QCOMPARE(spy0.count(), 0); else QVERIFY(spy0.count() >= 1);
    if (count == 0) QCOMPARE(spy1.count(), 0); else QVERIFY(spy1.count() >= 1);

    QVERIFY(createFiles(tmp, QStringList(".hidden_file")));
    QTest::qWait(WAITTIME);

    if (count != 0) QVERIFY(spy0.count() >= 1); else QVERIFY(spy0.count() == 0);
    if (count != 0) QVERIFY(spy1.count() >= 1); else QVERIFY(spy1.count() == 0);
}

void tst_QFileSystemModel::rowsRemoved_data()
{
    rowsInserted_data();
}

void tst_QFileSystemModel::rowsRemoved()
{
    QString tmp = QDir::temp().path() + QDir::separator() + QString("flatdirtest");
    rowCount();
    QModelIndex root = model->index(model->rootPath());

    QFETCH(int, count);
    QFETCH(int, assending);
    model->sort(0, (Qt::SortOrder)assending);
    QTest::qWait(WAITTIME);

    QSignalSpy spy0(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy spy1(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    int oldCount = model->rowCount(root);
    for (int i = count - 1; i >= 0; --i) {
        qDebug() << "removing" <<  model->index(i, 0, root).data().toString();
        QVERIFY(QFile::remove(tmp + QDir::separator() + model->index(i, 0, root).data().toString()));
        // Workaround for QFileSystemWatcher issue #141001
        QTest::qWait(WAITTIME);
    }
    for (int i = 0 ; i < 10; ++i) {
        QTest::qWait(WAITTIME);
        qApp->processEvents();
        if (count != 0) QVERIFY(spy0.count() >= 1); else QVERIFY(spy0.count() == 0);
        if (count != 0) QVERIFY(spy1.count() >= 1); else QVERIFY(spy1.count() == 0);
        QStringList lst;
        for (int i = 0; i < model->rowCount(root); ++i)
            lst.append(model->index(i, 0, root).data().toString());
        if (model->rowCount(root) == oldCount - count)
            break;
        qDebug() << "still have:" << lst << QFile::exists(tmp + QDir::separator() + QString(".a"));
        QDir tmpLister(tmp);
        qDebug() << tmpLister.entryList();
    }
    QCOMPARE(model->rowCount(root), oldCount - count);

    QVERIFY(QFile::exists(tmp + QDir::separator() + QString(".a")));
    QVERIFY(QFile::remove(tmp + QDir::separator() + QString(".a")));
    QVERIFY(QFile::remove(tmp + QDir::separator() + QString(".c")));
    QTest::qWait(WAITTIME);

    if (count != 0) QVERIFY(spy0.count() >= 1); else QVERIFY(spy0.count() == 0);
    if (count != 0) QVERIFY(spy1.count() >= 1); else QVERIFY(spy1.count() == 0);
}

void tst_QFileSystemModel::dataChanged_data()
{
    rowsInserted_data();
}

void tst_QFileSystemModel::dataChanged()
{
    QString tmp = QDir::temp().path() + QDir::separator() + QString("flatdirtest");
    rowCount();
    QModelIndex root = model->index(model->rootPath());

    QFETCH(int, count);
    QFETCH(int, assending);
    model->sort(0, (Qt::SortOrder)assending);

    QSignalSpy spy(model, SIGNAL(dataChanged (const QModelIndex &, const QModelIndex &)));
    QStringList files;
    for (int i = 0; i < count; ++i)
        files.append(model->index(i, 0, root).data().toString());
    createFiles(tmp, files);

    QTest::qWait(WAITTIME);

    if (count != 0) QVERIFY(spy.count() >= 1); else QVERIFY(spy.count() == 0);
}

#ifdef Q_OS_UNIX
void tst_QFileSystemModel::filters_data()
{
    QTest::addColumn<QStringList>("files");
    QTest::addColumn<QStringList>("dirs");
    QTest::addColumn<int>("dirFilters");
    QTest::addColumn<QStringList>("nameFilters");
    QTest::addColumn<int>("rowCount");
    QTest::newRow("no dirs") << (QStringList() << "a" << "b" << "c") << QStringList() << (int)(QDir::Dirs) << QStringList() << 2;
    QTest::newRow("one dir - dotdot") << (QStringList() << "a" << "b" << "c") << (QStringList() << "Z") << (int)(QDir::Dirs | QDir::NoDotAndDotDot) << QStringList() << 1;
    QTest::newRow("one dir") << (QStringList() << "a" << "b" << "c") << (QStringList() << "Z") << (int)(QDir::Dirs) << QStringList() << 3;
    QTest::newRow("no dir + hidden") << (QStringList() << "a" << "b" << "c") << (QStringList()) << (int)(QDir::Dirs | QDir::Hidden) << QStringList() << 2;
    QTest::newRow("dir+hid+files") << (QStringList() << "a" << "b" << "c") << (QStringList()) <<
                         (int)(QDir::Dirs | QDir::Files | QDir::Hidden) << QStringList() << 5;
    QTest::newRow("dir+file+hid-dot .A") << (QStringList() << "a" << "b" << "c") << (QStringList() << ".A") <<
                         (int)(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot) << QStringList() << 4;
    QTest::newRow("dir+files+hid+dot A") << (QStringList() << "a" << "b" << "c") << (QStringList() << "AFolder") <<
                         (int)(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot) << (QStringList() << "A*") << 2;
    QTest::newRow("dir+files+hid+dot+cas1") << (QStringList() << "a" << "b" << "c") << (QStringList() << "Z") <<
                         (int)(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::CaseSensitive) << (QStringList() << "Z") << 1;
    QTest::newRow("dir+files+hid+dot+cas2") << (QStringList() << "a" << "b" << "c") << (QStringList() << "Z") <<
                         (int)(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::CaseSensitive) << (QStringList() << "a") << 1;
    QTest::newRow("dir+files+hid+dot+cas+alldir") << (QStringList() << "a" << "b" << "c") << (QStringList() << "Z") <<
                         (int)(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::CaseSensitive | QDir::AllDirs) << (QStringList() << "Z") << 1;
}

void tst_QFileSystemModel::filters()
{
    QString tmp = QDir::temp().path() + QDir::separator() + QString("flatdirtest");
    QVERIFY(createFiles(tmp, QStringList()));
    QModelIndex root = model->setRootPath(tmp);

    QFETCH(QStringList, files);
    QFETCH(QStringList, dirs);
    QFETCH(int, dirFilters);
    QFETCH(QStringList, nameFilters);
    QFETCH(int, rowCount);

    if (nameFilters.count() > 0)
        model->setNameFilters(nameFilters);
    model->setNameFilterDisables(false);
    model->setFilter((QDir::Filters)dirFilters);

    QVERIFY(createFiles(tmp, files, dirs));
    for (int i = 0; i < 10; ++i) {
        QTest::qWait(WAITTIME);
        if (model->rowCount(root) == rowCount)
            break;
    }
    QCOMPARE(model->rowCount(root), rowCount);

    // Make sure that we do what QDir does
    QDir xFactor(tmp);
    QDir::Filters  filters = (QDir::Filters)dirFilters;
    if (nameFilters.count() > 0)
        QCOMPARE(xFactor.entryList(nameFilters, filters).count(), rowCount);
    else
        QVERIFY(xFactor.entryList(filters).count() == rowCount);

    if (files.count() > 3 && rowCount >= 3) {
        QString fileName1 = (tmp + QDir::separator() + files.at(0));
        QString fileName2 = (tmp + QDir::separator() + files.at(1));
        QString fileName3 = (tmp + QDir::separator() + files.at(2));
        QFile::Permissions originalPermissions = QFile::permissions(fileName1);
        QVERIFY(QFile::setPermissions(fileName1, QFile::WriteOwner));
        QVERIFY(QFile::setPermissions(fileName2, QFile::ReadOwner));
        QVERIFY(QFile::setPermissions(fileName3, QFile::ExeOwner));
        model->setFilter((QDir::Readable));
        QTest::qWait(WAITTIME);
        QCOMPARE(model->rowCount(root), 1);

        model->setFilter((QDir::Writable));
        QTest::qWait(WAITTIME);
        QCOMPARE(model->rowCount(root), 1);

        model->setFilter((QDir::Executable));
        QTest::qWait(WAITTIME);
        QCOMPARE(model->rowCount(root), 1);

        // reset permissions
        QVERIFY(QFile::setPermissions(fileName1, originalPermissions));
        QVERIFY(QFile::setPermissions(fileName2, originalPermissions));
        QVERIFY(QFile::setPermissions(fileName3, originalPermissions));
    }
}
#endif

void tst_QFileSystemModel::nameFilters()
{
    QStringList list;
    list << "a" << "b" << "c";
    model->setNameFilters(list);
    model->setNameFilterDisables(false);
    QCOMPARE(model->nameFilters(), list);

    QString tmp = QDir::temp().path() + QDir::separator() + QString("flatdirtest");
    QVERIFY(createFiles(tmp, list));
    QModelIndex root = model->setRootPath(tmp);
    QTest::qWait(WAITTIME);
    QCOMPARE(model->rowCount(root), 3);

    QStringList filters;
    filters << "a" << "b";
    model->setNameFilters(filters);
    QTest::qWait(WAITTIME);

    QCOMPARE(model->rowCount(root), 2);
}


QTEST_MAIN(tst_QFileSystemModel)
#include "tst_qfilesystemmodel.moc"


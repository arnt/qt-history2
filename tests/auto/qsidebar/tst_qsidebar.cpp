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
#include "../../../src/gui/dialogs/qsidebar_p.h"
#include "../../../src/gui/dialogs/qfilesystemmodel_p.h"

//TESTED_CLASS=QSidebar
//TESTED_FILES=qsidebar.h qsidebar.cpp

class tst_QSidebar : public QObject {
  Q_OBJECT

public:
    tst_QSidebar();
    virtual ~tst_QSidebar();

public Q_SLOTS:
    void init();
    void cleanup();

private slots:
    void setUrls();
    void selectUrls();
    void addUrls();

    void goToUrl();
};

tst_QSidebar::tst_QSidebar()
{
}

tst_QSidebar::~tst_QSidebar()
{
}

void tst_QSidebar::init()
{
}

void tst_QSidebar::cleanup()
{
}

void tst_QSidebar::setUrls()
{
    QList<QUrl> urls;
    QFileSystemModel fsmodel;
    QSidebar qsidebar(&fsmodel, urls);
    QAbstractItemModel *model = qsidebar.model();

    urls << QUrl::fromLocalFile(QDir::rootPath())
         << QUrl::fromLocalFile(QDir::temp().absolutePath());

    QCOMPARE(model->rowCount(), 0);
    qsidebar.setUrls(urls);
    QCOMPARE(qsidebar.urls(), urls);
    QCOMPARE(model->rowCount(), urls.count());
    qsidebar.setUrls(urls);
    QCOMPARE(model->rowCount(), urls.count());
}

void tst_QSidebar::selectUrls()
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QDir::rootPath())
         << QUrl::fromLocalFile(QDir::temp().absolutePath());
    QFileSystemModel fsmodel;
    QSidebar qsidebar(&fsmodel, urls);

    QSignalSpy spy(&qsidebar, SIGNAL(goToUrl(const QUrl &)));
    qsidebar.selectUrl(urls.at(0));
    QCOMPARE(spy.count(), 0);
}

void tst_QSidebar::addUrls()
{
    QList<QUrl> emptyUrls;
    QFileSystemModel fsmodel;
    QSidebar qsidebar(&fsmodel, emptyUrls);
    QAbstractItemModel *model = qsidebar.model();

    // default
    QCOMPARE(model->rowCount(), 0);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QDir::rootPath())
         << QUrl::fromLocalFile(QDir::temp().absolutePath());
    qsidebar.addUrls(urls, -1);
    QCOMPARE(model->rowCount(), 2);

    qsidebar.setUrls(emptyUrls);
    qsidebar.addUrls(urls, 0);
    QCOMPARE(model->rowCount(), 2);

    qsidebar.setUrls(emptyUrls);
    qsidebar.addUrls(urls, 100);
    QCOMPARE(model->rowCount(), 2);

    QList<QUrl> moreUrls;
    moreUrls << QUrl::fromLocalFile(QDir::home().absolutePath());
    qsidebar.addUrls(moreUrls, -1);
    QCOMPARE(model->rowCount(), 3);

    QList<QUrl> badUrls;
    badUrls << QUrl::fromLocalFile(QDir::home().absolutePath() + "/I used to exist");
    qsidebar.addUrls(badUrls, 0);
    QCOMPARE(model->rowCount(), 4);
}

void tst_QSidebar::goToUrl()
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QDir::rootPath())
         << QUrl::fromLocalFile(QDir::temp().absolutePath());
    QFileSystemModel fsmodel;
    QSidebar qsidebar(&fsmodel, urls);
    qsidebar.show();

    QSignalSpy spy(&qsidebar, SIGNAL(goToUrl(const QUrl &)));
    QTest::mousePress(qsidebar.viewport(), Qt::LeftButton, 0, qsidebar.visualItemRect(qsidebar.item(0)).center());
    QCOMPARE(spy.count(), 1);
    QCOMPARE((spy.value(0)).at(0).toUrl(), urls.first());
}

QTEST_MAIN(tst_QSidebar)
#include "tst_qsidebar.moc"


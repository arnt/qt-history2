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
#if QT_VERSION < 0x040200
QTEST_NOOP_MAIN
#else
#include <QStandardItemModel>
#include <qdebug.h>
#include <qdesktopservices.h>

//TESTED_CLASS=QDesktopServices
//TESTED_FILES=gui/util/qdesktopservices.h gui/util/qdesktopservices.cpp

class tst_qdesktopservices : public QObject {
  Q_OBJECT

public:
    tst_qdesktopservices();
    virtual ~tst_qdesktopservices();

private slots:
    void init();
    void cleanup();
    void openUrl();
    void handlers();

protected:
};

tst_qdesktopservices::tst_qdesktopservices()
{
}

tst_qdesktopservices::~tst_qdesktopservices()
{
}

void tst_qdesktopservices::init()
{
}

void tst_qdesktopservices::cleanup()
{
}

void tst_qdesktopservices::openUrl()
{
    // At the bare minimum check that they return false for invalid url's
    QCOMPARE(QDesktopServices::openUrl(QUrl()), false);
}

class MyUrlHandler : public QObject
{
    Q_OBJECT
public:
    QUrl lastHandledUrl;

public slots:
    inline void handle(const QUrl &url) {
        lastHandledUrl = url;
    }
};

void tst_qdesktopservices::handlers()
{
    MyUrlHandler fooHandler;
    MyUrlHandler barHandler;

    QDesktopServices::setUrlHandler(QString("foo"), &fooHandler, "handle");
    QDesktopServices::setUrlHandler(QString("bar"), &barHandler, "handle");

    QUrl fooUrl("foo://blub/meh");
    QUrl barUrl("bar://hmm/hmmmm");

    QVERIFY(QDesktopServices::openUrl(fooUrl));
    QVERIFY(QDesktopServices::openUrl(barUrl));

    QCOMPARE(fooHandler.lastHandledUrl.toString(), fooUrl.toString());
    QCOMPARE(barHandler.lastHandledUrl.toString(), barUrl.toString());
}

QTEST_MAIN(tst_qdesktopservices)
#include "tst_qdesktopservices.moc"
#endif


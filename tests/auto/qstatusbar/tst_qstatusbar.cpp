/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qstatusbar.h>
#include <QLabel>


//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qstatusbar.h gui/widgets/qstatusbar.cpp

class tst_QStatusBar: public QObject
{
    Q_OBJECT

public:
    tst_QStatusBar();
    virtual ~tst_QStatusBar();


protected slots:
    void messageChanged(const QString&);

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void tempMessage();
    void insertWidget();
    void insertPermanentWidget();

private:
    QStatusBar *testWidget;
    QString currentMessage;
};

tst_QStatusBar::tst_QStatusBar()
{
}

tst_QStatusBar::~tst_QStatusBar()
{
}

void tst_QStatusBar::init()
{
    testWidget = new QStatusBar;
    connect(testWidget, SIGNAL(messageChanged(QString)), this, SLOT(messageChanged(QString)));

    QWidget *item1 = new QWidget(testWidget);
    testWidget->addWidget(item1);
}

void tst_QStatusBar::cleanup()
{
    delete testWidget;
}

void tst_QStatusBar::initTestCase()
{
}

void tst_QStatusBar::cleanupTestCase()
{
}

void tst_QStatusBar::messageChanged(const QString &m)
{
    currentMessage = m;
}

void tst_QStatusBar::tempMessage()
{
    QVERIFY(testWidget->currentMessage().isNull());
    QVERIFY(currentMessage.isNull());

    testWidget->showMessage("Ready", 500);
    QCOMPARE(testWidget->currentMessage(), QString("Ready"));
    QCOMPARE(testWidget->currentMessage(), currentMessage);

    QTest::qWait(1000);

    QVERIFY(testWidget->currentMessage().isNull());
    QVERIFY(currentMessage.isNull());

    testWidget->showMessage("Ready again", 500);
    QCOMPARE(testWidget->currentMessage(), QString("Ready again"));
    QCOMPARE(testWidget->currentMessage(), currentMessage);

    testWidget->clearMessage();
    QVERIFY(testWidget->currentMessage().isNull());
    QVERIFY(currentMessage.isNull());
}

void tst_QStatusBar::insertWidget()
{
    QStatusBar sb;
    sb.addPermanentWidget(new QLabel("foo"));
    QTest::ignoreMessage(QtWarningMsg, "QStatusBar::insertWidget: Index out of range (-1), appending widget");
    QCOMPARE(sb.insertWidget(-1, new QLabel("foo")), 0);
    QTest::ignoreMessage(QtWarningMsg, "QStatusBar::insertWidget: Index out of range (2), appending widget");
    QCOMPARE(sb.insertWidget(2, new QLabel("foo")), 1);
    QCOMPARE(sb.insertWidget(0, new QLabel("foo")), 0);
    QCOMPARE(sb.insertWidget(3, new QLabel("foo")), 3);
}

void tst_QStatusBar::insertPermanentWidget()
{
    QStatusBar sb;
    sb.addWidget(new QLabel("foo"));
    QTest::ignoreMessage(QtWarningMsg, "QStatusBar::insertPermanentWidget: Index out of range (-1), appending widget");
    QCOMPARE(sb.insertPermanentWidget(-1, new QLabel("foo")), 1);
    QTest::ignoreMessage(QtWarningMsg, "QStatusBar::insertPermanentWidget: Index out of range (0), appending widget");
    QCOMPARE(sb.insertPermanentWidget(0, new QLabel("foo")), 2);
    QCOMPARE(sb.insertPermanentWidget(2, new QLabel("foo")), 2);
    QTest::ignoreMessage(QtWarningMsg, "QStatusBar::insertPermanentWidget: Index out of range (5), appending widget");
    QCOMPARE(sb.insertPermanentWidget(5, new QLabel("foo")), 4);
    QCOMPARE(sb.insertWidget(1, new QLabel("foo")), 1);
    QTest::ignoreMessage(QtWarningMsg, "QStatusBar::insertPermanentWidget: Index out of range (1), appending widget");
    QCOMPARE(sb.insertPermanentWidget(1, new QLabel("foo")), 6);
}


QTEST_MAIN(tst_QStatusBar)
#include "tst_qstatusbar.moc"

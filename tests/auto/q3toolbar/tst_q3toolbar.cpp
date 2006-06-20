/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <q3toolbar.h>
#include <qaction.h>
#include <qapplication.h>
#include <QToolButton>
//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3toolbar.h compat/widgets/q3toolbar.cpp

#if defined Q_CC_MSVC && _MSC_VER <= 1200
#define NOFINDCHILDRENMETHOD
#endif

class tst_Q3ToolBar : public QObject
{
    Q_OBJECT

public:
    tst_Q3ToolBar();
    virtual ~tst_Q3ToolBar();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void toggled();

private:
    Q3ToolBar* testWidget;
};

tst_Q3ToolBar::tst_Q3ToolBar()
{
}

tst_Q3ToolBar::~tst_Q3ToolBar()
{

}

void tst_Q3ToolBar::initTestCase()
{
    testWidget = new Q3ToolBar(0, "testWidget");
    testWidget->show();
    qApp->setMainWidget(testWidget);

    QTest::qWait(100);
}

void tst_Q3ToolBar::cleanupTestCase()
{
    delete testWidget;
}

void tst_Q3ToolBar::init()
{
}

void tst_Q3ToolBar::cleanup()
{
}

void tst_Q3ToolBar::toggled()
{
    // When clicking on a toggled action it should emit a signal
    QAction *action = new QAction( this, "action" );
    action->setToggleAction( true );
    action->addTo(testWidget);
    testWidget->show();
    QSignalSpy spy(action, SIGNAL(toggled(bool)));
#ifndef NOFINDCHILDRENMETHOD
    QList<QToolButton *> list = testWidget->findChildren<QToolButton *>();
#else
    QList<QToolButton *> list = qFindChildren<QToolButton *>(testWidget, QString());

#endif
    for (int i = 0; i < list.size(); ++i)
        QTest::mouseClick(list.at(i), Qt::LeftButton);
    QCOMPARE(spy.count(), 1);

    // Also try the othe case (a toggled action will emit the toolbuttons toggled)
    QSignalSpy spy2(list.at(1), SIGNAL(toggled(bool)));
    action->setChecked(!action->isChecked());
    QCOMPARE(spy2.count(), 1);

}


QTEST_MAIN(tst_Q3ToolBar)
#include "tst_q3toolbar.moc"

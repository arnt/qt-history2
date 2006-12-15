/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qapplication.h>
#include <qevent.h>
#include <qaction.h>
#include <qmenu.h>

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qaction.h gui/kernel/qaction.cpp

class tst_QAction : public QObject
{
    Q_OBJECT

public:
    tst_QAction();
    virtual ~tst_QAction();


    void updateState(QActionEvent *e);

public slots:
    void initTestCase();
    void cleanupTestCase();
private slots:
    void getSetCheck();
    void setText_data();
    void setText();
    void setIconText_data() { setText_data(); }
    void setIconText();
    void actionEvent();
    void setStandardKeys();
    void alternateShortcuts();

private:
    int m_lastEventType;
    QAction *m_lastAction;
    QWidget *m_tstWidget;
};

// Testing get/set functions
void tst_QAction::getSetCheck()
{
    QAction obj1(0);
    // QActionGroup * QAction::actionGroup()
    // void QAction::setActionGroup(QActionGroup *)
    QActionGroup *var1 = new QActionGroup(0);
    obj1.setActionGroup(var1);
    QCOMPARE(var1, obj1.actionGroup());
    obj1.setActionGroup((QActionGroup *)0);
    QCOMPARE((QActionGroup *)0, obj1.actionGroup());
    delete var1;

    // QMenu * QAction::menu()
    // void QAction::setMenu(QMenu *)
    QMenu *var2 = new QMenu(0);
    obj1.setMenu(var2);
    QCOMPARE(var2, obj1.menu());
    obj1.setMenu((QMenu *)0);
    QCOMPARE((QMenu *)0, obj1.menu());
    delete var2;
}

class MyWidget : public QWidget
{
public:
    MyWidget(tst_QAction *tst, QWidget *parent=0) : QWidget(parent) { this->tst = tst; }

protected:
    virtual void actionEvent(QActionEvent *e) { tst->updateState(e); }

private:
    tst_QAction *tst;
};

tst_QAction::tst_QAction()
{
}

tst_QAction::~tst_QAction()
{

}

void tst_QAction::initTestCase()
{
    m_lastEventType = 0;
    m_lastAction = 0;

    MyWidget *mw = new MyWidget(this);
    m_tstWidget = mw;
    mw->show();

    qApp->processEvents();
}

void tst_QAction::cleanupTestCase()
{
    QWidget *testWidget = m_tstWidget;
    if ( testWidget ) {
        testWidget->hide();
        delete testWidget;
    }
}

void tst_QAction::setText_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("iconText");
    QTest::addColumn<QString>("textFromIconText");

    //next we fill it with data
    QTest::newRow("Normal") << "Action" << "Action" << "Action";
    QTest::newRow("Ampersand") << "Search && Destroy" << "Search & Destroy" << "Search && Destroy";
    QTest::newRow("Mnemonic and ellipsis") << "O&pen File ..." << "Open File" << "Open File";
}

void tst_QAction::setText()
{
    QFETCH(QString, text);

    QAction action(0);
    action.setText(text);

    QCOMPARE(action.text(), text);

    QFETCH(QString, iconText);
    QCOMPARE(action.iconText(), iconText);
}

void tst_QAction::setIconText()
{
    QFETCH(QString, iconText);

    QAction action(0);
    action.setIconText(iconText);
    QCOMPARE(action.iconText(), iconText);

    QFETCH(QString, textFromIconText);
    QCOMPARE(action.text(), textFromIconText);
}


void tst_QAction::updateState(QActionEvent *e)
{
    if (!e) {
        m_lastEventType = 0;
	m_lastAction = 0;
    } else {
        m_lastEventType = (int)e->type();
        m_lastAction = e->action();
    }
}

void tst_QAction::actionEvent()
{
    QAction a(0);
    a.setText("action text");

    // add action
    m_tstWidget->addAction(&a);
    qApp->processEvents();

    QCOMPARE(m_lastEventType, (int)QEvent::ActionAdded);
    QCOMPARE(m_lastAction, &a);

    // change action
    a.setText("new action text");
    qApp->processEvents();

    QCOMPARE(m_lastEventType, (int)QEvent::ActionChanged);
    QCOMPARE(m_lastAction, &a);

    // remove action
    m_tstWidget->removeAction(&a);
    qApp->processEvents();

    QCOMPARE(m_lastEventType, (int)QEvent::ActionRemoved);
    QCOMPARE(m_lastAction, &a);
}

//basic testing of standard keys
void tst_QAction::setStandardKeys()
{
    QAction act(0);
    act.setShortcut(QKeySequence("CTRL+L"));
    QList<QKeySequence> list;
    act.setShortcuts(list);
    act.setShortcuts(QKeySequence::Copy);
    QVERIFY(act.shortcut() == act.shortcuts().first());

    QList<QKeySequence> expected;
#ifdef Q_WS_MAC
    expected  << QKeySequence("CTRL+C");
#elif defined(Q_WS_WIN)
    expected  << QKeySequence("CTRL+C") << QKeySequence("CTRL+INSERT");
#else
    expected  << QKeySequence("CTRL+C") << QKeySequence("F16") << QKeySequence("CTRL+INSERT");
#endif
    QVERIFY(act.shortcuts() == expected);
}


void tst_QAction::alternateShortcuts()
{
    //test the alternate shortcuts (by adding more than 1 shortcut)

    QWidget *wid=m_tstWidget;

    {
        QAction act(wid);
        wid->addAction(&act);
        QList<QKeySequence> shlist= QList<QKeySequence>() << QKeySequence("CTRL+P") <<QKeySequence("CTRL+A");
        act.setShortcuts(shlist);

        QSignalSpy spy(&act, SIGNAL(triggered()));

        act.setAutoRepeat(true);
        QTest::keyClick(wid, Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(spy.count(), 1); //act should have been triggered

        act.setAutoRepeat(false);
        QTest::keyClick(wid, Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(spy.count(), 2); //act should have been triggered a 2nd time

        //end of the scope of the action, it will be destroyed and removed from wid
        //This action should also unregister its shortcuts
    }


    //this tests a crash (if the action did not unregister its alternate shortcuts)
    QTest::keyClick(wid, Qt::Key_A, Qt::ControlModifier);
}

QTEST_MAIN(tst_QAction)
#include "tst_qaction.moc"

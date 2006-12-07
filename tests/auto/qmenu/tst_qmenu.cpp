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
#include <QPushButton>
#include <QMainWindow>
#include <QMenuBar>

#include <qmenu.h>
#include <qstyle.h>
#include <qdebug.h>
//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qmenu.h gui/widgets/qmenu.cpp

class tst_QMenu : public QObject
{
    Q_OBJECT

public:
    tst_QMenu();
    virtual ~tst_QMenu();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void addActionsAndClear();

    void keyboardNavigation_data();
    void keyboardNavigation();
    void focus();
	void overrideMenuAction();

#if defined(QT3_SUPPORT)
    void indexBasedInsertion_data();
    void indexBasedInsertion();
#endif

protected slots:
    void onActivated(QAction*);
    void onHighlighted(QAction*);
private:
    void createActions();
    QMenu *menus[2], *lastMenu;
    enum { num_builtins = 10 };
    QAction *activated, *highlighted, *builtins[num_builtins];
};

// Testing get/set functions
void tst_QMenu::getSetCheck()
{
    QMenu obj1;
    // QAction * QMenu::defaultAction()
    // void QMenu::setDefaultAction(QAction *)
    QAction *var1 = new QAction(0);
    obj1.setDefaultAction(var1);
    QCOMPARE(var1, obj1.defaultAction());
    obj1.setDefaultAction((QAction *)0);
    QCOMPARE((QAction *)0, obj1.defaultAction());
    delete var1;

    // QAction * QMenu::activeAction()
    // void QMenu::setActiveAction(QAction *)
    QAction *var2 = new QAction(0);
    obj1.setActiveAction(var2);
    QCOMPARE(var2, obj1.activeAction());
    obj1.setActiveAction((QAction *)0);
    QCOMPARE((QAction *)0, obj1.activeAction());
    delete var2;
}

tst_QMenu::tst_QMenu()
{
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);
}

tst_QMenu::~tst_QMenu()
{

}

void
tst_QMenu::initTestCase()
{
    for(int i = 0; i < num_builtins; i++)
        builtins[i] = 0;
    for(int i = 0; i < 2; i++) {
        menus[i] = new QMenu;
        QObject::connect(menus[i], SIGNAL(triggered(QAction*)), this, SLOT(onActivated(QAction*)));
        QObject::connect(menus[i], SIGNAL(hovered(QAction*)), this, SLOT(onHighlighted(QAction*)));
    }
}

void
tst_QMenu::cleanupTestCase()
{
    for(int i = 0; i < 2; i++)
        menus[i]->clear();
    for(int i = 0; i < num_builtins; i++) {
        bool menuAction = false;
        for (int j = 0; j < 2; ++j)
            if (menus[j]->menuAction() == builtins[i])
                menuAction = true;
        if (!menuAction)
            delete builtins[i];
    }
    delete menus[0];
    delete menus[1];
}

void
tst_QMenu::init()
{
    activated = highlighted = 0;
    lastMenu = 0;
}

void
tst_QMenu::cleanup()
{
}

void
tst_QMenu::createActions()
{
    if(!builtins[0])
        builtins[0] = new QAction("New", 0);
    menus[0]->addAction(builtins[0]);

    if(!builtins[1]) {
        builtins[1] = new QAction(0);
        builtins[1]->setSeparator(true);
    }
    menus[0]->addAction(builtins[1]);

    if(!builtins[2]) {
        builtins[2] = menus[1]->menuAction();
        builtins[2]->setText("&Open..");
        builtins[8] = new QAction("Close", 0);
        menus[1]->addAction(builtins[8]);
        builtins[9] = new QAction("Quit", 0);
        menus[1]->addAction(builtins[9]);
    }
    menus[0]->addAction(builtins[2]);

    if(!builtins[3])
        builtins[3] = new QAction("Open &as..", 0);
    menus[0]->addAction(builtins[3]);

    if(!builtins[4]) {
        builtins[4] = new QAction("Save", 0);
        builtins[4]->setEnabled(false);
    }
    menus[0]->addAction(builtins[4]);

    if(!builtins[5])
        builtins[5] = new QAction("Sa&ve as..", 0);
    menus[0]->addAction(builtins[5]);

    if(!builtins[6]) {
        builtins[6] = new QAction(0);
        builtins[6]->setSeparator(true);
    }
    menus[0]->addAction(builtins[6]);

    if(!builtins[7])
        builtins[7] = new QAction("Prin&t", 0);
    menus[0]->addAction(builtins[7]);
}

void
tst_QMenu::onHighlighted(QAction *action)
{
    highlighted = action;
    lastMenu = qobject_cast<QMenu*>(sender());
}

void
tst_QMenu::onActivated(QAction *action)
{
    activated = action;
    lastMenu = qobject_cast<QMenu*>(sender());
}

//actual tests
void
tst_QMenu::addActionsAndClear()
{
    QCOMPARE(menus[0]->actions().count(), 0);
    createActions();
    QCOMPARE(menus[0]->actions().count(), 8);
    menus[0]->clear();
    QCOMPARE(menus[0]->actions().count(), 0);
}

void
tst_QMenu::keyboardNavigation_data()
{
    QTest::addColumn<int>("key");
    QTest::addColumn<int>("expected_action");
    QTest::addColumn<int>("expected_menu");
    QTest::addColumn<bool>("init");
    QTest::addColumn<bool>("expected_activated");
    QTest::addColumn<bool>("expected_highlighted");

    //test up and down (order is important here)
    QTest::newRow("data0") << int(Qt::Key_Down) << 0 << 0 << true << false << true;
    QTest::newRow("data1") << int(Qt::Key_Down) << 2 << 0 << false << false << true; //skips the separator
    QTest::newRow("data2") << int(Qt::Key_Down) << 3 << 0 << false << false << true;

    if (QApplication::style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled))
        QTest::newRow("data3_noMac") << int(Qt::Key_Down) << 4 << 0 << false << false << true;
    else
        QTest::newRow("data3_Mac") << int(Qt::Key_Down) << 5 << 0 << false << false << true;
    QTest::newRow("data4") << int(Qt::Key_Up) << 3 << 0 << false << false << true;
    QTest::newRow("data5") << int(Qt::Key_Up) << 2 << 0 << false << false << true;
    QTest::newRow("data6") << int(Qt::Key_Right) << 8 << 1 << false << false << true;
    QTest::newRow("data7") << int(Qt::Key_Down) << 9 << 1 << false << false << true;
    QTest::newRow("data8") << int(Qt::Key_Escape) << 2 << 0 << false << false << false;
    QTest::newRow("data9") << int(Qt::Key_Down) << 3 << 0 << false << false<< true;
    QTest::newRow("data10") << int(Qt::Key_Return) << 3 << 0 << false << true << false;

    //test shortcuts
#if 0
    QTest::newRow("shortcut0") << (Qt::ALT | Qt::Key_A) << 2 << 0 << true << true << false;
#endif
}

void
tst_QMenu::keyboardNavigation()
{
    DEPENDS_ON( "addActionsAndClear" ); //if add/clear fails...
    QFETCH(int, key);
    QFETCH(int, expected_action);
    QFETCH(int, expected_menu);
    QFETCH(bool, init);
    QFETCH(bool, expected_activated);
    QFETCH(bool, expected_highlighted);

    if(init) {
        lastMenu = menus[0];
        lastMenu->clear();
        createActions();
        lastMenu->popup(QPoint(0, 0));
    }

    QTest::keyClick(lastMenu, (Qt::Key)key);
    if(expected_activated) {
        QCOMPARE(activated, builtins[expected_action]);
        QCOMPARE(menus[expected_menu]->activeAction(), (QAction *)0);
    } else {
        QCOMPARE(menus[expected_menu]->activeAction(), builtins[expected_action]);
        if(expected_highlighted)
            QCOMPARE(menus[expected_menu]->activeAction(), highlighted);
    }
}


void tst_QMenu::focus()
{
    QMenu menu;
    menu.addAction("One");
    menu.addAction("Two");
    menu.addAction("Three");
    bool fullKeyboardControl = true;

#ifdef Q_WS_MAC
    extern bool qt_tab_all_widgets; // qapplication_mac.cpp
    fullKeyboardControl = qt_tab_all_widgets;
#endif

    if (!fullKeyboardControl)
        QSKIP("Computer is currently set up to NOT tab to all widgets,"
             " this test assumes you can tab to all widgets", SkipAll);

    QWidget window;
    QPushButton button("Push me", &window);
    window.show();
    qApp->setActiveWindow(&window);

    QVERIFY(button.hasFocus());
    QCOMPARE(QApplication::focusWidget(), (QWidget *)&button);
    QCOMPARE(QApplication::activeWindow(), &window);
    menu.show();
#if 0
    QVERIFY(!button.hasFocus());
    QCOMPARE(QApplication::focusWidget(), &menu);
    QCOMPARE(QApplication::activeWindow(), &window);
#else
    QVERIFY(button.hasFocus());
    QCOMPARE(QApplication::focusWidget(), (QWidget *)&button);
    QCOMPARE(QApplication::activeWindow(), &window);
#endif
    menu.hide();
    QVERIFY(button.hasFocus());
    QCOMPARE(QApplication::focusWidget(), (QWidget *)&button);
    QCOMPARE(QApplication::activeWindow(), &window);
}

#if defined(QT3_SUPPORT)
void tst_QMenu::indexBasedInsertion_data()
{
    QTest::addColumn<int>("indexForInsertion");
    QTest::addColumn<int>("expectedIndex");

    QTest::newRow("negative-index-appends") << -1 << 1;
    QTest::newRow("prepend") << 0 << 0;
    QTest::newRow("append") << 1 << 1;
}

void tst_QMenu::overrideMenuAction()
{
	//test the override menu action by first creating an action to which we set its menu
	QMainWindow w;

	QAction *aFileMenu = new QAction("&File", &w);
	w.menuBar()->addAction(aFileMenu);

	QMenu *m = new QMenu(&w);
	QAction *menuaction = m->menuAction();
	connect(m, SIGNAL(triggered(QAction*)), SLOT(onActivated(QAction*)));
	aFileMenu->setMenu(m); //this sets the override menu action for the QMenu
    QCOMPARE(m->menuAction(), aFileMenu);

	QAction *aQuit = new QAction("Quit", &w);
	aQuit->setShortcut(QKeySequence("Ctrl+X"));
	m->addAction(aQuit);

	w.show();

	//test of the action inside the menu
	QTest::keyClick(&w, Qt::Key_X, Qt::ControlModifier);
    QCOMPARE(aQuit, activated);

	//test if the menu still pops out
	QTest::keyClick(&w, Qt::Key_F, Qt::AltModifier);
    QVERIFY(m->isVisible());

	delete aFileMenu;

	//after the deletion of the override menu action, 
	//the menu should have its default menu action back
	QCOMPARE(m->menuAction(), menuaction);

}

void tst_QMenu::indexBasedInsertion()
{
    // test the compat'ed index based insertion

    QFETCH(int, indexForInsertion);
    QFETCH(int, expectedIndex);

    {
        QMenu menu;
        menu.addAction("Regular Item");

        menu.insertItem("New Item", -1 /*id*/, indexForInsertion);

        QAction *act = menu.actions().value(expectedIndex);
        QVERIFY(act);
        QCOMPARE(act->text(), QString("New Item"));
    }
    {
        QMenu menu;
        menu.addAction("Regular Item");

        menu.insertSeparator(indexForInsertion);

        QAction *act = menu.actions().value(expectedIndex);
        QVERIFY(act);
        QVERIFY(act->isSeparator());
    }
}

#endif

QTEST_MAIN(tst_QMenu)
#include "tst_qmenu.moc"

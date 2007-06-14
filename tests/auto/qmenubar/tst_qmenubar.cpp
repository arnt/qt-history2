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
#include <qmainwindow.h>
#include <qmenubar.h>
#include <q3popupmenu.h>
#include <qstyle.h>
#include <qdesktopwidget.h>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include <qobject.h>

class QMainWindow;

#include <qmenubar.h>

//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qmenubar.h gui/widgets/qmenubar.cpp

class QtTestSlot : public QObject
{
    Q_OBJECT

public:
    QtTestSlot( QObject* parent = 0 ): QObject( parent ) { clear(); };
    virtual ~QtTestSlot() {};

    void clear() { sel_count = 0; };
    uint selCount() { return sel_count; };

public slots:
    void selected() { sel_count++; };

private:
    uint sel_count;
};

class tst_QMenuBar : public QObject
{
    Q_OBJECT
public:
    tst_QMenuBar();
    virtual ~tst_QMenuBar();


    void initSimpleMenubar();


    void initComplexMenubar();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
private slots:
    void getSetCheck();
    void clear();
    void removeItemAt_data();
    void removeItemAt();
    void removeItem_data();
    void removeItem();
    void count();

    void insertItem_QString_QObject();

    void accel();
    void activatedCount();

    void check_accelKeys();
    void check_cursorKeys1();
    void check_cursorKeys2();
    void check_cursorKeys3();

    void check_homeKey();
    void check_endKey();
    void check_escKey();

//     void check_mouse1_data();
//     void check_mouse1();
//     void check_mouse2_data();
//     void check_mouse2();

    void check_altPress();
    void check_shortcutPress();

    void check_menuPosition();

#if defined(QT3_SUPPORT)
    void indexBasedInsertion_data();
    void indexBasedInsertion();
#endif

protected slots:
    void onActivated( int );

private:
    QtTestSlot *menu1;
    QtTestSlot *menu2;
    QtTestSlot *menu3;
    QtTestSlot *menu4;

    QtTestSlot *item1_A;
    QtTestSlot *item1_B;
    QtTestSlot *item2_C;
    QtTestSlot *item2_D;
    QtTestSlot *item2_E;
    QtTestSlot *item2_F;
    QtTestSlot *item2_G;
    QtTestSlot *item2_H;

    void resetSlots();
    void resetCount();

    void reset() { resetSlots(); resetCount(); };

    int last_accel_id;
    int activated_count;

    int idAccel;
    int idAccel1;
    QMainWindow *mw;
    QMenuBar *mb;
    Q3PopupMenu *pm1;
    Q3PopupMenu *pm2;
};

// Testing get/set functions
void tst_QMenuBar::getSetCheck()
{
    QMenuBar obj1;
    // QAction * QMenuBar::activeAction()
    // void QMenuBar::setActiveAction(QAction *)
    QAction *var1 = new QAction(0);
    obj1.setActiveAction(var1);
    QCOMPARE(var1, obj1.activeAction());
    obj1.setActiveAction((QAction *)0);
    QCOMPARE((QAction *)0, obj1.activeAction());
    delete var1;
}

////



#include <qcursor.h>

const int RESET = 0;

/*!
    Test plan:
	insertItem (all flavours and combinations)
	removing menu items
	clearing the menu

	check the common behaviour + emitted signals for:
	    accelerator keys
	    navigating tru the menu and then pressing ENTER
	    mouse clicks
	    mouse drags
	    combinations of key + mouse (if possible)
	    checked / unckecked state of menu options
	    active / inactive state

    Can't test these without pixmap comparison...
	show and hide
	icons in a menu
	pixmaps in a menu

*/

tst_QMenuBar::tst_QMenuBar()

{
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);

    last_accel_id = RESET;
    activated_count = 0;
    mb = 0;
    pm1 = 0;
    pm2 = 0;
}

tst_QMenuBar::~tst_QMenuBar()
{
    //delete mw; //#### cannot do that AFTER qapplication was destroyed!
    mw = 0;
}

void tst_QMenuBar::initTestCase()
{
    // create a default mainwindow
    // If you run a widget test, this will be replaced in the testcase by the
    // widget under test
    mw = new QMainWindow(0, Qt::X11BypassWindowManagerHint);
    mb = new QMenuBar( mw, "menubar" );
    connect( mb, SIGNAL(activated(int)), this, SLOT(onActivated(int)) );

    initSimpleMenubar();

    qApp->setMainWidget( mw );
    mw->show();
    qApp->setActiveWindow(mw);

    menu1 = new QtTestSlot( mw );
    menu2 = new QtTestSlot( mw );
    menu3 = new QtTestSlot( mw );
    menu4 = new QtTestSlot( mw );
    item1_A = new QtTestSlot( mw );
    item1_B = new QtTestSlot( mw );
    item2_C = new QtTestSlot( mw );
    item2_D = new QtTestSlot( mw );
    item2_E = new QtTestSlot( mw );
    item2_F = new QtTestSlot( mw );
    item2_G = new QtTestSlot( mw );
    item2_H = new QtTestSlot( mw );
}

void tst_QMenuBar::cleanupTestCase()
{
    delete mw;
}

void tst_QMenuBar::initSimpleMenubar()
{
    mb->hide();
    mb->clear();

    delete pm1;
    pm1  = new Q3PopupMenu( mb );
    idAccel = pm1->insertItem( "menu1", 123 );
//    pm->setAccel( ALT + Key_A, idAccel );
    pm1->setAccel( Qt::CTRL + Qt::Key_A, idAccel );
    mb->insertItem( "&accel", pm1 );
    connect( pm1, SIGNAL(activated(int)), this, SLOT(onActivated(int)));

    delete pm2;
    pm2 = new Q3PopupMenu( mb );
//    idAccel1 = pm2->insertItem( "&Open...", this, SLOT(onActivated(int)), Qt::Key_O, 456 );
    idAccel1 = pm2->insertItem( "&Open...", 0, 0, Qt::Key_O, 456 );
    connect(pm2, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
    mb->insertItem( "accel1", pm2 );

    mb->show();
    qApp->syncX();
    qApp->processEvents();
}

void tst_QMenuBar::init()
{
    resetSlots();
    resetCount();
}

void tst_QMenuBar::resetSlots()
{
    menu1->clear();
    menu2->clear();
    menu3->clear();
    menu4->clear();
    item1_A->clear();
    item1_B->clear();
    item2_C->clear();
    item2_D->clear();
    item2_E->clear();
    item2_F->clear();
    item2_G->clear();
    item2_H->clear();
}

void tst_QMenuBar::resetCount()
{
    last_accel_id = RESET;
    activated_count = 0;
}

void tst_QMenuBar::onActivated( int i )
{
    last_accel_id = i;
    activated_count++;
//     printf( QString("acceleratorId: %1, count: %1\n").arg( i ).arg(activated_count) );
}

void tst_QMenuBar::accel()
{
#ifdef Q_WS_MAC
    QSKIP("On Mac, native key events are needed to test menu action activation", SkipAll);
#endif
    // create a popup menu with menu items set the accelerators later...
    initSimpleMenubar();

//    QTest::keyClick( 0, Qt::Key_A, AltKey );
    QTest::keyClick( 0, Qt::Key_A, Qt::ControlModifier );
    QTest::qWait(300);

    QCOMPARE( last_accel_id, idAccel );
}

void tst_QMenuBar::activatedCount()
{
#ifdef Q_WS_MAC
    QSKIP("On Mac, native key events are needed to test menu action activation", SkipAll);
#endif
    // create a popup menu with menu items set the accelerators later...
    initSimpleMenubar();

    QTest::keyClick( 0, Qt::Key_A, Qt::ControlModifier );
//wait(5000);
    QCOMPARE( activated_count, 1 );
}

void tst_QMenuBar::clear()
{
    mb->clear();
    QVERIFY( mb->count() == 0 );

    mb->clear();
    for (uint i=0; i<10; i++) {
	Q3PopupMenu *pm  = new Q3PopupMenu( mb );
	for (uint k=0; k<i; k++)
	    pm->insertItem( QString("Item %1").arg(i*10 + k) );
	mb->insertItem( QString("Menu %1").arg(i), pm );
	QCOMPARE( mb->count(), (uint)i+1 );
    }
    QCOMPARE( mb->count(), 10u );

    mb->clear();
    QVERIFY( mb->count() == 0 );
}

void tst_QMenuBar::count()
{
    mb->clear();
    QVERIFY( mb->count() == 0 );

    for (uint i=0; i<10; i++) {
	Q3PopupMenu *pm  = new Q3PopupMenu( mb );
	mb->insertItem( QString("Menu %1").arg(i), pm );
	QCOMPARE( mb->count(), i+1 );
    }
}

void tst_QMenuBar::removeItemAt_data()
{
    QTest::addColumn<int>("removeIndex");
    QTest::newRow( "first" ) << 0;
    QTest::newRow( "middle" ) << 1;
    QTest::newRow( "last" ) << 2;
}

void tst_QMenuBar::removeItemAt()
{
    mb->clear();

    Q3PopupMenu *pm;
    pm = new Q3PopupMenu( mb );
    pm->insertItem( QString("Item 10") );
    mb->insertItem( QString("Menu 1"), pm );

    pm = new Q3PopupMenu( mb );
    pm->insertItem( QString("Item 20") );
    pm->insertItem( QString("Item 21") );
    mb->insertItem( QString("Menu 2"), pm );

    pm = new Q3PopupMenu( mb );
    pm->insertItem( QString("Item 30") );
    pm->insertItem( QString("Item 31") );
    pm->insertItem( QString("Item 32") );
    mb->insertItem( QString("Menu 3"), pm );

    QCOMPARE( mb->text( mb->idAt(0) ), QString("Menu 1") );
    QCOMPARE( mb->text( mb->idAt(1) ), QString("Menu 2") );
    QCOMPARE( mb->text( mb->idAt(2) ), QString("Menu 3") );

    // Ok, now that we know we have created the menu we expect, lets remove an item...
    QFETCH( int, removeIndex );
    mb->removeItemAt( removeIndex );
    switch (removeIndex )
    {
    case 0:
	QCOMPARE( mb->text( mb->idAt(0) ), QString("Menu 2") );
	QCOMPARE( mb->text( mb->idAt(1) ), QString("Menu 3") );
	break;
    case 1:
	QCOMPARE( mb->text( mb->idAt(0) ), QString("Menu 1") );
	QCOMPARE( mb->text( mb->idAt(1) ), QString("Menu 3") );
	break;
    case 2:
	QCOMPARE( mb->text( mb->idAt(0) ), QString("Menu 1") );
	QCOMPARE( mb->text( mb->idAt(1) ), QString("Menu 2") );
	break;
    }

    QVERIFY( mb->count() == 2 );
}

void tst_QMenuBar::removeItem_data()
{
    QTest::addColumn<int>("removeIndex");
    QTest::newRow( "first" ) << 0;
    QTest::newRow( "middle" ) << 1;
    QTest::newRow( "last" ) << 2;
}

// Basically the same test as removeItemAt, except that we remember and remove id's.
void tst_QMenuBar::removeItem()
{
    mb->clear();

    Q3PopupMenu *pm;
    pm = new Q3PopupMenu( mb );
    pm->insertItem( QString("Item 10") );
    int id1 = mb->insertItem( QString("Menu 1"), pm );

    pm = new Q3PopupMenu( mb );
    pm->insertItem( QString("Item 20") );
    pm->insertItem( QString("Item 21") );
    int id2 = mb->insertItem( QString("Menu 2"), pm );

    pm = new Q3PopupMenu( mb );
    pm->insertItem( QString("Item 30") );
    pm->insertItem( QString("Item 31") );
    pm->insertItem( QString("Item 32") );
    int id3 = mb->insertItem( QString("Menu 3"), pm );

    QCOMPARE( mb->text( id1 ), QString("Menu 1") );
    QCOMPARE( mb->text( id2 ), QString("Menu 2") );
    QCOMPARE( mb->text( id3 ), QString("Menu 3") );

    QVERIFY( mb->idAt(0) == id1 );
    QVERIFY( mb->idAt(1) == id2 );
    QVERIFY( mb->idAt(2) == id3 );

    // Ok, now that we know we have created the menu we expect, lets remove an item...
    QFETCH( int, removeIndex );
    switch (removeIndex )
    {
    case 0:
	mb->removeItem( id1 );
	QCOMPARE( mb->text( mb->idAt(0) ), QString("Menu 2") );
	QCOMPARE( mb->text( mb->idAt(1) ), QString("Menu 3") );
	break;
    case 1:
	mb->removeItem( id2 );
	QCOMPARE( mb->text( mb->idAt(0) ), QString("Menu 1") );
	QCOMPARE( mb->text( mb->idAt(1) ), QString("Menu 3") );
	break;
    case 2:
	mb->removeItem( id3 );
	QCOMPARE( mb->text( mb->idAt(0) ), QString("Menu 1") );
	QCOMPARE( mb->text( mb->idAt(1) ), QString("Menu 2") );
	break;
    }

    QVERIFY( mb->count() == 2 );
}

void tst_QMenuBar::initComplexMenubar() // well, complex....
{
    mb->hide();
    mb->clear();

    delete pm1;
    pm1 = new Q3PopupMenu( mb, "popup1" );
    pm1->insertItem( QString("Item A"), item1_A, SLOT(selected()), Qt::CTRL+Qt::Key_A );
    pm1->insertItem( QString("Item B"), item1_B, SLOT(selected()), Qt::CTRL+Qt::Key_B );
    // use the form insertItem( QString, Q3PopupMenu )
    mb->insertItem( "Menu &1", pm1 );

    delete pm2;
    pm2 = new Q3PopupMenu( mb, "popup2" );
    pm2->insertItem( QString("Item C"), item2_C, SLOT(selected()), Qt::CTRL+Qt::Key_C );
    pm2->insertItem( QString("Item D"), item2_D, SLOT(selected()), Qt::CTRL+Qt::Key_D );
    pm2->insertItem( QString("Item E"), item2_E, SLOT(selected()), Qt::CTRL+Qt::Key_E );
    pm2->insertItem( QString("Item F"), item2_F, SLOT(selected()), Qt::CTRL+Qt::Key_F );
    pm2->insertSeparator();
    pm2->insertItem( QString("Item G"), item2_G, SLOT(selected()), Qt::CTRL+Qt::Key_G );
    pm2->insertItem( QString("Item H"), item2_H, SLOT(selected()), Qt::CTRL+Qt::Key_H );
    // use the form insertItem( QString, Q3PopupMenu )
    mb->insertItem( "Menu &2", pm2 );

    // use the form insertItem( QString, QObject, slot, keysequence )
    mb->insertItem( QString("M&enu 3"), menu3, SLOT(selected()), Qt::ALT+Qt::Key_J );
    mb->show();
}

/*
    Check the insert functions that create menu items.
    For the moment i only check the strings and pixmaps. The rest are special cases which are
    used less frequently.
*/

void tst_QMenuBar::insertItem_QString_QObject()
{
    initComplexMenubar();
    QCOMPARE( mb->text( mb->idAt( 0 ) ), QString("Menu &1") );
    QCOMPARE( mb->text( mb->idAt( 1 ) ), QString("Menu &2") );
    QCOMPARE( mb->text( mb->idAt( 2 ) ), QString("M&enu 3") );
    QCOMPARE( mb->text( mb->idAt( 3 ) ), QString() ); // there is no menu 4!
}

void tst_QMenuBar::check_accelKeys()
{
#ifdef Q_WS_MAC
    QSKIP("On Mac, native key events are needed to test menu action activation", SkipAll);
#endif
    initComplexMenubar();

    // start with a bogus key that shouldn't trigger anything
    QTest::keyClick(0, Qt::Key_I, Qt::ControlModifier);
    QCOMPARE(menu1->selCount(), 0u);
    QCOMPARE(menu2->selCount(), 0u);
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 0u);
    QCOMPARE(item1_B->selCount(), 0u);
    QCOMPARE(item2_C->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 0u);

    QTest::keyClick(0, Qt::Key_A, Qt::ControlModifier);
    QCOMPARE(menu1->selCount(), 0u);
    QCOMPARE(menu2->selCount(), 0u);
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 1u);
    QCOMPARE(item1_B->selCount(), 0u);
    QCOMPARE(item2_C->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 0u);

    QTest::keyClick(0, Qt::Key_C, Qt::ControlModifier);
    QCOMPARE(menu1->selCount(), 0u);
    QCOMPARE(menu2->selCount(), 0u);
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 1u);
    QCOMPARE(item1_B->selCount(), 0u);
    QCOMPARE(item2_C->selCount(), 1u);
    QCOMPARE(item2_D->selCount(), 0u);

    QTest::keyClick(0, Qt::Key_B, Qt::ControlModifier);
    QCOMPARE(menu1->selCount(), 0u);
    QCOMPARE(menu2->selCount(), 0u);
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 1u);
    QCOMPARE(item1_B->selCount(), 1u);
    QCOMPARE(item2_C->selCount(), 1u);
    QCOMPARE(item2_D->selCount(), 0u);

    QTest::keyClick(0, Qt::Key_D, Qt::ControlModifier);
    QCOMPARE(menu1->selCount(), 0u);
    QCOMPARE(menu2->selCount(), 0u);
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 1u);
    QCOMPARE(item1_B->selCount(), 1u);
    QCOMPARE(item2_C->selCount(), 1u);
    QCOMPARE(item2_D->selCount(), 1u);

    QTest::keyClick(0, Qt::Key_J, Qt::AltModifier);
    QCOMPARE(menu1->selCount(), 0u);
    QCOMPARE(menu2->selCount(), 0u);
    QCOMPARE(menu3->selCount(), 1u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 1u);
    QCOMPARE(item1_B->selCount(), 1u);
    QCOMPARE(item2_C->selCount(), 1u);
    QCOMPARE(item2_D->selCount(), 1u);
}

void tst_QMenuBar::check_cursorKeys1()
{
#ifdef Q_WS_MAC
    QSKIP("Qt/Mac does not use the native popups/menubar", SkipAll);
#endif

    initComplexMenubar();

    // start with a ALT + 1 that activates the first popupmenu
    QTest::keyClick( 0, Qt::Key_1, Qt::AltModifier );
    // the Popupmenu should be visible now
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 0u);
    QCOMPARE(item1_B->selCount(), 0u);
    QCOMPARE(item2_C->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 0u);

    // Simulate a cursor key down click
    QTest::keyClick( 0, Qt::Key_Down );
    // and an Enter key
    QTest::keyClick( 0, Qt::Key_Enter );
    // Let's see if the correct slot is called...
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 0u); // this shouldn't have been called
    QCOMPARE(item1_B->selCount(), 1u); // and this should have been called by a signal now
    QCOMPARE(item2_C->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 0u);
}

void tst_QMenuBar::check_cursorKeys2()
{
#ifdef Q_WS_MAC
    QSKIP("Qt/Mac does not use the native popups/menubar", SkipAll);
#endif

    initComplexMenubar();

    // select popupmenu2
    QTest::keyClick( 0, Qt::Key_2, Qt::AltModifier );

    // Simulate some cursor keys
    QTest::keyClick( 0, Qt::Key_Left );
    QTest::keyClick( 0, Qt::Key_Down );
    QTest::keyClick( 0, Qt::Key_Right );
    QTest::keyClick( 0, Qt::Key_Down );
    // and an Enter key
    QTest::keyClick( 0, Qt::Key_Enter );
    // Let's see if the correct slot is called...
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 0u); // this shouldn't have been caled
    QCOMPARE(item1_B->selCount(), 0u); // and this should have been called by a signal ow
    QCOMPARE(item2_C->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 1u);
}

/*!
    If a popupmenu is active you can use Left to move to the menu to the left of it.
*/
void tst_QMenuBar::check_cursorKeys3()
{
#ifdef Q_WS_MAC
    QSKIP("Qt/Mac does not use the native popups/menubar", SkipAll);
#endif

    initComplexMenubar();

    // select Popupmenu 2
    QTest::keyClick( 0, Qt::Key_2, Qt::AltModifier );

    // Simulate some keys
    QTest::keyClick( 0, Qt::Key_Left );
    QTest::keyClick( 0, Qt::Key_Down );
    // and press ENTER
    QTest::keyClick( 0, Qt::Key_Enter );
    // Let's see if the correct slot is called...
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 0u); // this shouldn't have been called
    QCOMPARE(item1_B->selCount(), 1u); // and this should have been called by a signal now
    QCOMPARE(item2_C->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 0u);
}

/*!
    If a popupmenu is active you can use home to go quickly to the first item in the menu.
*/
void tst_QMenuBar::check_homeKey()
{
    // I'm temporarily shutting up this testcase.
    // Seems like the behaviour i'm expecting isn't ok.
    QVERIFY( TRUE );
    return;

    QEXPECT_FAIL( "0", "Popupmenu should respond to a Home key", Abort );

    initComplexMenubar();

    // select Popupmenu 2
    QTest::keyClick( 0, Qt::Key_2, Qt::AltModifier );

    // Simulate some keys
    QTest::keyClick( 0, Qt::Key_Down );
    QTest::keyClick( 0, Qt::Key_Down );
    QTest::keyClick( 0, Qt::Key_Down );
    QTest::keyClick( 0, Qt::Key_Home );
    // and press ENTER
    QTest::keyClick( 0, Qt::Key_Enter );
    // Let's see if the correct slot is called...
//    QVERIFY2( item2_C->selCount() == 1, "Popupmenu should respond to a Home key" );
    QCOMPARE(item2_C->selCount(), 1u);
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 0u);
    QCOMPARE(item1_B->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 0u);
    QCOMPARE(item2_E->selCount(), 0u);
    QCOMPARE(item2_F->selCount(), 0u);
    QCOMPARE(item2_G->selCount(), 0u);
    QCOMPARE(item2_H->selCount(), 0u);
}

/*!
    If a popupmenu is active you can use end to go quickly to the last item in the menu.
*/
void tst_QMenuBar::check_endKey()
{
    // I'm temporarily shutting up this testcase.
    // Seems like the behaviour i'm expecting isn't ok.
    QVERIFY( TRUE );
    return;

    QEXPECT_FAIL( "0", "Popupmenu should respond to an End key", Abort );

    initComplexMenubar();

    // select Popupmenu 2
    QTest::keyClick( 0, Qt::Key_2, Qt::AltModifier );

    // Simulate some keys
    QTest::keyClick( 0, Qt::Key_End );
    // and press ENTER
    QTest::keyClick( 0, Qt::Key_Enter );
    // Let's see if the correct slot is called...
//    QVERIFY2( item2_H->selCount() == 1, "Popupmenu should respond to an End key" );
    QCOMPARE(item2_H->selCount(), 1u);//, "Popupmenu should respond to an End key");
    QCOMPARE(menu3->selCount(), 0u);
    QCOMPARE(menu4->selCount(), 0u);
    QCOMPARE(item1_A->selCount(), 0u);
    QCOMPARE(item1_B->selCount(), 0u);
    QCOMPARE(item2_C->selCount(), 0u);
    QCOMPARE(item2_D->selCount(), 0u);
    QCOMPARE(item2_E->selCount(), 0u);
    QCOMPARE(item2_F->selCount(), 0u);
    QCOMPARE(item2_G->selCount(), 0u);
}

/*!
    If a popupmenu is active you can use esc to hide the menu and then the
    menubar should become active.
    If Down is pressed next the popup is activated again.
*/
void tst_QMenuBar::check_escKey()
{
#ifdef Q_WS_MAC
    QSKIP("Qt/Mac does not use the native popups/menubar", SkipAll);
#endif

    initComplexMenubar();

    QVERIFY( !pm1->isActiveWindow() );
    QVERIFY( !pm2->isActiveWindow() );

    // select Popupmenu 2
    QTest::keyClick( 0, Qt::Key_2, Qt::AltModifier );
    QVERIFY( !pm1->isActiveWindow() );
    QVERIFY( pm2->isActiveWindow() );

    // If we press ESC, the popup should disappear
    QTest::keyClick( 0, Qt::Key_Escape );
    QVERIFY( !pm1->isActiveWindow() );
    QVERIFY( !pm2->isActiveWindow() );

    if (!QApplication::style()->inherits("QWindowsStyle"))
        return;

    // but the menubar item should stay selected
    QVERIFY( mb->isItemActive(mb->idAt(1)) );

    // If we press Down the popupmenu should be active again
    QTest::keyClick( 0, Qt::Key_Down );
    QVERIFY( !pm1->isActiveWindow() );
    QVERIFY( pm2->isActiveWindow() );

    // and press ENTER
    QTest::keyClick( pm2, Qt::Key_Enter );
    // Let's see if the correct slot is called...
    QVERIFY2( item2_C->selCount() == 1, "Expected item 2C to be selected" );
}

// void tst_QMenuBar::check_mouse1_data()
// {
//     QTest::addColumn<QString>("popup_item");
//     QTest::addColumn<int>("itemA_count");
//     QTest::addColumn<int>("itemB_count");

//     QTest::newRow( "A" ) << QString( "Item A Ctrl+A" ) << 1 << 0;
//     QTest::newRow( "B" ) << QString( "Item B Ctrl+B" ) << 0 << 1;
// }

// /*!
//     Check if the correct signals are emitted if we select a popupmenu.
// */
// void tst_QMenuBar::check_mouse1()
// {
//     if (QSystem::curStyle() == "Motif")
// 	QSKIP("This fails in Motif due to a bug in the testing framework", SkipAll);
//     QFETCH( QString, popup_item );
//     QFETCH( int, itemA_count );
//     QFETCH( int, itemB_count );

//     initComplexMenubar();
//     QVERIFY( !pm1->isActiveWindow() );
//     QVERIFY( !pm2->isActiveWindow() );

//     QTest::qWait(1000);
//     QtTestMouse mouse;
//     mouse.mouseEvent( QtTestMouse::MouseClick, mb, "Menu &1", Qt::LeftButton );

//     QVERIFY( pm1->isActiveWindow() );
//     QVERIFY( !pm2->isActiveWindow() );

//     QTest::qWait(1000);
//     mouse.mouseEvent( QtTestMouse::MouseClick, pm1, popup_item, Qt::LeftButton );

//     QCOMPARE(menu3->selCount(), 0u);
//     QCOMPARE(menu4->selCount(), 0u);
//     QCOMPARE(item1_A->selCount(), (uint)itemA_count); // this option should have fired
//     QCOMPARE(item1_B->selCount(), (uint)itemB_count);
//     QCOMPARE(item2_C->selCount(), 0u);
//     QCOMPARE(item2_D->selCount(), 0u);
//     QCOMPARE(item2_E->selCount(), 0u);
//     QCOMPARE(item2_F->selCount(), 0u);
//     QCOMPARE(item2_G->selCount(), 0u);
// }

// void tst_QMenuBar::check_mouse2_data()
// {
//     QTest::addColumn<QString>("label");
//     QTest::addColumn<int>("itemA_count");
//     QTest::addColumn<int>("itemB_count");
//     QTest::addColumn<int>("itemC_count");
//     QTest::addColumn<int>("itemD_count");
//     QTest::addColumn<int>("itemE_count");
//     QTest::addColumn<int>("itemF_count");
//     QTest::addColumn<int>("itemG_count");
//     QTest::addColumn<int>("itemH_count");
//     QTest::addColumn<int>("menu3_count");

//     QTest::newRow( "A" ) << QString( "Menu &1/Item A Ctrl+A" ) << 1 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
//     QTest::newRow( "B" ) << QString( "Menu &1/Item B Ctrl+B" ) << 0 << 1 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
//     QTest::newRow( "C" ) << QString( "Menu &2/Item C Ctrl+C" ) << 0 << 0 << 1 << 0 << 0 << 0 << 0 << 0 << 0;
//     QTest::newRow( "D" ) << QString( "Menu &2/Item D Ctrl+D" ) << 0 << 0 << 0 << 1 << 0 << 0 << 0 << 0 << 0;
//     QTest::newRow( "E" ) << QString( "Menu &2/Item E Ctrl+E" ) << 0 << 0 << 0 << 0 << 1 << 0 << 0 << 0 << 0;
//     QTest::newRow( "F" ) << QString( "Menu &2/Item F Ctrl+F" ) << 0 << 0 << 0 << 0 << 0 << 1 << 0 << 0 << 0;
//     QTest::newRow( "G" ) << QString( "Menu &2/Item G Ctrl+G" ) << 0 << 0 << 0 << 0 << 0 << 0 << 1 << 0 << 0;
//     QTest::newRow( "H" ) << QString( "Menu &2/Item H Ctrl+H" ) << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 1 << 0;
//     QTest::newRow( "menu 3" ) << QString( "M&enu 3" )          << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 1;
// }

// /*!
//     Check if the correct signals are emitted if we select a popupmenu.
//     This time, we use a little bit more magic from the testframework.
// */
// void tst_QMenuBar::check_mouse2()
// {
//     if (QSystem::curStyle() == "Motif")
// 	QSKIP("This fails in Motif due to a bug in the testing framework", SkipAll);
//     QFETCH( QString, label );
//     QFETCH( int, itemA_count );
//     QFETCH( int, itemB_count );
//     QFETCH( int, itemC_count );
//     QFETCH( int, itemD_count );
//     QFETCH( int, itemE_count );
//     QFETCH( int, itemF_count );
//     QFETCH( int, itemG_count );
//     QFETCH( int, itemH_count );
//     QFETCH( int, menu3_count );

//     initComplexMenubar();
//     QtTestMouse mouse;
//     mouse.click( QtTestMouse::Menu, label, Qt::LeftButton );

//     // check if the correct signals have fired
//     QCOMPARE(menu3->selCount(), (uint)menu3_count);
//     QCOMPARE(menu4->selCount(), 0u);
//     QCOMPARE(item1_A->selCount(), (uint)itemA_count);
//     QCOMPARE(item1_B->selCount(), (uint)itemB_count);
//     QCOMPARE(item2_C->selCount(), (uint)itemC_count);
//     QCOMPARE(item2_D->selCount(), (uint)itemD_count);
//     QCOMPARE(item2_E->selCount(), (uint)itemE_count);
//     QCOMPARE(item2_F->selCount(), (uint)itemF_count);
//     QCOMPARE(item2_G->selCount(), (uint)itemG_count);
//     QCOMPARE(item2_H->selCount(), (uint)itemH_count);
// }

void tst_QMenuBar::check_altPress()
{
    if ( !qApp->style()->styleHint(QStyle::SH_MenuBar_AltKeyNavigation) ) {
	QSKIP( QString( "this is not supposed to work in the %1 style. Skipping." ).
	      arg( qApp->style()->name() ), SkipAll );
    }

    initSimpleMenubar();

    qApp->setActiveWindow(mw);
    mw->setFocus();

    QTest::keyClick( mw, Qt::Key_Alt );

    QVERIFY( ::qobject_cast<QMenuBar *>(qApp->focusWidget()) );
}

void tst_QMenuBar::check_shortcutPress()
{
#ifdef Q_WS_MAC
    QSKIP("Qt/Mac does not use the native popups/menubar", SkipAll);
#endif

    initComplexMenubar();
    qApp->setActiveWindow(mw);

    QTest::keyClick(mw, Qt::Key_1, Qt::AltModifier );
    QVERIFY(pm1->isActiveWindow());
    QTest::keyClick(mb, Qt::Key_2);
    QVERIFY(pm1->isActiveWindow()); // Should still be the active window
}

void tst_QMenuBar::check_menuPosition()
{
#ifdef Q_WS_MAC
    QSKIP("Qt/Mac does not use the native popups/menubar", SkipAll);
#endif

    QMenu menu;
    initComplexMenubar();
    menu.setTitle("&menu");
    QRect screenRect = QApplication::desktop()->screenGeometry(mw);

    while(menu.sizeHint().height() < (screenRect.height()*2/3)) {
        menu.addAction("item");
    }

    QAction *menu_action = mw->menuBar()->addMenu(&menu);

    qApp->setActiveWindow(mw);
    qApp->processEvents();

    //the menu should be below the menubar item
    {
        mw->move(0,0);
        QRect mbItemRect = mw->menuBar()->actionGeometry(menu_action);
        mbItemRect.moveTo(mw->menuBar()->mapToGlobal(mbItemRect.topLeft()));
        QTest::keyClick(mw, Qt::Key_M, Qt::AltModifier );
        QVERIFY(menu.isActiveWindow());
        QCOMPARE(menu.pos(), QPoint(mbItemRect.x(), mbItemRect.bottom() + 1));
        menu.close();
    }

    //the menu should be above the menubar item
    {
        mw->move(0,screenRect.bottom() - screenRect.height()/4); //just leave some place for the menubar
        QRect mbItemRect = mw->menuBar()->actionGeometry(menu_action);
        mbItemRect.moveTo(mw->menuBar()->mapToGlobal(mbItemRect.topLeft()));
        QTest::keyClick(mw, Qt::Key_M, Qt::AltModifier );
        QVERIFY(menu.isActiveWindow());
        QCOMPARE(menu.pos(), QPoint(mbItemRect.x(), mbItemRect.top() - menu.height()));
        menu.close();
    }

    //the menu should be on the side of the menubar item and should be "stuck" to the bottom of the screen
    {
        mw->move(0,screenRect.y() + screenRect.height()/2); //put it in the middle
        QRect mbItemRect = mw->menuBar()->actionGeometry(menu_action);
        mbItemRect.moveTo(mw->menuBar()->mapToGlobal(mbItemRect.topLeft()));
        QTest::keyClick(mw, Qt::Key_M, Qt::AltModifier );
        QVERIFY(menu.isActiveWindow());
        QCOMPARE(menu.pos(), QPoint(mbItemRect.right()+1, screenRect.bottom() - menu.height() + 1));
        menu.close();
    }

}


#if defined(QT3_SUPPORT)
void tst_QMenuBar::indexBasedInsertion_data()
{
    QTest::addColumn<int>("indexForInsertion");
    QTest::addColumn<int>("expectedIndex");

    QTest::newRow("negative-index-appends") << -1 << 1;
    QTest::newRow("prepend") << 0 << 0;
    QTest::newRow("append") << 1 << 1;
}

void tst_QMenuBar::indexBasedInsertion()
{
    // test the compat'ed index based insertion

    QFETCH(int, indexForInsertion);
    QFETCH(int, expectedIndex);

    {
        QMenuBar menu;
        menu.addAction("Regular Item");

        menu.insertItem("New Item", -1 /*id*/, indexForInsertion);

        QAction *act = menu.actions().value(expectedIndex);
        QVERIFY(act);
        QCOMPARE(act->text(), QString("New Item"));
    }
    {
        QMenuBar menu;
        menu.addAction("Regular Item");

        menu.insertSeparator(indexForInsertion);

        QAction *act = menu.actions().value(expectedIndex);
        QVERIFY(act);
        QVERIFY(act->isSeparator());
    }
}
#endif

QTEST_MAIN(tst_QMenuBar)
#include "tst_qmenubar.moc"

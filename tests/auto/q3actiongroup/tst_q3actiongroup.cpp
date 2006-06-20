/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <q3action.h>
#include <q3mainwindow.h>
#include <q3toolbar.h>
#include <qcombobox.h>
#include <qlist.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <q3popupmenu.h>

#include <qtoolbar.h>

//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3action.h compat/widgets/q3action.cpp

class tst_Q3ActionGroup : public QObject
{
    Q_OBJECT

public:
    tst_Q3ActionGroup();
    virtual ~tst_Q3ActionGroup();

private slots:
    void enabledPropagation();
    void visiblePropagation();
    void dropDownDeleted();
    void exclusive();

    void separators();
};

tst_Q3ActionGroup::tst_Q3ActionGroup()
{
}

tst_Q3ActionGroup::~tst_Q3ActionGroup()
{

}

void tst_Q3ActionGroup::enabledPropagation()
{
    Q3ActionGroup testActionGroup( 0 );

    Q3Action* childAction = new Q3Action( &testActionGroup );
    Q3Action* anotherChildAction = new Q3Action( &testActionGroup );
    Q3Action* freeAction = new Q3Action(0);

    QVERIFY( testActionGroup.isEnabled() );
    QVERIFY( childAction->isEnabled() );

    testActionGroup.setEnabled( FALSE );
    QVERIFY( !testActionGroup.isEnabled() );
    QVERIFY( !childAction->isEnabled() );
    QVERIFY( !anotherChildAction->isEnabled() );

    anotherChildAction->setEnabled( FALSE );

    testActionGroup.setEnabled( TRUE );
    QVERIFY( testActionGroup.isEnabled() );
    QVERIFY( childAction->isEnabled() );
    QVERIFY( !anotherChildAction->isEnabled() );

    testActionGroup.setEnabled( FALSE );
    Q3Action *lastChildAction = new Q3Action(&testActionGroup);

    QVERIFY(!lastChildAction->isEnabled());
    testActionGroup.setEnabled( TRUE );
    QVERIFY(lastChildAction->isEnabled());

    freeAction->setEnabled(FALSE);
    testActionGroup.add(freeAction);
    QVERIFY(!freeAction->isEnabled());
    delete freeAction;
}

void tst_Q3ActionGroup::visiblePropagation()
{
    Q3ActionGroup testActionGroup( 0 );

    Q3Action* childAction = new Q3Action( &testActionGroup );
    Q3Action* anotherChildAction = new Q3Action( &testActionGroup );
    Q3Action* freeAction = new Q3Action(0);

    QVERIFY( testActionGroup.isVisible() );
    QVERIFY( childAction->isVisible() );

    testActionGroup.setVisible( FALSE );
    QVERIFY( !testActionGroup.isVisible() );
    QVERIFY( !childAction->isVisible() );
    QVERIFY( !anotherChildAction->isVisible() );

    anotherChildAction->setVisible(FALSE);

    testActionGroup.setVisible( TRUE );
    QVERIFY( testActionGroup.isVisible() );
    QVERIFY( childAction->isVisible() );

    QVERIFY( !anotherChildAction->isVisible() );

    testActionGroup.setVisible( FALSE );
    Q3Action *lastChildAction = new Q3Action(&testActionGroup);

    QVERIFY(!lastChildAction->isVisible());
    testActionGroup.setVisible( TRUE );
    QVERIFY(lastChildAction->isVisible());

    freeAction->setVisible(FALSE);
    testActionGroup.add(freeAction);
    QVERIFY(!freeAction->isVisible());
    delete freeAction;
}

void tst_Q3ActionGroup::exclusive()
{
    Q3ActionGroup group( 0, 0, FALSE );
    QVERIFY( !group.isExclusive() );

    Q3Action* actOne = new Q3Action( &group );
    actOne->setToggleAction( TRUE );
    Q3Action* actTwo = new Q3Action( &group );
    actTwo->setToggleAction( TRUE );
    Q3Action* actThree = new Q3Action( &group );
    actThree->setToggleAction( TRUE );

    group.setExclusive( TRUE );
    QVERIFY( !actOne->isOn() );
    QVERIFY( !actTwo->isOn() );
    QVERIFY( !actThree->isOn() );

    actOne->setOn( TRUE );
    QVERIFY( actOne->isOn() );
    QVERIFY( !actTwo->isOn() );
    QVERIFY( !actThree->isOn() );

    actTwo->setOn( TRUE );
    QVERIFY( !actOne->isOn() );
    QVERIFY( actTwo->isOn() );
    QVERIFY( !actThree->isOn() );
}

void tst_Q3ActionGroup::dropDownDeleted()
{
    Q3MainWindow mw;
    Q3ToolBar *tb = new Q3ToolBar(&mw);
    Q3ActionGroup *actGroup = new Q3ActionGroup(&mw);
    actGroup->setUsesDropDown(TRUE);
    Q3Action *actOne = new Q3Action(actGroup);
    actOne->setText("test one");
    Q3Action *actTwo = new Q3Action(actGroup);
    actTwo->setText("test two");
    Q3Action *actThree= new Q3Action(actGroup);
    actThree->setText("test three");
    actGroup->addTo(tb);
    QObjectList comboList = tb->queryList("QComboBox");
    QCOMPARE(comboList.count(), 1);
    QCOMPARE((int)((QComboBox*)comboList[0])->count(), 3);

    delete actOne;
    QCOMPARE((int)((QComboBox*)comboList[0])->count(), 2);
    delete actTwo;
    QCOMPARE((int)((QComboBox*)comboList[0])->count(), 1);
    delete actThree;
    QCOMPARE((int)((QComboBox*)comboList[0])->count(), 0);

    delete actGroup;
}

void tst_Q3ActionGroup::separators()
{
    QMainWindow mw;
    Q3PopupMenu menu(&mw);
    Q3ActionGroup actGroup(&mw);

    mw.show();

    Q3Action *action = new Q3Action(&actGroup);
    action->setText("test one");
    actGroup.addSeparator();

    actGroup.addTo(&menu);
    QCOMPARE((int)menu.count(), 2);

    actGroup.removeFrom(&menu);
    QCOMPARE((int)menu.count(), 0);

    action = new Q3Action(&actGroup);
    action->setText("test two");
    actGroup.addTo(&menu);
    QCOMPARE((int)menu.count(), 3);
}

QTEST_MAIN(tst_Q3ActionGroup)
#include "tst_q3actiongroup.moc"


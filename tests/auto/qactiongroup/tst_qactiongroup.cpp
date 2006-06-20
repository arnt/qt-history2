/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcombobox.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qtoolbar.h>


#include <qaction.h>

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qactiongroup.h gui/kernel/qactiongroup.cpp

class tst_QActionGroup : public QObject
{
    Q_OBJECT

public:
    tst_QActionGroup();
    virtual ~tst_QActionGroup();

private slots:
    void enabledPropagation();
    void visiblePropagation();
    void dropDownDeleted();
    void exclusive();

    void separators();
};

tst_QActionGroup::tst_QActionGroup()
{
}

tst_QActionGroup::~tst_QActionGroup()
{
}

void tst_QActionGroup::enabledPropagation()
{
    QActionGroup testActionGroup( 0 );

    QAction* childAction = new QAction( &testActionGroup );
    QAction* anotherChildAction = new QAction( &testActionGroup );
    QAction* freeAction = new QAction(0);

    QVERIFY( testActionGroup.isEnabled() );
    QVERIFY( childAction->isEnabled() );

    testActionGroup.setEnabled( FALSE );
    QVERIFY( !testActionGroup.isEnabled() );
    QVERIFY( !childAction->isEnabled() );
    QVERIFY( !anotherChildAction->isEnabled() );

    childAction->setEnabled(true);
    QVERIFY( !childAction->isEnabled());

    anotherChildAction->setEnabled( FALSE );

    testActionGroup.setEnabled( TRUE );
    QVERIFY( testActionGroup.isEnabled() );
    QVERIFY( childAction->isEnabled() );
    QVERIFY( !anotherChildAction->isEnabled() );

    testActionGroup.setEnabled( FALSE );
    QAction *lastChildAction = new QAction(&testActionGroup);

    QVERIFY(!lastChildAction->isEnabled());
    testActionGroup.setEnabled( TRUE );
    QVERIFY(lastChildAction->isEnabled());

    freeAction->setEnabled(FALSE);
    testActionGroup.addAction(freeAction);
    QVERIFY(!freeAction->isEnabled());
    delete freeAction;
}

void tst_QActionGroup::visiblePropagation()
{
    QActionGroup testActionGroup( 0 );

    QAction* childAction = new QAction( &testActionGroup );
    QAction* anotherChildAction = new QAction( &testActionGroup );
    QAction* freeAction = new QAction(0);

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
    QAction *lastChildAction = new QAction(&testActionGroup);

    QVERIFY(!lastChildAction->isVisible());
    testActionGroup.setVisible( TRUE );
    QVERIFY(lastChildAction->isVisible());

    freeAction->setVisible(FALSE);
    testActionGroup.addAction(freeAction);
    QVERIFY(!freeAction->isVisible());
    delete freeAction;
}

void tst_QActionGroup::exclusive()
{
    QActionGroup group(0);
    group.setExclusive(false);
    QVERIFY( !group.isExclusive() );

    QAction* actOne = new QAction( &group );
    actOne->setCheckable( TRUE );
    QAction* actTwo = new QAction( &group );
    actTwo->setCheckable( TRUE );
    QAction* actThree = new QAction( &group );
    actThree->setCheckable( TRUE );

    group.setExclusive( TRUE );
    QVERIFY( !actOne->isChecked() );
    QVERIFY( !actTwo->isChecked() );
    QVERIFY( !actThree->isChecked() );

    actOne->setChecked( TRUE );
    QVERIFY( actOne->isChecked() );
    QVERIFY( !actTwo->isChecked() );
    QVERIFY( !actThree->isChecked() );

    actTwo->setChecked( TRUE );
    QVERIFY( !actOne->isChecked() );
    QVERIFY( actTwo->isChecked() );
    QVERIFY( !actThree->isChecked() );
}

void tst_QActionGroup::dropDownDeleted()
{
    QSKIP("dropDownDeleted test for Qt 4.0 not expected to work since it is not implemented yet", SkipAll);

    QMainWindow mw;
    QToolBar *tb = new QToolBar(&mw);
    QActionGroup *actGroup = new QActionGroup(&mw);

    /// ### actGroup->setUsesDropDown(TRUE);
    QAction *actOne = new QAction(actGroup);
    actOne->setText("test one");
    QAction *actTwo = new QAction(actGroup);
    actOne->setText("test one");
    QAction *actThree= new QAction(actGroup);
    actOne->setText("test one");

    QListIterator<QAction*> it(actGroup->actions());
    while (it.hasNext())
        tb->addAction(it.next());

    QList<QComboBox*> comboList = qFindChildren<QComboBox*>(tb);
    QCOMPARE(comboList[0]->count(), 3);

    delete actOne;
    QCOMPARE((int)comboList[0]->count(), 2);
    delete actTwo;
    QCOMPARE((int)comboList[0]->count(), 1);
    delete actThree;
    QCOMPARE((int)comboList[0]->count(), 0);

    delete actGroup;
}

void tst_QActionGroup::separators()
{
    QMainWindow mw;
    QMenu menu(&mw);
    QActionGroup actGroup(&mw);

    mw.show();

    QAction *action = new QAction(&actGroup);
    action->setText("test one");

    QAction *separator = new QAction(&actGroup);
    separator->setSeparator(true);
    actGroup.addAction(separator);

    QListIterator<QAction*> it(actGroup.actions());
    while (it.hasNext())
        menu.addAction(it.next());

    QCOMPARE((int)menu.actions().size(), 2);

    it = QListIterator<QAction*>(actGroup.actions());
    while (it.hasNext())
        menu.removeAction(it.next());

    QCOMPARE((int)menu.actions().size(), 0);

    action = new QAction(&actGroup);
    action->setText("test two");

    it = QListIterator<QAction*>(actGroup.actions());
    while (it.hasNext())
        menu.addAction(it.next());

    QCOMPARE((int)menu.actions().size(), 3);
}

QTEST_MAIN(tst_QActionGroup)
#include "tst_qactiongroup.moc"

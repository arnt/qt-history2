/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qaction.h>
#include <qwindowsstyle.h>

//TESTED_CLASS=
//TESTED_FILES=qtoolbutton.h

class tst_QToolButton : public QObject
{
Q_OBJECT

public:
    tst_QToolButton();
    virtual ~tst_QToolButton();

private slots:
    void getSetCheck();
};

tst_QToolButton::tst_QToolButton()
{
}

tst_QToolButton::~tst_QToolButton()
{
}

// Testing get/set functions
void tst_QToolButton::getSetCheck()
{
    QToolButton obj1;
    // QMenu* QToolButton::menu()
    // void QToolButton::setMenu(QMenu*)
    QMenu *var1 = new QMenu;
    obj1.setMenu(var1);
    QCOMPARE(var1, obj1.menu());
    obj1.setMenu((QMenu *)0);
    QCOMPARE((QMenu *)0, obj1.menu());
    delete var1;

    // ToolButtonPopupMode QToolButton::popupMode()
    // void QToolButton::setPopupMode(ToolButtonPopupMode)
    obj1.setPopupMode(QToolButton::ToolButtonPopupMode(QToolButton::DelayedPopup));
    QCOMPARE(QToolButton::ToolButtonPopupMode(QToolButton::DelayedPopup), obj1.popupMode());
    obj1.setPopupMode(QToolButton::ToolButtonPopupMode(QToolButton::MenuButtonPopup));
    QCOMPARE(QToolButton::ToolButtonPopupMode(QToolButton::MenuButtonPopup), obj1.popupMode());
    obj1.setPopupMode(QToolButton::ToolButtonPopupMode(QToolButton::InstantPopup));
    QCOMPARE(QToolButton::ToolButtonPopupMode(QToolButton::InstantPopup), obj1.popupMode());

    // bool QToolButton::autoRaise()
    // void QToolButton::setAutoRaise(bool)
    obj1.setAutoRaise(false);
    QCOMPARE(false, obj1.autoRaise());
    obj1.setAutoRaise(true);
    QCOMPARE(true, obj1.autoRaise());

    // QAction * QToolButton::defaultAction()
    // void QToolButton::setDefaultAction(QAction *)
    QAction *var4 = new QAction(0);
    obj1.setDefaultAction(var4);
    QCOMPARE(var4, obj1.defaultAction());
    obj1.setDefaultAction((QAction *)0);
    QCOMPARE((QAction *)0, obj1.defaultAction());
    delete var4;

	
	//ensure that popup delay is not reset on style change
	obj1.setPopupDelay(5);
	obj1.setStyle(new QWindowsStyle);
	QCOMPARE(obj1.popupDelay(), 5);
}

QTEST_MAIN(tst_QToolButton)
#include "tst_qtoolbutton.moc"

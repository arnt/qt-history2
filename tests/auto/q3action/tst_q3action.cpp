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

QT_USE_NAMESPACE

//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3action.h compat/widgets/q3action.cpp


class tst_Q3Action : public QObject
{
    Q_OBJECT

public:
    tst_Q3Action();
    virtual ~tst_Q3Action();

private slots:
    void getSetCheck();
    void setText_data();
    void setText();
    void toolTip();
};

// Testing get/set functions
void tst_Q3Action::getSetCheck()
{
    Q3ActionGroup obj1(0);
    // bool Q3ActionGroup::usesDropDown()
    // void Q3ActionGroup::setUsesDropDown(bool)
    obj1.setUsesDropDown(false);
    QCOMPARE(false, obj1.usesDropDown());
    obj1.setUsesDropDown(true);
    QCOMPARE(true, obj1.usesDropDown());
}



tst_Q3Action::tst_Q3Action()
{
}

tst_Q3Action::~tst_Q3Action()
{

}

void tst_Q3Action::setText_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("menuText");
    QTest::addColumn<QString>("toolTip");
    QTest::addColumn<QString>("statusTip");
    QTest::addColumn<QString>("whatsThis");

    //next we fill it with data
    QTest::newRow("Normal") << "Action" << "Action" << "Action" << "Action" << QString();
    QTest::newRow("Ampersand") << "Search & Destroy" << "Search && Destroy" << "Search & Destroy" << "Search & Destroy" << QString();
}

void tst_Q3Action::setText()
{
    QFETCH(QString,text);
    QFETCH(QString,menuText);
    QFETCH(QString,toolTip);
    QFETCH(QString,statusTip);
    QFETCH(QString,whatsThis);

    Q3Action action(0);
    action.setText(text);
    QCOMPARE(action.menuText(), menuText);
    QCOMPARE(action.toolTip(), toolTip);
    QCOMPARE(action.statusTip(), statusTip);
    QCOMPARE(action.whatsThis(), whatsThis);
}

void tst_Q3Action::toolTip()
{
    QWidget widget;
    Q3Action action(&widget);
    action.setAccel(QKeySequence(Qt::CTRL | Qt::Key_A));
    QCOMPARE(action.toolTip(), QString(" (Ctrl+A)"));
}

QTEST_MAIN(tst_Q3Action)
#include "tst_q3action.moc"


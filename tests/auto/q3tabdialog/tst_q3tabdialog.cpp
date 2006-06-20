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
#include <qdebug.h>
#include <q3tabdialog.h>
#include <qtabbar.h>

//TESTED_CLASS=
//TESTED_FILES=q3tabdialog.h

class tst_Q3TabDialog : public QObject
{
Q_OBJECT

public:
    tst_Q3TabDialog();
    virtual ~tst_Q3TabDialog();

private slots:
    void getSetCheck();
};

tst_Q3TabDialog::tst_Q3TabDialog()
{
}

tst_Q3TabDialog::~tst_Q3TabDialog()
{
}

class My3TabDialog : public Q3TabDialog
{
public:
    My3TabDialog() : Q3TabDialog() {}
    void setTabBar(QTabBar* bar) { Q3TabDialog::setTabBar(bar); }
    QTabBar* tabBar() const { return Q3TabDialog::tabBar(); }
};

// Testing get/set functions
void tst_Q3TabDialog::getSetCheck()
{
    My3TabDialog obj1;
    // QTabBar* Q3TabDialog::tabBar()
    // void Q3TabDialog::setTabBar(QTabBar*)
    QTabBar *var1 = new QTabBar;
    obj1.setTabBar(var1);
    QCOMPARE(var1, obj1.tabBar());
#if QT_VERSION >= 0x040200
    // Assert in QTabWidget on this. Should be handled gracefully in Qt 4.2
    obj1.setTabBar((QTabBar *)0);
    QCOMPARE((QTabBar *)0, obj1.tabBar());
#endif
    delete var1;
}

QTEST_MAIN(tst_Q3TabDialog)
#include "tst_q3tabdialog.moc"

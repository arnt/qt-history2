/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <q3datetimeedit.h>
#include <qapplication.h>
#include <qgroupbox.h>

//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3action.h compat/widgets/q3action.cpp

class tst_Q3DateEdit : public QObject
{
    Q_OBJECT

public:
    tst_Q3DateEdit();
    virtual ~tst_Q3DateEdit();



public slots:
    void initTestCase();
    void init();
    void cleanup();
private slots:
    void enabledPropagation();

private:
    Q3DateEdit* testWidget;
};

tst_Q3DateEdit::tst_Q3DateEdit()
{
}

tst_Q3DateEdit::~tst_Q3DateEdit()
{

}

void tst_Q3DateEdit::initTestCase()
{
    testWidget = new Q3DateEdit( 0, "testWidget" );
    testWidget->show();
    qApp->setActiveWindow(testWidget);
    qApp->setMainWidget( testWidget );
    QTest::qWait(100);
}

void tst_Q3DateEdit::init()
{
}

void tst_Q3DateEdit::cleanup()
{
}


void tst_Q3DateEdit::enabledPropagation()
{
    // Check a QDateEdit on its own
    testWidget->setEnabled(TRUE);
    QVERIFY(testWidget->isEnabled());
    testWidget->setEnabled(FALSE);
    QVERIFY(!testWidget->isEnabled());
    testWidget->setEnabled(TRUE);
    QVERIFY(testWidget->isEnabled());

    // Now check a QDateEdit on a QWidget
    QWidget w;
    Q3DateEdit *childOfW = new Q3DateEdit(&w, "childOfW");
    w.show();
    QVERIFY(childOfW->isEnabled());
    QObjectList children = childOfW->children();
    int i;
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(static_cast<QWidget *>(children.at(i))->isEnabled());
    }
    w.setEnabled(FALSE);
    QVERIFY(!childOfW->isEnabled());
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(!static_cast<QWidget *>(children.at(i))->isEnabled());
    }
    w.setEnabled(TRUE);
    QVERIFY(childOfW->isEnabled());
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(static_cast<QWidget *>(children.at(i))->isEnabled());
    }

    // Now check a QDateEdit on a non-checkable QGroupBox
    QGroupBox *gb = new QGroupBox(&w, "nonCheckGroupBox");
    Q3DateEdit *childOfGB = new Q3DateEdit(gb, "childOfGB");
    gb->show();
    QVERIFY(childOfGB->isEnabled());
    children = childOfGB->children();
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(static_cast<QWidget *>(children.at(i))->isEnabled());
    }

    gb->setEnabled(FALSE);
    QVERIFY(!childOfGB->isEnabled());
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(!static_cast<QWidget *>(children.at(i))->isEnabled());
    }
    gb->setEnabled(TRUE);
    QVERIFY(childOfGB->isEnabled());
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(static_cast<QWidget *>(children.at(i))->isEnabled());
    }

    // Now check a QDateEdit on a checkable QGroupBox
    QGroupBox *cgb = new QGroupBox(&w, "checkGroupBox");
    cgb->setCheckable(TRUE);
    Q3DateEdit *childOfCGB = new Q3DateEdit(cgb, "childOfCGB");
    cgb->show();
    QVERIFY(childOfCGB->isEnabled());
    children = childOfCGB->children();
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(static_cast<QWidget *>(children.at(i))->isEnabled());
    }
    cgb->setChecked(FALSE);
    QVERIFY(!childOfCGB->isEnabled());
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(!static_cast<QWidget *>(children.at(i))->isEnabled());
    }
    cgb->setChecked(TRUE);
    QVERIFY(childOfCGB->isEnabled());
    for (i = 0; i < children.count(); ++i) {
        if (children.at(i)->isWidgetType())
	    QVERIFY(static_cast<QWidget *>(children.at(i))->isEnabled());
    }
}


QTEST_MAIN(tst_Q3DateEdit)
#include "tst_q3dateedit.moc"


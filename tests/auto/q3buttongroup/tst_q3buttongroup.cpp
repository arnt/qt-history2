/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <Q3ButtonGroup>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QDebug>
#include <QLayout>
#include <QtTest/QtTest>

class tst_q3buttongroup : public QObject
{
Q_OBJECT
private slots:
    void exclusiveButtons();
    void nonExclusiveButtons();
    void buttonIds();
    void buttonId();
    void clickLock();
};

/*
    Test that check boxes created with a Q3ButtonGroup parent in exclusive
    mode really are exclusive.
*/
void tst_q3buttongroup::exclusiveButtons()
{
    Q3ButtonGroup group(1, Qt::Horizontal);
    group.setExclusive(true);

    QCheckBox *b1 = new QCheckBox("Hi", &group);
    QCheckBox *b2 = new QCheckBox("there", &group);
    QCheckBox *b3 = new QCheckBox("foo", &group);

    group.show();

    // Check b1 and verify that it stuck.
    b1->setCheckState(Qt::Checked);
    QCOMPARE(b1->checkState(), Qt::Checked);

    // Check b2 and verify that b1 is now unchecked.
    b2->setCheckState(Qt::Checked);
    QCOMPARE(b1->checkState(), Qt::Unchecked);

    // Check b3 and verify that b2 and b1 are now unchecked.
    b3->setCheckState(Qt::Checked);
    QCOMPARE(b1->checkState(), Qt::Unchecked);
    QCOMPARE(b2->checkState(), Qt::Unchecked);
}

/*
    Test that setting exclusive to false works.
*/
void tst_q3buttongroup::nonExclusiveButtons()
{
    Q3ButtonGroup group(1, Qt::Horizontal);

    QWidget parent;

    QCheckBox *b1 = new QCheckBox("Hi", &parent);
    group.insert(b1);
    QCheckBox *b2 = new QCheckBox("there", &parent);
    group.insert(b2);
    QCheckBox *b3 = new QCheckBox("foo", &parent);
    group.insert(b3);

    group.setExclusive(false);
    group.show();

    // Check b1 and verify that it stuck.
    b1->setCheckState(Qt::Checked);
    QCOMPARE(b1->checkState(), Qt::Checked);

    // Check b2 and verify that b1 is still checked.
    b2->setCheckState(Qt::Checked);
    QCOMPARE(b1->checkState(), Qt::Checked);

    // Check b3 and verify that b2 and b1 are still checked.
    b3->setCheckState(Qt::Checked);
    QCOMPARE(b1->checkState(), Qt::Checked);
    QCOMPARE(b2->checkState(), Qt::Checked);
}

/*
    Test that Ids get assigned
*/
void tst_q3buttongroup::buttonIds()
{
    Q3ButtonGroup group(0, Qt::Vertical, "ButtonGroup");
    group.setExclusive(true);
    QVERIFY(group.isExclusive());

    for (int i=0; i < 10; i++) {
        QRadioButton *button = new QRadioButton(QString("Button_%1").arg(i + 1) , &group);
        QCOMPARE(group.id(button) , i);
        int id = group.insert(button);
        QCOMPARE(id, i);
        group.setButton(id);
        QCOMPARE(group.selectedId(), id);
    }

    QCheckBox *button2 = new QCheckBox(QString("manuallyAdded"));
    int id = group.insert( button2 );
    QCOMPARE(id , 10 );

    button2->setChecked(true);
    QCOMPARE( group.selectedId() , id );

    group.remove(group.find(5));
    QCOMPARE(group.count() , 10);

    delete button2;
}

void tst_q3buttongroup::buttonId()
{
    Q3ButtonGroup bg;
    QPushButton *button = new QPushButton("Foo", &bg);
    int id = bg.insert(button, 1);
    QApplication::instance()->processEvents();
    QCOMPARE(id, bg.id(button));
}

void tst_q3buttongroup::clickLock()
{
    // Task 177677
    QProcess process;
    process.start(QLatin1String("clickLock/clickLock"));
    if (!process.waitForStarted(10000)) {
        QFAIL("Could not launch process.");
    }

    if (!process.waitForFinished(15000)) {
        process.terminate();
        QFAIL("Could not handle click events properly");
    }
}

QTEST_MAIN(tst_q3buttongroup)
#include "tst_q3buttongroup.moc"

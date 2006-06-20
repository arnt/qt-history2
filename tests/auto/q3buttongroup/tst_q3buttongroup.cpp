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
#include <QDebug>
#include <QtTest/QtTest>

class tst_q3buttongroup : public QObject
{
Q_OBJECT
private slots:
    void exclusiveButtons();
    void nonExclusiveButtons();
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


QTEST_MAIN(tst_q3buttongroup)
#include "tst_q3buttongroup.moc"

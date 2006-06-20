/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QGroupBox>
#include <Q3GroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QtTest/QtTest>

class tst_q3groupbox : public QObject
{
Q_OBJECT
private slots:
    void getSetCheck();
    void groupBoxHeight();
};

// Testing get/set functions
void tst_q3groupbox::getSetCheck()
{
    Q3GroupBox obj1;
    // int Q3GroupBox::insideMargin()
    // void Q3GroupBox::setInsideMargin(int)
    obj1.setInsideMargin(0);
    QCOMPARE(0, obj1.insideMargin());
    obj1.setInsideMargin(INT_MIN);
    QCOMPARE(INT_MIN, obj1.insideMargin());
    obj1.setInsideMargin(INT_MAX);
    QCOMPARE(INT_MAX, obj1.insideMargin());

    // int Q3GroupBox::insideSpacing()
    // void Q3GroupBox::setInsideSpacing(int)
    obj1.setInsideSpacing(0);
    QCOMPARE(0, obj1.insideSpacing());
    obj1.setInsideSpacing(INT_MIN);
    QCOMPARE(INT_MIN, obj1.insideSpacing());
    obj1.setInsideSpacing(INT_MAX);
    QCOMPARE(INT_MAX, obj1.insideSpacing());
}

/*
    Test that a Q3GroupBox has a reasonable height compared to a QGroupBox.
*/
void tst_q3groupbox::groupBoxHeight()
{
    QWidget w;

    // Create group boxes.
    Q3GroupBox * const g3 = new Q3GroupBox(1000, Qt::Vertical, "Q3 Group Box", &w);
    new QLabel("Row 1", g3);

    QGroupBox * const g4 = new QGroupBox(&w, "QGroupBox");
    g4->setTitle("QGroupBox");
    QVBoxLayout * const g4Layout = new QVBoxLayout(g4);
    g4Layout->addWidget(new QLabel("QT4 Row 1"));

    // Add them to a layout.
    QVBoxLayout * const layout = new QVBoxLayout(&w, 5, 5);
    layout->addWidget(g3);
    layout->addWidget(g4);
    layout->addWidget(new QLabel("Label at Bottom"));
    w.show();

    // Measure height and test.
    const int q3height = g3->height();
    const int q4height = g4->height();

    const double withinReason = 0.5; // Up to 50% off is OK.
    const int minimum = int(q4height * (1.0 - withinReason));
    const int maximum = int(q4height * (1.0 + withinReason));

    QVERIFY(q3height > minimum);
    QVERIFY(q3height < maximum);
}

QTEST_MAIN(tst_q3groupbox)
#include "tst_q3groupbox.moc"

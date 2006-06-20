/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <Q3HBox>
#include <QPushButton>

class tst_Q3HBox : public QObject
{
    Q_OBJECT
public:
    tst_Q3HBox();


public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void setStretchFactor();
};

tst_Q3HBox::tst_Q3HBox()

{
}

void tst_Q3HBox::initTestCase()
{
}

void tst_Q3HBox::cleanupTestCase()
{
}

void tst_Q3HBox::setStretchFactor()
{
    Q3HBox box;

    int stretch1 = 500;
    QPushButton *b1 = new QPushButton(QString("Strech %1").arg(stretch1), &box);
    QVERIFY(box.setStretchFactor(b1, stretch1));

    int stretch2 = 1;
    QPushButton *b2 = new QPushButton(QString("Strech %1").arg(stretch2), &box);
    QVERIFY(box.setStretchFactor(b2, stretch2));
}

QTEST_MAIN(tst_Q3HBox)
#include "tst_q3hbox.moc"


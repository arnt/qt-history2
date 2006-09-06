/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScrollBar>
#include <QStyleOptionSlider>

class tst_QScrollBar : public QObject
{
    Q_OBJECT
public slots:
    void initTestCase();
    void cleanupTestCase();
    void hideAndShow(int action);

private slots:
    void scrollSingleStep();

private:
    QScrollBar *testWidget;
};

void tst_QScrollBar::initTestCase()
{
    testWidget = new QScrollBar(Qt::Horizontal);
    testWidget->resize(100, testWidget->height());
    testWidget->show();
}

void tst_QScrollBar::cleanupTestCase()
{
    delete testWidget;
    testWidget = 0;
}

void tst_QScrollBar::hideAndShow(int)
{
    testWidget->hide();
    testWidget->show();
}

// Check that the scrollbar doesn't scroll after calling hide and show
// from a slot connected to the scrollbar's actionTriggered signal.
void tst_QScrollBar::scrollSingleStep()
{
    testWidget->setValue(testWidget->minimum());
    QCOMPARE(testWidget->value(), testWidget->minimum());
    connect(testWidget, SIGNAL(actionTriggered(int)), this, SLOT(hideAndShow(int)));

    // Get rect for the area to click on
    const QStyleOptionSlider opt = qt_qscrollbarStyleOption(testWidget);
    QRect sr = testWidget->style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                                   QStyle::SC_ScrollBarAddLine, testWidget);

    QTest::mouseClick(testWidget, Qt::LeftButton, Qt::NoModifier, QPoint(sr.x(), sr.y()));
    QTest::qWait(510); // initial delay is 500 for setRepeatAction
    disconnect(testWidget, SIGNAL(actionTriggered(int)), 0, 0);
    QCOMPARE(testWidget->value(), testWidget->singleStep());
}

QTEST_MAIN(tst_QScrollBar)
#include "tst_qscrollbar.moc"

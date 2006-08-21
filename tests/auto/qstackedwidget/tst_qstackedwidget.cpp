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
#include <qstackedwidget.h>
#include <qpushbutton.h>
#include <QHBoxLayout>

//TESTED_CLASS=
//TESTED_FILES=qstackedwidget.h

class tst_QStackedWidget : public QObject
{
Q_OBJECT

public:
    tst_QStackedWidget();
    virtual ~tst_QStackedWidget();

private slots:
    void getSetCheck();
	void testMinimumSize();
};

tst_QStackedWidget::tst_QStackedWidget()
{
}

tst_QStackedWidget::~tst_QStackedWidget()
{
}

// Testing that stackedwidget respect the minimum size of it's contents (task 95319)
void tst_QStackedWidget::testMinimumSize()
{
	QWidget w;
    QStackedWidget sw(&w);
    QPushButton button("Text", &sw);
	sw.addWidget(&button);
    QHBoxLayout hboxLayout;
    hboxLayout.addWidget(&sw);
    w.setLayout(&hboxLayout);
    w.show();
	QVERIFY(w.minimumSize() != QSize(0, 0));
}

// Testing get/set functions
void tst_QStackedWidget::getSetCheck()
{
    QStackedWidget obj1;
    // int QStackedWidget::currentIndex()
    // void QStackedWidget::setCurrentIndex(int)
    obj1.setCurrentIndex(0);
    QCOMPARE(-1, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MIN);
    QCOMPARE(-1, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MAX);
    QCOMPARE(-1, obj1.currentIndex());

    // QWidget * QStackedWidget::currentWidget()
    // void QStackedWidget::setCurrentWidget(QWidget *)
    QWidget *var2 = new QWidget();
    obj1.addWidget(var2);
    obj1.setCurrentWidget(var2);
    QCOMPARE(var2, obj1.currentWidget());
#if QT_VERSION >= 0x040200
    // Layouts assert on any unknown widgets here, 0-pointers included.
    // This seems wrong behavior, since the setCurrentIndex(int), which
    // is really a convenience function for setCurrentWidget(QWidget*),
    // has no problem handling out-of-bounds indices.
    // ("convenience function" => "just another way of achieving the
    // same goal")
    QTest::ignoreMessage(QtWarningMsg, "QStackedWidget::setCurrentWidget: widget (nil) not contained in stack");
    obj1.setCurrentWidget((QWidget *)0);
    QCOMPARE(obj1.currentWidget(), var2);
#endif
    delete var2;
}

QTEST_MAIN(tst_QStackedWidget)
#include "tst_qstackedwidget.moc"

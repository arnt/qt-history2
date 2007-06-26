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
#include <qscrollarea.h>
#include <qlayout.h>

//TESTED_CLASS=
//TESTED_FILES=qscrollarea.h

class tst_QScrollArea : public QObject
{
Q_OBJECT

public:
    tst_QScrollArea();
    virtual ~tst_QScrollArea();

private slots:
    void getSetCheck();
    void ensureMicroFocusVisible_Task_167838();
};

tst_QScrollArea::tst_QScrollArea()
{
}

tst_QScrollArea::~tst_QScrollArea()
{
}

// Testing get/set functions
void tst_QScrollArea::getSetCheck()
{
    QScrollArea obj1;
    // QWidget * QScrollArea::widget()
    // void QScrollArea::setWidget(QWidget *)
    QWidget *var1 = new QWidget();
    obj1.setWidget(var1);
    QCOMPARE(var1, obj1.widget());
    obj1.setWidget((QWidget *)0);
    QCOMPARE(var1, obj1.widget()); // Cannot set a 0-widget. Old widget returned
    // delete var1; // No delete, since QScrollArea takes ownership

    // bool QScrollArea::widgetResizable()
    // void QScrollArea::setWidgetResizable(bool)
    obj1.setWidgetResizable(false);
    QCOMPARE(false, obj1.widgetResizable());
    obj1.setWidgetResizable(true);
    QCOMPARE(true, obj1.widgetResizable());
}

class WidgetWithMicroFocus : public QWidget
{
public:
    WidgetWithMicroFocus(QWidget *parent = 0) : QWidget(parent)
    {
        setBackgroundRole(QPalette::Dark);
    }
protected:
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const
    {
        if (query == Qt::ImMicroFocus)
            return QRect(width() / 2, height() / 2, 5, 5);
        return QWidget::inputMethodQuery(query);
    }
//     void paintEvent(QPaintEvent *event)
//     {
//         QPainter painter(this);
//         painter.fillRect(rect(), QBrush(Qt::red));
//     }
};

void tst_QScrollArea::ensureMicroFocusVisible_Task_167838()
{
    QScrollArea scrollArea;
    scrollArea.resize(100, 100);
    scrollArea.show();
    QWidget *parent = new QWidget;
    parent->setLayout(new QVBoxLayout);
    QWidget *child = new WidgetWithMicroFocus;
    parent->layout()->addWidget(child);
    parent->resize(300, 300); 
    scrollArea.setWidget(parent);
    scrollArea.ensureWidgetVisible(child, 10, 10);
    QRect microFocus = child->inputMethodQuery(Qt::ImMicroFocus).toRect();
    QPoint p = child->mapTo(scrollArea.viewport(), microFocus.topLeft());
    microFocus.translate(p - microFocus.topLeft());
    QVERIFY(scrollArea.viewport()->rect().contains(microFocus));
}

QTEST_MAIN(tst_QScrollArea)
#include "tst_qscrollarea.moc"

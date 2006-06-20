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
#include <qabstractscrollarea.h>
#include <qscrollarea.h>
#include <qscrollbar.h>
#include <qlabel.h>
#include <qwidget.h>


//TESTED_CLASS=
//TESTED_FILES=qabstractscrollarea.h

class tst_QAbstractScrollArea : public QObject
{
Q_OBJECT

public:
    tst_QAbstractScrollArea();
    virtual ~tst_QAbstractScrollArea();
private slots:
    void scrollBarWidgets();
    void setScrollBars();
};

tst_QAbstractScrollArea::tst_QAbstractScrollArea()
{
}

tst_QAbstractScrollArea::~tst_QAbstractScrollArea()
{
}

void tst_QAbstractScrollArea::scrollBarWidgets()
{
#if (QT_VERSION >= 0x040200)
    QWidget *w1 = new QWidget(0);
    QWidget *w2 = new QWidget(0);
    QWidget *w3 = new QWidget(0); 

    Qt::Alignment all = Qt::AlignLeft | Qt::AlignRight | Qt::AlignTop | Qt::AlignBottom;

    QWidgetList w1List = QWidgetList() << w1;
    QWidgetList w2List = QWidgetList() << w2;
    QWidgetList w3List = QWidgetList() << w3;
    
    QWidgetList w1w2List = w1List + w2List;
    QWidgetList allList = w1List + w2List + w3List;
    
    QAbstractScrollArea area;
    area.show();
    QCOMPARE(area.scrollBarWidgets(all), QWidgetList());

    area.addScrollBarWidget(w1, Qt::AlignLeft);
    QCOMPARE(area.scrollBarWidgets(all), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignLeft), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignRight), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignTop), QWidgetList()); 
    QCOMPARE(area.scrollBarWidgets(Qt::AlignBottom), QWidgetList());

    area.addScrollBarWidget(w2, Qt::AlignBottom);
    QCOMPARE(area.scrollBarWidgets(all), w1w2List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignLeft), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignRight), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignTop), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignBottom), w2List);

    // duplicate add
    area.addScrollBarWidget(w2, Qt::AlignBottom);
    QCOMPARE(area.scrollBarWidgets(all), w1w2List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignLeft), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignRight), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignTop), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignBottom), w2List);

    //reparent 
    w2->setParent(w1);
    QCOMPARE(area.scrollBarWidgets(all), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignLeft), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignRight), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignTop), QWidgetList()); 
    QCOMPARE(area.scrollBarWidgets(Qt::AlignBottom), QWidgetList());

    // add after reparent
    area.addScrollBarWidget(w2, Qt::AlignBottom);
    QCOMPARE(area.scrollBarWidgets(all), w1w2List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignLeft), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignRight), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignTop), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignBottom), w2List);

    // two widgets at Bottom.
    area.addScrollBarWidget(w3, Qt::AlignBottom);
    QCOMPARE(area.scrollBarWidgets(all).toSet(), allList.toSet());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignLeft), w1List);
    QCOMPARE(area.scrollBarWidgets(Qt::AlignRight), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignTop), QWidgetList());
    QCOMPARE(area.scrollBarWidgets(Qt::AlignBottom).toSet(), (w2List + w3List).toSet());

    //delete
    delete w1;
    delete w2;
    delete w3;

    QCOMPARE(area.scrollBarWidgets(all), QWidgetList());    
    QCOMPARE(area.scrollBarWidgets(Qt::AlignLeft), QWidgetList());    
    QCOMPARE(area.scrollBarWidgets(Qt::AlignRight), QWidgetList());    
    QCOMPARE(area.scrollBarWidgets(Qt::AlignTop), QWidgetList());    
    QCOMPARE(area.scrollBarWidgets(Qt::AlignBottom), QWidgetList());    
#endif
}

void tst_QAbstractScrollArea::setScrollBars()
{
#if (QT_VERSION >= 0x040200)
    QScrollArea scrollArea;
    scrollArea.resize(300, 300);
    scrollArea.show();
    
    QPointer<QScrollBar> vbar = scrollArea.verticalScrollBar();
    QPointer<QScrollBar> hbar = scrollArea.horizontalScrollBar();

    // Now set properties on the scroll bars
    scrollArea.verticalScrollBar()->setInvertedAppearance(true);
    scrollArea.verticalScrollBar()->setInvertedControls(true);
    scrollArea.verticalScrollBar()->setTracking(true);
    scrollArea.verticalScrollBar()->setRange(-100, 100);
    scrollArea.verticalScrollBar()->setPageStep(42);
    scrollArea.verticalScrollBar()->setSingleStep(3);
    scrollArea.verticalScrollBar()->setValue(43);
    scrollArea.horizontalScrollBar()->setInvertedAppearance(true);
    scrollArea.horizontalScrollBar()->setInvertedControls(true);
    scrollArea.horizontalScrollBar()->setTracking(true);
    scrollArea.horizontalScrollBar()->setRange(-100, 100);
    scrollArea.horizontalScrollBar()->setPageStep(42);
    scrollArea.horizontalScrollBar()->setSingleStep(3);
    scrollArea.horizontalScrollBar()->setValue(43);

    qApp->processEvents();

    // Then replace the scroll bars
    scrollArea.setVerticalScrollBar(new QScrollBar);
    scrollArea.setHorizontalScrollBar(new QScrollBar);

    // Check that the old ones were deleted
    QVERIFY(!vbar);
    QVERIFY(!hbar);

    qApp->processEvents();
    
    // Check that all properties have been populated
    QVERIFY(scrollArea.verticalScrollBar()->invertedAppearance());
    QVERIFY(scrollArea.verticalScrollBar()->invertedControls());
    QVERIFY(scrollArea.verticalScrollBar()->hasTracking());
    QVERIFY(scrollArea.verticalScrollBar()->isVisible());
    QCOMPARE(scrollArea.verticalScrollBar()->minimum(), -100);
    QCOMPARE(scrollArea.verticalScrollBar()->maximum(), 100);
    QCOMPARE(scrollArea.verticalScrollBar()->pageStep(), 42);
    QCOMPARE(scrollArea.verticalScrollBar()->singleStep(), 3);
    QCOMPARE(scrollArea.verticalScrollBar()->value(), 43);
    QVERIFY(scrollArea.horizontalScrollBar()->invertedAppearance());
    QVERIFY(scrollArea.horizontalScrollBar()->invertedControls());
    QVERIFY(scrollArea.horizontalScrollBar()->hasTracking());
    QVERIFY(scrollArea.horizontalScrollBar()->isVisible());
    QCOMPARE(scrollArea.horizontalScrollBar()->minimum(), -100);
    QCOMPARE(scrollArea.horizontalScrollBar()->maximum(), 100);
    QCOMPARE(scrollArea.horizontalScrollBar()->pageStep(), 42);
    QCOMPARE(scrollArea.horizontalScrollBar()->singleStep(), 3);
    QCOMPARE(scrollArea.horizontalScrollBar()->value(), 43);
#endif
}

QTEST_MAIN(tst_QAbstractScrollArea)
#include "tst_qabstractscrollarea.moc"

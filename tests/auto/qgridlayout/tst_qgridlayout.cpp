/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qlayout.h>
#include <qapplication.h>
#include <qwidget.h>

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qlayout.cpp gui/kernel/qlayout.h

class tst_QGridLayout : public QObject
{
Q_OBJECT

public:
    tst_QGridLayout();
    virtual ~tst_QGridLayout();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void getItemPosition();
    void badDistributionBug();
    void setMinAndMaxSize();
    void spacingAndSpacers();

private:
    QWidget *testWidget;
    QGridLayout *testLayout;
    QWidget *w1;
    QWidget *w2;
    QWidget *w3;
};


tst_QGridLayout::tst_QGridLayout()
{
}

tst_QGridLayout::~tst_QGridLayout()
{
}

void tst_QGridLayout::initTestCase()
{
    // Create the test class
    testWidget = new QWidget(0);

    testLayout = new QGridLayout(testWidget);

    w1 = new QWidget(testWidget);
    w1->setPalette(QPalette(Qt::red));
    testLayout->addWidget(w1, 0, 0);

    w2 = new QWidget(testWidget);
    testLayout->addWidget(w2, 1, 1, 2, 2);
    w2->setPalette(QPalette(Qt::green));

    w3 = new QWidget(testWidget);
    testLayout->addWidget(w3, 0, 1, 1, 2);
    w3->setPalette(QPalette(Qt::blue));


    testLayout->addItem(new QSpacerItem(4,4), 1, 3, 2, 1);

    testWidget->resize( 200, 200 );
    testWidget->show();
}

void tst_QGridLayout::cleanupTestCase()
{
    delete testWidget;
    testWidget = 0;
}

void tst_QGridLayout::init()
{
}

void tst_QGridLayout::cleanup()
{
}

void tst_QGridLayout::getItemPosition()
{
    QLayoutItem *item;
    int counter = 0;

    bool seenW1 = false;
    bool seenW2 = false;
    bool seenW3 = false;
    bool seenSpacer = false;

    while ((item = testLayout->itemAt(counter))) {
        QWidget *w = item->widget();
        int r,c,rs,cs;
        testLayout->getItemPosition(counter, &r, &c, &rs, &cs);

//        qDebug() << "item" << counter << "has" <<r << c << rs << cs;

        if (w == w1) {
            QVERIFY(!seenW1);
            seenW1 = true;
            QCOMPARE(r, 0);
            QCOMPARE(c, 0);
            QCOMPARE(rs, 1);
            QCOMPARE(cs, 1);
        } else if (w == w2) {
            QVERIFY(!seenW2);
            seenW2 = true;
            QCOMPARE(r, 1);
            QCOMPARE(c, 1);
            QCOMPARE(rs, 2);
            QCOMPARE(cs, 2);
        } else if (w == w3) {
            QVERIFY(!seenW3);
            seenW3 = true;
            QCOMPARE(r, 0);
            QCOMPARE(c, 1);
            QCOMPARE(rs, 1);
            QCOMPARE(cs, 2);
        } else {
            QVERIFY(!w);
            QVERIFY(!seenSpacer);
            seenSpacer = true;
            QCOMPARE(r, 1);
            QCOMPARE(c, 3);
            QCOMPARE(rs, 2);
            QCOMPARE(cs, 1);
        }
        ++counter;
    }
    QCOMPARE(counter, 4);
    QVERIFY(seenW1);
    QVERIFY(seenW2);
    QVERIFY(seenW3);
    QVERIFY(seenSpacer);
}

#include "ui_sortdialog.h"

void tst_QGridLayout::badDistributionBug()
{
    QDialog dialog;
    Ui::SortDialog ui;
    ui.setupUi(&dialog);
    ui.gridLayout->setMargin(0);
    ui.gridLayout->setSpacing(0);
    ui.vboxLayout->setMargin(0);
    ui.vboxLayout->setSpacing(0);
    ui.okButton->setFixedHeight(20);
    ui.moreButton->setFixedHeight(20);
    ui.primaryGroupBox->setFixedHeight(200);

    QSize minSize = dialog.layout()->minimumSize();
    QCOMPARE(minSize.height(), 200);
}

void tst_QGridLayout::setMinAndMaxSize()
{
    QWidget widget;
    QGridLayout layout(&widget);
    layout.setMargin(0);
    layout.setSpacing(0);
    layout.setSizeConstraint(QLayout::SetMinAndMaxSize);
    widget.show();

    QWidget leftChild;
    leftChild.setPalette(QPalette(Qt::red));
    leftChild.setMinimumSize(100, 100);
    leftChild.setMaximumSize(200, 200);
    layout.addWidget(&leftChild, 0, 0);
    QApplication::processEvents();
    QCOMPARE(widget.minimumSize(), leftChild.minimumSize());
    QCOMPARE(widget.maximumSize(), leftChild.maximumSize());

    QWidget rightChild;
    rightChild.setPalette(QPalette(Qt::green));
    rightChild.setMinimumSize(100, 100);
    rightChild.setMaximumSize(200, 200);
    layout.addWidget(&rightChild, 0, 2);
    QApplication::processEvents();

    QCOMPARE(widget.minimumWidth(),
             leftChild.minimumWidth() + rightChild.minimumWidth());
    QCOMPARE(widget.minimumHeight(),
             qMax(leftChild.minimumHeight(), rightChild.minimumHeight()));
    QCOMPARE(widget.maximumWidth(),
             leftChild.maximumWidth() + rightChild.maximumWidth());
    QCOMPARE(widget.maximumHeight(),
             qMax(leftChild.maximumHeight(), rightChild.maximumHeight()));


    static const int colMin = 100;
    layout.setColumnMinimumWidth(1, colMin);
    QCOMPARE(layout.columnMinimumWidth(1), colMin);

    QApplication::processEvents();
    QCOMPARE(widget.minimumWidth(),
             leftChild.minimumWidth() + rightChild.minimumWidth() + colMin);
    QCOMPARE(widget.maximumWidth(),
             leftChild.maximumWidth() + rightChild.maximumWidth() + colMin);
    QCOMPARE(widget.minimumHeight(),
             qMax(leftChild.minimumHeight(), rightChild.minimumHeight()));
    QCOMPARE(widget.maximumHeight(),
             qMax(leftChild.maximumHeight(), rightChild.maximumHeight()));



    layout.setColumnStretch(1,1);
    QApplication::processEvents();
    QCOMPARE(widget.minimumWidth(),
             leftChild.minimumWidth() + rightChild.minimumWidth() + colMin);
    QCOMPARE(widget.maximumWidth(), QLAYOUTSIZE_MAX);
    QCOMPARE(widget.minimumHeight(),
             qMax(leftChild.minimumHeight(), rightChild.minimumHeight()));
    QCOMPARE(widget.maximumHeight(),
             qMax(leftChild.maximumHeight(), rightChild.maximumHeight()));



    layout.setColumnStretch(1,0);
    QApplication::processEvents();
    QCOMPARE(widget.minimumWidth(),
             leftChild.minimumWidth() + rightChild.minimumWidth() + colMin);
    QCOMPARE(widget.maximumWidth(),
             leftChild.maximumWidth() + rightChild.maximumWidth() + colMin);
    QCOMPARE(widget.minimumHeight(),
             qMax(leftChild.minimumHeight(), rightChild.minimumHeight()));
    QCOMPARE(widget.maximumHeight(),
             qMax(leftChild.maximumHeight(), rightChild.maximumHeight()));



    layout.setColumnMinimumWidth(1, 0);

    static const int spacerS = 250;
    QSpacerItem *spacer = new QSpacerItem(spacerS, spacerS);
    layout.addItem(spacer, 0, 1);
    QApplication::processEvents();

    QCOMPARE(widget.minimumWidth(),
             leftChild.minimumWidth() + rightChild.minimumWidth() + spacerS);
    QCOMPARE(widget.maximumWidth(), QLAYOUTSIZE_MAX);
    QCOMPARE(widget.minimumHeight(),
             qMax(qMax(leftChild.minimumHeight(), rightChild.minimumHeight()), spacerS));
    QCOMPARE(widget.maximumHeight(),
             qMax(leftChild.maximumHeight(), rightChild.maximumHeight()));


    spacer->changeSize(spacerS, spacerS, QSizePolicy::Fixed, QSizePolicy::Minimum);
    layout.invalidate();
    QApplication::processEvents();
    QCOMPARE(widget.minimumWidth(),
             leftChild.minimumWidth() + rightChild.minimumWidth() + spacerS);
    QCOMPARE(widget.maximumWidth(),
             leftChild.maximumWidth() + rightChild.maximumWidth() + spacerS);


    layout.removeItem(spacer);

    rightChild.hide();
    QApplication::processEvents();
    QCOMPARE(widget.minimumSize(), leftChild.minimumSize());
    QCOMPARE(widget.maximumSize(), leftChild.maximumSize());

    rightChild.show();
    layout.removeWidget(&rightChild);
    QApplication::processEvents();
    QCOMPARE(widget.minimumSize(), leftChild.minimumSize());
    QCOMPARE(widget.maximumSize(), leftChild.maximumSize());

    QWidget bottomChild(&widget);
    bottomChild.setPalette(QPalette(Qt::green));
    bottomChild.setMinimumSize(100, 100);
    bottomChild.setMaximumSize(200, 200);
    layout.addWidget(&bottomChild, 1, 0);
    QApplication::processEvents();

    QCOMPARE(widget.minimumHeight(),
             leftChild.minimumHeight() + bottomChild.minimumHeight());
    QCOMPARE(widget.minimumWidth(),
             qMax(leftChild.minimumWidth(), bottomChild.minimumWidth()));
    QCOMPARE(widget.maximumHeight(),
             leftChild.maximumHeight() + bottomChild.maximumHeight());
    QCOMPARE(widget.maximumWidth(),
             qMax(leftChild.maximumWidth(), bottomChild.maximumWidth()));

    bottomChild.hide();
    QApplication::processEvents();
    QCOMPARE(widget.minimumSize(), leftChild.minimumSize());
    QCOMPARE(widget.maximumSize(), leftChild.maximumSize());

    bottomChild.show();
    layout.removeWidget(&bottomChild);
    QApplication::processEvents();
    QCOMPARE(widget.minimumSize(), leftChild.minimumSize());
    QCOMPARE(widget.maximumSize(), leftChild.maximumSize());
}


class SizeHinter : public QWidget
{
public:
    SizeHinter(QSize s) : sh(s) {}
    SizeHinter(int w, int h) : sh(QSize(w,h)) {}
    void setSizeHint(QSize s) { sh = s; }
    QSize sizeHint() const { return sh; }
private:
    QSize sh;
};

void tst_QGridLayout::spacingAndSpacers()
{
    QWidget widget;
    QGridLayout layout(&widget);
    layout.setMargin(0);
    layout.setSpacing(0);
    widget.show();

    QSize expectedSizeHint;

    SizeHinter leftChild(100,100);
    leftChild.setPalette(QPalette(Qt::red));
    layout.addWidget(&leftChild, 0, 0);
    QApplication::processEvents();
    expectedSizeHint = leftChild.sizeHint();
    QCOMPARE(widget.sizeHint(), expectedSizeHint);



    SizeHinter rightChild(200,100);
    rightChild.setPalette(QPalette(Qt::green));
    layout.addWidget(&rightChild, 0, 2);
    QApplication::processEvents();
    QCOMPARE(rightChild.sizeHint(), QSize(200,100));

    expectedSizeHint += QSize(rightChild.sizeHint().width(), 0);
    QCOMPARE(widget.sizeHint(), expectedSizeHint);

    layout.setColumnMinimumWidth(1, 100);
    widget.adjustSize();
    expectedSizeHint += QSize(100,0);
    QApplication::processEvents();
    QCOMPARE(widget.sizeHint(), expectedSizeHint);

    rightChild.hide();
    QApplication::processEvents();
    expectedSizeHint -= QSize(rightChild.sizeHint().width(), 0);
    QCOMPARE(widget.sizeHint(), expectedSizeHint);


    layout.setColumnMinimumWidth(1, 0);
    expectedSizeHint -= QSize(100, 0);
    QCOMPARE(widget.sizeHint(), expectedSizeHint);

    rightChild.show();

#if 0
    leftChild.setMaximumWidth(200);
    rightChild.setMaximumWidth(200);

    QApplication::processEvents();
    QCOMPARE(widget.maximumWidth(), leftChild.maximumWidth() + rightChild.maximumWidth());
#endif

    layout.removeWidget(&rightChild);
    QApplication::processEvents();
    QCOMPARE(widget.sizeHint(), expectedSizeHint);


}


QTEST_MAIN(tst_QGridLayout)
#include "tst_qgridlayout.moc"

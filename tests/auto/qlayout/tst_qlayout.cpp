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
#include <qboxlayout.h>
#include <qmenubar.h>
#include <QtGui/QFrame>
#include <QtGui/QWindowsStyle>
#include <QtGui/QSizePolicy>
#include <QPushButton>
#include <private/qlayoutengine_p.h>

//TESTED_CLASS=
//TESTED_FILES=qlayout.h

class tst_QLayout : public QObject
{
Q_OBJECT

public:
    tst_QLayout();
    virtual ~tst_QLayout();

private slots:
    void getSetCheck();
    void geometry();
    void smartMaxSize();
    void setLayoutBugs();
};

tst_QLayout::tst_QLayout()
{
}

tst_QLayout::~tst_QLayout()
{
}

// Testing get/set functions
void tst_QLayout::getSetCheck()
{
    QBoxLayout obj1(QBoxLayout::LeftToRight);
    // QWidget * QLayout::menuBar()
    // void QLayout::setMenuBar(QWidget *)
    QMenuBar *var1 = new QMenuBar();
    obj1.setMenuBar(var1);
    QCOMPARE(var1, obj1.menuBar());
    obj1.setMenuBar((QWidget *)0);
    QCOMPARE((QWidget *)0, obj1.menuBar());
    delete var1;
}

class SizeHinterFrame : public QFrame
{
public:
    SizeHinterFrame(const QSize &s) 
    : QFrame(0), sh(s) {
        setFrameStyle(QFrame::Box | QFrame::Plain);
    }



    void setSizeHint(const QSize &s) { sh = s; }
    QSize sizeHint() const { return sh; }

private:
    QSize sh;
};


void tst_QLayout::geometry()
{
    // For QWindowsStyle we know that QWidgetItem::geometry() and QWidget::geometry()
    // should be the same.
    QApplication::setStyle(new QWindowsStyle);
    QWidget w;
    QVBoxLayout layout(&w);
    SizeHinterFrame widget(QSize(100,100));
    layout.addWidget(&widget);
    QLayoutItem *item = layout.itemAt(0);
    w.show();
    QApplication::processEvents();
    QCOMPARE(item->geometry().size(), QSize(100,100));

    widget.setMinimumSize(QSize(110,110));
    QCOMPARE(item->geometry().size(), QSize(110,110));

    widget.setMinimumSize(QSize(0,0));
    widget.setMaximumSize(QSize(90,90));
    widget.setSizeHint(QSize(100,100));
    QCOMPARE(item->geometry().size(), QSize(90,90));
}

void tst_QLayout::smartMaxSize()
{
    QVector<int> expectedWidths; 

    QFile f(QLatin1String("baseline/smartmaxsize"));
    QCOMPARE(f.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QTextStream stream(&f);

    while(!stream.atEnd()) {
        QString line = stream.readLine(200);
        expectedWidths.append(line.section(QLatin1Char(' '), 6, -1, QString::SectionSkipEmpty).toInt());
    }
    f.close();

    int sizeCombinations[] = { 0, 10, 20, QWIDGETSIZE_MAX};
    QSizePolicy::Policy policies[] = {  QSizePolicy::Fixed, 
                                        QSizePolicy::Minimum, 
                                        QSizePolicy::Maximum, 
                                        QSizePolicy::Preferred,
                                        QSizePolicy::Expanding,
                                        QSizePolicy::MinimumExpanding,
                                        QSizePolicy::Ignored
                                        };
    Qt::Alignment alignments[] = {  0,
                                    Qt::AlignLeft,
                                    Qt::AlignRight,
                                    Qt::AlignHCenter
                                    };

    int expectedIndex = 0;
    int regressionCount = 0;
    for (int p = 0; p < sizeof(policies)/sizeof(QSizePolicy::Policy); ++p) {
        QSizePolicy sizePolicy;
        sizePolicy.setHorizontalPolicy(policies[p]);
        for (int min = 0; min < sizeof(sizeCombinations)/sizeof(int); ++min) {
            int minSize = sizeCombinations[min];
            for (int max = 0; max < sizeof(sizeCombinations)/sizeof(int); ++max) {
                int maxSize = sizeCombinations[max];
                for (int sh = 0; sh < sizeof(sizeCombinations)/sizeof(int); ++sh) {
                    int sizeHint = sizeCombinations[sh];
                    for (int a = 0; a < sizeof(alignments)/sizeof(int); ++a) {
                        Qt::Alignment align = alignments[a];
                        QSize sz = qSmartMaxSize(QSize(sizeHint, 1), QSize(minSize, 1), QSize(maxSize, 1), sizePolicy, align);
                        int width = sz.width();
#if 0
                        qDebug() << expectedIndex << sizePolicy.horizontalPolicy() << align << minSize << sizeHint << maxSize << width;
#else
                        int expectedWidth = expectedWidths[expectedIndex];
                        if (width != expectedWidth) {
                            qDebug() << "error at index" << expectedIndex << ":" << sizePolicy.horizontalPolicy() << align << minSize << sizeHint << maxSize << width;
                            ++regressionCount;
                        }
#endif
                        ++expectedIndex;
                    }
                }
            }
        }
    }
    QCOMPARE(regressionCount, 0);
}

void tst_QLayout::setLayoutBugs()
{
    QWidget widget(0);
    QHBoxLayout *hBoxLayout = new QHBoxLayout(&widget);

    for(int i = 0; i < 6; ++i) {
        QPushButton *pushButton = new QPushButton("Press me!", &widget);
        hBoxLayout->addWidget(pushButton);
    }

    widget.setLayout(hBoxLayout);
    QVERIFY(widget.layout() == hBoxLayout);

    QWidget containerWidget(0);
    containerWidget.setLayout(widget.layout());
    QVERIFY(widget.layout() == hBoxLayout);
    QVERIFY(containerWidget.layout() == 0);
}

QTEST_MAIN(tst_QLayout)
#include "tst_qlayout.moc"

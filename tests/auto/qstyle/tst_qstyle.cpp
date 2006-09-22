/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include "qstyle.h"
#include <qevent.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qstyleoption.h>
#include <qscrollbar.h>

#include <qplastiquestyle.h>
#include <qwindowsstyle.h>
#include <qcdestyle.h>
#include <qmotifstyle.h>
#include <qcommonstyle.h>

#if QT_VERSION >= 0x040200
#include <QCleanlooksStyle>
#endif

#ifdef Q_WS_MAC
#include <QMacStyle>
#endif

#ifdef Q_WS_WIN
#include <QWindowsXPStyle>
#endif

#include <qwidget.h>

//TESTED_CLASS=
//TESTED_FILES=gui/styles/qstyle.h gui/styles/qstyle.cpp

class tst_QStyle : public QObject
{
    Q_OBJECT
public:
    tst_QStyle();
    virtual ~tst_QStyle();
private:
    void testAllFunctions(QStyle *);
    void testScrollBarSubControls(QStyle *);
private slots:
    void drawItemPixmap();
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testMotifStyle();
    void testPlastiqueStyle();
    void testWindowsStyle();
    void testCDEStyle();
    void testWindowsXPStyle();
    void testCleanlooksStyle();
    void testMacStyle();

private:
    QWidget *testWidget;
};


tst_QStyle::tst_QStyle()
{
    testWidget = 0;
}

tst_QStyle::~tst_QStyle()
{
}

class MyWidget : public QWidget
{
public:
    MyWidget( QWidget* QWidget=0, const char* name=0 );
protected:
    void paintEvent( QPaintEvent* );
};

void tst_QStyle::init()
{
    testWidget = new MyWidget( 0, "testObject");
}

void tst_QStyle::cleanup()
{
    delete testWidget;
    testWidget = 0;
}

void tst_QStyle::initTestCase()
{
}

void tst_QStyle::cleanupTestCase()
{
}

void tst_QStyle::drawItemPixmap()
{
    testWidget->resize(300, 300);
    testWidget->show();
    QPixmap p("task_25863.png", "PNG");
    QPixmap actualPix = QPixmap::grabWidget(testWidget);
    QVERIFY(pixmapsAreEqual(&actualPix,&p));
    testWidget->hide();
}

void tst_QStyle::testAllFunctions(QStyle *style)
{
    QStyleOption opt;
    opt.init(testWidget);
    testWidget->setStyle(style);
    
    //Tests styleHint with default arguments for potential crashes
    for ( int hint = 0 ; hint < QStyle::SH_ScrollBar_RollBetweenButtons ; ++hint) {
        style->styleHint(QStyle::StyleHint(hint));
        style->styleHint(QStyle::StyleHint(hint), &opt, testWidget);
    }
    
    //Tests pixelMetric with default arguments for potential crashes
    for ( int pm = 0 ; pm < QStyle::PM_DockWidgetTitleMargin ; ++pm) {
        style->pixelMetric(QStyle::PixelMetric(pm));
        style->pixelMetric(QStyle::PixelMetric(pm), &opt, testWidget);        
    }

    //Tests drawControl with default arguments for potential crashes
    for ( int control = 0 ; control < QStyle::CE_ToolBar ; ++control) {
        QPixmap surface(QSize(200, 200));
        QPainter painter(&surface);
        style->drawControl(QStyle::ControlElement(control), &opt, &painter, 0);
    }
    
    //Check standard pixmaps/icons
    for ( int i = 0 ; i < QStyle::SP_ToolBarVerticalExtensionButton ; ++i) {
        QPixmap pixmap = style->standardPixmap(QStyle::StandardPixmap(i));
        if (pixmap.isNull()) {
            qWarning("missing StandardPixmap: %d", i);
        }
        QIcon icn = style->standardIcon(QStyle::StandardPixmap(i));
        if (icn.isNull()) {
            qWarning("missing StandardIcon: %d", i);
        }
    }
	
    style->itemPixmapRect(QRect(0, 0, 100, 100), Qt::AlignHCenter, QPixmap(200, 200));
	style->itemTextRect(QFontMetrics(qApp->font()), QRect(0, 0, 100, 100), Qt::AlignHCenter, true, QString("Test"));
    
    testScrollBarSubControls(style);
}

void tst_QStyle::testScrollBarSubControls(QStyle *)
{
    QScrollBar scrollBar;
    scrollBar.show();
    const QStyleOptionSlider opt = qt_qscrollbarStyleOption(&scrollBar);
    foreach (int subControl, QList<int>() << 1 << 2 << 4 << 8) {
        QRect sr = testWidget->style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                    QStyle::SubControl(subControl), &scrollBar);
        
        QVERIFY(sr.isNull() == false);
    }
}

void tst_QStyle::testPlastiqueStyle()
{
    QPlastiqueStyle pstyle;
    testAllFunctions(&pstyle);
}

void tst_QStyle::testCleanlooksStyle()
{
#if QT_VERSION >= 0x040200
    QCleanlooksStyle cstyle;
    testAllFunctions(&cstyle);
#endif
}

void tst_QStyle::testWindowsStyle()
{
    QWindowsStyle wstyle;
    testAllFunctions(&wstyle);
}

void tst_QStyle::testWindowsXPStyle()
{
#ifdef Q_WS_WIN
    QWindowsXPStyle xpstyle;
    testAllFunctions(&xpstyle);
#endif
}

void tst_QStyle::testMacStyle()
{
#ifdef Q_WS_MAC
    QMacStyle mstyle;
    testAllFunctions(&mstyle);
#endif
}

void tst_QStyle::testMotifStyle()
{
    QMotifStyle mstyle;
    testAllFunctions(&mstyle);
}

void tst_QStyle::testCDEStyle()
{
    QCDEStyle cstyle;
    testAllFunctions(&cstyle);
}

// Helper class...

MyWidget::MyWidget( QWidget* parent, const char* name )
    : QWidget( parent )
{
    setObjectName(name);
}

void MyWidget::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    QPixmap big(400,400);
    big.fill(Qt::green);
    style()->drawItemPixmap(&p, rect(), Qt::AlignCenter, big);
}

QTEST_MAIN(tst_QStyle)
#include "tst_qstyle.moc"

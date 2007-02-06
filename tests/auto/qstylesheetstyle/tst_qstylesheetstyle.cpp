#include <QtCore>
#include <QtGui>
#include <QtTest/QtTest>
#include <QtDebug>
#include <QMetaObject>

#include <private/qstylesheetstyle_p.h>

class tst_QStyleSheetStyle : public QObject
{
    Q_OBJECT
public:
    tst_QStyleSheetStyle();
    ~tst_QStyleSheetStyle();

private slots:
    void numinstances();
    void widgetsBeforeAppStyleSheet();
    void widgetsAfterAppStyleSheet();
    void applicationStyleSheet();
    void windowStyleSheet();
    void widgetStyleSheet();
    void reparentWithNoChildStyleSheet();
    void reparentWithChildStyleSheet();
    void repolish();

    void sharedStyle();
    void widgetStyle();
    void appStyle();

private:
    QColor COLOR(const QWidget& w) {
        w.ensurePolished();
        return w.palette().color(w.foregroundRole());
    }
    QColor APPCOLOR(const QWidget& w) {
        w.ensurePolished();
        return qApp->palette(&w).color(w.foregroundRole());
    }
    QColor BACKGROUND(const QWidget& w) {
        w.ensurePolished();
        return w.palette().color(w.backgroundRole());
    }
    QColor APPBACKGROUND(const QWidget& w) {
        w.ensurePolished();
        return qApp->palette(&w).color(w.backgroundRole());
    }

};

tst_QStyleSheetStyle::tst_QStyleSheetStyle()
{
}

tst_QStyleSheetStyle::~tst_QStyleSheetStyle()
{
}

void tst_QStyleSheetStyle::numinstances()
{
    QWidget w;
    QCommonStyle *style = new QCommonStyle;
    style->setParent(&w);
    QWidget c(&w);
    w.show();

    // set and unset application stylesheet
    QCOMPARE(QStyleSheetStyle::numinstances, 0);
    qApp->setStyleSheet("* { color: red; }");
    QCOMPARE(QStyleSheetStyle::numinstances, 1);
    qApp->setStyleSheet("");
    QCOMPARE(QStyleSheetStyle::numinstances, 0);

    // set and unset application stylesheet+widget
    qApp->setStyleSheet("* { color: red; }");
    w.setStyleSheet("color: red;");
    QCOMPARE(QStyleSheetStyle::numinstances, 2);
    w.setStyle(style);
    QCOMPARE(QStyleSheetStyle::numinstances, 2);
    qApp->setStyleSheet("");
    QCOMPARE(QStyleSheetStyle::numinstances, 1);
    w.setStyleSheet("");
    QCOMPARE(QStyleSheetStyle::numinstances, 0);

    // set and unset widget stylesheet
    w.setStyle(0);
    w.setStyleSheet("color: red");
    QCOMPARE(QStyleSheetStyle::numinstances, 1);
    c.setStyle(style);
    QCOMPARE(QStyleSheetStyle::numinstances, 2);
    w.setStyleSheet("");
    QCOMPARE(QStyleSheetStyle::numinstances, 0);
}

void tst_QStyleSheetStyle::widgetsBeforeAppStyleSheet()
{
    QPushButton w1; // widget with no stylesheet
    qApp->setStyleSheet("* { color: red; }");
    QVERIFY(COLOR(w1) == QColor("red"));
    w1.setStyleSheet("color: white");
    QVERIFY(COLOR(w1) == QColor("white"));
    qApp->setStyleSheet("");
    QVERIFY(COLOR(w1) == QColor("white"));
    w1.setStyleSheet("");
    QVERIFY(COLOR(w1) == APPCOLOR(w1));
}

void tst_QStyleSheetStyle::widgetsAfterAppStyleSheet()
{
    qApp->setStyleSheet("* { color: red; }");
    QPushButton w1;
    QVERIFY(COLOR(w1) == QColor("red"));
    w1.setStyleSheet("color: white");
    QVERIFY(COLOR(w1) == QColor("white"));
    w1.setStyleSheet("");
    QVERIFY(COLOR(w1) == QColor("red"));
    w1.setStyleSheet("color: white");
    QVERIFY(COLOR(w1) == QColor("white"));
    qApp->setStyleSheet("");
    QVERIFY(COLOR(w1) == QColor("white"));
    w1.setStyleSheet("");
    QVERIFY(COLOR(w1) == APPCOLOR(w1));
}

void tst_QStyleSheetStyle::applicationStyleSheet()
{
    QPushButton w1;
    qApp->setStyleSheet("* { color: red; }");
    QVERIFY(COLOR(w1) == QColor("red"));
    qApp->setStyleSheet("* { color: white; }");
    QVERIFY(COLOR(w1) == QColor("white"));
    qApp->setStyleSheet("");
    QVERIFY(COLOR(w1) == APPCOLOR(w1));
    qApp->setStyleSheet("* { color: red }");
    QVERIFY(COLOR(w1) == QColor("red"));
}

void tst_QStyleSheetStyle::windowStyleSheet()
{
    QPushButton w1;
    qApp->setStyleSheet("");
    w1.setStyleSheet("* { color: red; }");
    QVERIFY(COLOR(w1) == QColor("red"));
    w1.setStyleSheet("* { color: white; }");
    QVERIFY(COLOR(w1) == QColor("white"));
    w1.setStyleSheet("");
    QVERIFY(COLOR(w1) == APPCOLOR(w1));
    w1.setStyleSheet("* { color: red }");
    QVERIFY(COLOR(w1) == QColor("red"));

    qApp->setStyleSheet("* { color: green }");
    QVERIFY(COLOR(w1) == QColor("red"));
    w1.setStyleSheet("");
    QVERIFY(COLOR(w1) == QColor("green"));
    qApp->setStyleSheet("");
    QVERIFY(COLOR(w1) == APPCOLOR(w1));
}

void tst_QStyleSheetStyle::widgetStyleSheet()
{
    QPushButton w1;
    QPushButton *pb = new QPushButton(&w1);
    QPushButton &w2 = *pb;

    qApp->setStyleSheet("");
    w1.setStyleSheet("* { color: red }");
    QVERIFY(COLOR(w1) == QColor("red"));
    QVERIFY(COLOR(w2) == QColor("red"));

    w2.setStyleSheet("* { color: white }");
    QVERIFY(COLOR(w2) == QColor("white"));

    w1.setStyleSheet("* { color: blue }");
    QVERIFY(COLOR(w1) == QColor("blue"));
    QVERIFY(COLOR(w2) == QColor("white"));

    w1.setStyleSheet("");
    QVERIFY(COLOR(w1) == APPCOLOR(w1));
    QVERIFY(COLOR(w2) == QColor("white"));

    w2.setStyleSheet("");
    QVERIFY(COLOR(w1) == APPCOLOR(w1));
    QVERIFY(COLOR(w2) == APPCOLOR(w2));
}

void tst_QStyleSheetStyle::reparentWithNoChildStyleSheet()
{
    QPushButton p1, p2;
    QPushButton *pb = new QPushButton(&p1);
    QPushButton &c1 = *pb; // child with no stylesheet

    qApp->setStyleSheet("");
    p1.setStyleSheet("* { color: red }");
    QVERIFY(COLOR(c1) == QColor("red"));
    c1.setParent(&p2);
    QVERIFY(COLOR(c1) == APPCOLOR(c1));

    p2.setStyleSheet("* { color: white }");
    QVERIFY(COLOR(c1) == QColor("white"));

    c1.setParent(&p1);
    QVERIFY(COLOR(c1) == QColor("red"));

    qApp->setStyleSheet("* { color: blue }");
    c1.setParent(0);
    QVERIFY(COLOR(c1) == QColor("blue"));
    delete pb;
}

void tst_QStyleSheetStyle::reparentWithChildStyleSheet()
{
    qApp->setStyleSheet("");
    QPushButton p1, p2;
    QPushButton *pb = new QPushButton(&p1);
    QPushButton &c1 = *pb;

    c1.setStyleSheet("background: gray");
    QVERIFY(BACKGROUND(c1) == QColor("gray"));
    c1.setParent(&p2);
    QVERIFY(BACKGROUND(c1) == QColor("gray"));

    qApp->setStyleSheet("* { color: white }");
    c1.setParent(&p1);
    QVERIFY(BACKGROUND(c1) == QColor("gray"));
    QVERIFY(COLOR(c1) == QColor("white"));
}

void tst_QStyleSheetStyle::repolish()
{
    qApp->setStyleSheet("");
    QPushButton p1;
    p1.setStyleSheet("color: red; background: white");
    QVERIFY(BACKGROUND(p1) == QColor("white"));
    p1.setStyleSheet("background: white");
    QVERIFY(COLOR(p1) == APPCOLOR(p1));
    p1.setStyleSheet("color: red");
    QVERIFY(COLOR(p1) == QColor("red"));
    QVERIFY(BACKGROUND(p1) == APPBACKGROUND(p1));
    p1.setStyleSheet("");
    QVERIFY(COLOR(p1) == APPCOLOR(p1));
    QVERIFY(BACKGROUND(p1) == APPBACKGROUND(p1));
}

// Tests the feature that when the application has a stylesheet, all the widgets
// that have a stylesheet just use the application proxy (they dont create a proxy
// by themselves)
void tst_QStyleSheetStyle::sharedStyle()
{
#if QT_VERSION < 0x040300
    qApp->setStyleSheet("* { color: red }");

    // parent with no ss, child has ss
    {
    QWidget window;
    QPushButton *pb = new QPushButton(&window);
    pb->setStyleSheet("color: green");
    QVERIFY(window.style() == qApp->style());
    QVERIFY(pb->style() == qApp->style());
    }

    // parent with no ss, reparent a child with ss
    {
    QWidget window;
    QPushButton *pb = new QPushButton;
    QVERIFY(pb->style() == qApp->style());
    pb->setStyleSheet("color: red");
    QVERIFY(pb->style() == qApp->style());
    pb->setParent(&window);
    QVERIFY(pb->style() == qApp->style());
    }

    // parent with ss, child has ss
    {
    QWidget window;
    window.setStyleSheet("color: gray");
    QPushButton *pb = new QPushButton(&window);
    QVERIFY(pb->style() == qApp->style());
    pb->setStyleSheet("color: white");
    QVERIFY(pb->style() == qApp->style());
    }

    // parent with ss, reparent a child with ss
    {
    QWidget window;
    window.setStyleSheet("color: blue");
    QPushButton *pb = new QPushButton;
    QVERIFY(pb->style() == qApp->style());
    pb->setStyleSheet("color: red");
    QVERIFY(pb->style() == qApp->style());
    pb->setParent(&window);
    QVERIFY(pb->style() == qApp->style());
    }

    // parent with no ss, child with ss reparented out
    {
    QWidget window;
    QPushButton *pb = new QPushButton(&window);
    QVERIFY(pb->style() == qApp->style());
    pb->setStyleSheet("color: white");
    QVERIFY(pb->style() == qApp->style());
    pb->setParent(0);
    QVERIFY(pb->style() == qApp->style());
    }
#else
    QSKIP("QStyleSheetStyle:sharedStyle Widget's do not share application's style in Qt 4.3 and above", SkipAll);
#endif
}

void tst_QStyleSheetStyle::widgetStyle()
{
    QWidget *window1 = new QWidget;
    window1->setObjectName("window1");
    QWidget *widget1 = new QWidget(window1);
    widget1->setObjectName("widget1");
    QWidget *widget2 = new QWidget;
    widget2->setObjectName("widget2");
    QWidget *window2 = new QWidget;
    window2->setObjectName("window2");
    window1->ensurePolished();
    window2->ensurePolished();
    widget1->ensurePolished();
    widget2->ensurePolished();

    QWindowsStyle style1, style2;
    QPointer<QStyle> pstyle1 = &style1;
    QPointer<QStyle> pstyle2 = &style2;

    QStyle *appStyle = qApp->style();

    // Sanity: By default, a window inherits the application style
    QCOMPARE(appStyle, window1->style());

    // Setting a custom style on a widget
    window1->setStyle(&style1);
    QCOMPARE(&style1, window1->style());

    // Setting another style must not delete the older style
    window1->setStyle(&style2);
    QCOMPARE(&style2, window1->style());
    QVERIFY(!pstyle1.isNull()); // case we have not already crashed

    // Setting null style must make it follow the qApp style
    window1->setStyle(0);
    QCOMPARE(window1->style(), appStyle);
    QVERIFY(!pstyle2.isNull()); // case we have not already crashed
    QVERIFY(!pstyle2.isNull()); // case we have not already crashed

    // Sanity: Set the stylesheet
    window1->setStyleSheet(":x { }");

    QPointer<QStyleSheetStyle> proxy = (QStyleSheetStyle *)window1->style();
    QVERIFY(!proxy.isNull());
    QCOMPARE(proxy->metaObject()->className(), "QStyleSheetStyle"); // must be our proxy
    QVERIFY(proxy->base == 0); // and follows the application

    // Set the stylesheet
    window1->setStyle(&style1);
    QVERIFY(proxy.isNull()); // we create a new one each time
    proxy = (QStyleSheetStyle *)window1->style();
    QCOMPARE(proxy->metaObject()->className(), "QStyleSheetStyle"); // it is a proxy
    QCOMPARE(proxy->baseStyle(), &style1); // must have been replaced with the new one

    // Update the stylesheet and check nothing changes
    window1->setStyleSheet(":y { }");
    QCOMPARE(window1->style()->metaObject()->className(), "QStyleSheetStyle"); // it is a proxy
    QCOMPARE(proxy->baseStyle(), &style1); // the same guy

    // Remove the stylesheet
    proxy = (QStyleSheetStyle *)window1->style();
    window1->setStyleSheet("");
    QVERIFY(proxy.isNull()); // should have disappeared
    QCOMPARE(window1->style(), &style1); // its restored

    // Style Sheet existing children propagation
    window1->setStyleSheet(":z { }");
    proxy = (QStyleSheetStyle *)window1->style();
    QCOMPARE(proxy->metaObject()->className(), "QStyleSheetStyle");
    QCOMPARE(window1->style(), widget1->style()); // proxy must have propagated
    QCOMPARE(widget2->style(), appStyle); // widget2 is following the app style

    // Style Sheet automatic propagation to new children
    widget2->setParent(window1); // reparent in!
    QCOMPARE(window1->style(), widget2->style()); // proxy must have propagated

    // Style Sheet automatic removal from children who abandoned their parents
    window2->setStyle(&style2);
    widget2->setParent(window2); // reparent
    QCOMPARE(widget2->style(), appStyle); // widget2 is following the app style

    // Style Sheet propagation on a child widget with a custom style
    widget2->setStyle(&style1);
    window2->setStyleSheet(":x { }");
    proxy = (QStyleSheetStyle *)widget2->style();
    QCOMPARE(proxy->metaObject()->className(), "QStyleSheetStyle");
    QCOMPARE(proxy->baseStyle(), &style1);

    // Style Sheet propagation on a child widget with a custom style already set
    window2->setStyleSheet("");
    QCOMPARE(window2->style(), &style2);
    QCOMPARE(widget2->style(), &style1);
    widget2->setStyle(0);
    window2->setStyleSheet(":x { }");
    widget2->setStyle(&style1);
    proxy = (QStyleSheetStyle *)widget2->style();
    QCOMPARE(proxy->metaObject()->className(), "QStyleSheetStyle");

    // QApplication, QWidget both having a style sheet

    // clean everything out
    window1->setStyle(0);
    window1->setStyleSheet("");
    window2->setStyle(0);
    window2->setStyleSheet("");
    qApp->setStyle(0);

    qApp->setStyleSheet("may_insanity_prevail { }"); // app has styleshet
    QCOMPARE(window1->style(), qApp->style());
    QCOMPARE(window1->style()->metaObject()->className(), "QStyleSheetStyle");
    QCOMPARE(widget1->style()->metaObject()->className(), "QStyleSheetStyle"); // check the child
    window1->setStyleSheet("may_more_insanity_prevail { }"); // window has stylesheet
    QCOMPARE(window1->style()->metaObject()->className(), "QStyleSheetStyle"); // a new one
    QCOMPARE(widget1->style(), window1->style()); // child follows...
    proxy = (QStyleSheetStyle *) window1->style();
    QWindowsStyle *newStyle = new QWindowsStyle;
    qApp->setStyle(newStyle); // set a custom style on app
    proxy = (QStyleSheetStyle *) window1->style();
    QCOMPARE(proxy->baseStyle(), newStyle); // magic ;) the widget still follows the application
    QCOMPARE(static_cast<QStyle *>(proxy), widget1->style()); // child still follows...

    window1->setStyleSheet(""); // remove stylesheet
    QCOMPARE(window1->style(), qApp->style()); // is this cool or what
    QCOMPARE(widget1->style(), qApp->style()); // annoying child follows...
    QWindowsStyle wndStyle;
    window1->setStyle(&wndStyle);
    QCOMPARE(window1->style()->metaObject()->className(), "QStyleSheetStyle"); // auto wraps it
    QCOMPARE(widget1->style(), window1->style()); // and auto propagates to child
    qApp->setStyleSheet(""); // remove the app stylesheet
    QCOMPARE(window1->style(), &wndStyle); // auto dewrap
    QCOMPARE(widget1->style(), qApp->style()); // and child state is restored
    window1->setStyle(0); // let sanity prevail

    delete window1;
    delete widget2;
    delete window2;
}

void tst_QStyleSheetStyle::appStyle()
{
    // qApp style can never be 0
    QVERIFY(QApplication::style() != 0);
    QPointer<QStyle> style1 = new QWindowsStyle;
    QPointer<QStyle> style2 = new QWindowsStyle;
    qApp->setStyle(style1);
    // Basic sanity
    QVERIFY(qApp->style() == style1);
    qApp->setStyle(style2);
    QVERIFY(style1.isNull()); // qApp must have taken ownership and deleted it
    // Setting null should not crash
    qApp->setStyle(0);
    QVERIFY(qApp->style() == style2);

    // Set the stylesheet
    qApp->setStyleSheet("whatever");
    QPointer<QStyleSheetStyle> sss = (QStyleSheetStyle *)qApp->style();
    QVERIFY(!sss.isNull());
    QCOMPARE(sss->metaObject()->className(), "QStyleSheetStyle"); // must be our proxy now
    QVERIFY(!style2.isNull()); // this should exist as it is the base of the proxy
    QVERIFY(sss->baseStyle() == style2);
    style1 = new QWindowsStyle;
    qApp->setStyle(style1);
    QVERIFY(style2.isNull()); // should disappear automatically
    QVERIFY(sss.isNull()); // should disappear automatically

    // Update the stylesheet and check nothing changes
    sss = (QStyleSheetStyle *)qApp->style();
    qApp->setStyleSheet("whatever2");
    QVERIFY(qApp->style() == sss);
    QVERIFY(sss->baseStyle() == style1);

    // Revert the stylesheet
    qApp->setStyleSheet("");
    QVERIFY(sss.isNull()); // should have disappeared
    QVERIFY(qApp->style() == style1);

    qApp->setStyleSheet("");
    QVERIFY(qApp->style() == style1);
}


QTEST_MAIN(tst_QStyleSheetStyle)
#include "tst_qstylesheetstyle.moc"


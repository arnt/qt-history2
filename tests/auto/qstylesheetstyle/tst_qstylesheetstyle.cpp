#include <QtCore>
#include <QtGui>
#include <QtTest/QtTest>
#include <QtDebug>

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

QTEST_MAIN(tst_QStyleSheetStyle)
#include "tst_qstylesheetstyle.moc"


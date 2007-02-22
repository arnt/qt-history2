/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui>

#include <qmacstyle_mac.h>

#ifdef Q_OS_MAC

const int N = 1;

//TESTED_CLASS=
//TESTED_FILES=gui/styles/qmacstyle_mac.h gui/styles/qmacstyle_mac.cpp

enum Size { Normal, Small, Mini };

Q_DECLARE_METATYPE(Size);

#define CT(E) \
    static const ControlType E = QSizePolicy::E;

typedef QSizePolicy::ControlType ControlType;

CT(DefaultType)
CT(ButtonBox)
CT(CheckBox)
CT(ComboBox)
CT(Frame)
CT(GroupBox)
CT(Label)
CT(Line)
CT(LineEdit)
CT(PushButton)
CT(RadioButton)
CT(Slider)
CT(SpinBox)
CT(TabWidget)
CT(ToolButton)


class tst_QMacStyle : public QObject
{
    Q_OBJECT

public:
    tst_QMacStyle() { qRegisterMetaType<Size>("Size"); }

private slots:
    void sizeHints_data();
    void sizeHints();
    void layoutMargins_data();
    void layoutMargins();
    void layoutSpacings_data();
    void layoutSpacings();

private:
    static QSize msh(QWidget *widget);
    static QSize sh(QWidget *widget);
    static QRect geo(QWidget *widget);
    static QPoint pos(QWidget *widget) { return geo(widget).topLeft(); }
    static QSize size(QWidget *widget) { return geo(widget).size(); }
    static QSize gap(QWidget *widget1, QWidget *widget2);
    static int hgap(QWidget *widget1, QWidget *widget2) { return gap(widget1, widget2).width(); }
    static int vgap(QWidget *widget1, QWidget *widget2) { return gap(widget1, widget2).height(); }
    static void setSize(QWidget *widget, Size size);
    static int spacing(ControlType control1, ControlType control2, Qt::Orientation orientation,
                       QStyleOption *option = 0, QWidget *widget = 0);
    static int hspacing(ControlType control1, ControlType control2, Size size = Normal);
    static int vspacing(ControlType control1, ControlType control2, Size size = Normal);
};

#define SIZE(x, y, z) \
    ((size == Normal) ? (x) : (size == Small) ? (y) : (z))

void tst_QMacStyle::sizeHints_data()
{
    QTest::addColumn<Size>("size");
    QTest::newRow("normal") << Normal;
//    QTest::newRow("small") << Small;
//    QTest::newRow("mini") << Mini;
}

void tst_QMacStyle::sizeHints()
{
    QFETCH(Size, size);    
    QDialog w;
    setSize(&w, size);

    QLineEdit lineEdit1(&w);
    QCOMPARE(sh(&lineEdit1).height(), SIZE(22, 19, 16));    // 16 in Builder, 15 in AHIG

    QProgressBar progress1(&w);
    progress1.setOrientation(Qt::Horizontal);
    qDebug() << "sh" << progress1.sizeHint();
    QCOMPARE(sh(&progress1).height(), SIZE(16, 10, 10));   // Builder

    progress1.setOrientation(Qt::Vertical);
    QCOMPARE(sh(&progress1).width(), SIZE(16, 10, 10));   // Builder

    QRadioButton radio1("Radio", &w);
    QCOMPARE(sh(&radio1).height(), SIZE(15, 12, 10));   // Builder

    QCheckBox checkBox1("Switch", &w);
    QCOMPARE(sh(&checkBox1).height(), SIZE(14, 12, 10));   // Builder

    QComboBox comboBox1(&w);
    comboBox1.setEditable(false);
    comboBox1.addItem("Foo");
    QCOMPARE(sh(&comboBox1).height(), SIZE(20, 17, 15));

    QComboBox comboBox2(&w);
    comboBox2.setEditable(true);
    comboBox2.addItem("Foo");
    QCOMPARE(sh(&comboBox1).height(), SIZE(20, 17, 15));

    QSlider slider1(Qt::Horizontal, &w);
    QCOMPARE(sh(&slider1).height(), SIZE(15, 12, 10));

    slider1.setTickPosition(QSlider::TicksAbove);
    QCOMPARE(sh(&slider1).height(), SIZE(24, 17, 16));  // Builder

    slider1.setTickPosition(QSlider::TicksBelow);
    QCOMPARE(sh(&slider1).height(), SIZE(24, 17, 16));  // Builder

    slider1.setTickPosition(QSlider::TicksBothSides);
    QVERIFY(sh(&slider1).height() > SIZE(15, 12, 10));  // common sense

    QPushButton ok1("OK", &w);
    QPushButton cancel1("Cancel", &w);

    QSize s1 = sh(&ok1);
    if (size == Normal) {
        // AHIG says 68, Builder does 70, and Qt seems to do 69
        QVERIFY(s1.width() >= 68 && s1.width() <= 70);
    }
    QCOMPARE(s1.height(), SIZE(20, 17, 14));    // 14 in Builder, 15 in AHIG

    // Cancel should be identical to OK, no matter what
    QCOMPARE(s1, sh(&cancel1));

    // Play with auto-default and default
    cancel1.setAutoDefault(false);
    QCOMPARE(s1, sh(&cancel1));
    cancel1.setAutoDefault(true);
    QCOMPARE(s1, sh(&cancel1));
    cancel1.setDefault(true);
    QCOMPARE(s1, sh(&cancel1));

    QDialogButtonBox bbox(&w);
    bbox.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QCOMPARE(s1, sh(bbox.button(QDialogButtonBox::Ok)));
    QCOMPARE(s1, sh(bbox.button(QDialogButtonBox::Cancel)));

    QMessageBox mbox(&w);
    mbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    QCOMPARE(s1, sh(mbox.button(QMessageBox::Ok)));
    QCOMPARE(s1, sh(mbox.button(QMessageBox::Cancel)));

    /*
    QSpinBox spinBox1(&w);
    int h1 = sh(&spinBox1).height();
    QCOMPARE(h1, SIZE(22, 19, 15));

    QDateEdit date1(&w);
    QCOMPARE(sh(&date1).height(), h1);

    QTimeEdit time1(&w);
    QCOMPARE(sh(&time1).height(), h1);

    QDateTimeEdit dateTime1(&w);
    QCOMPARE(sh(&dateTime1).height(), h1);

    ok1.setAttribute(Qt::WA_MacMetalStyle, true);
    QSize s2 = sh(&ok1);
    if (size == Normal) {
        QVERIFY(s2.height() >= 21 && s2.height() <= 32);
    } else {
        QVERIFY(s2.height() >= 18 && s2.height() <= 24);
    }
    */

    // QMacStyle bug: label doesn't react to Small and Mini
    QLabel label1("Blah", &w);
    QCOMPARE(sh(&label1).height(), SIZE(17, 14, 11));
}

void tst_QMacStyle::layoutMargins_data()
{
    tst_QMacStyle::sizeHints_data();
}

void tst_QMacStyle::layoutMargins()
{
    QFETCH(Size, size);    
    QWidget w;
    setSize(&w, size);

}

void tst_QMacStyle::layoutSpacings_data()
{
    tst_QMacStyle::sizeHints_data();
}

void tst_QMacStyle::layoutSpacings()
{
    QFETCH(Size, size);

    /*
        Constraints specified by AHIG.
    */

    for (int i = 0; i < 4; ++i) {
        ControlType c1 = (i % 2 == 0) ? PushButton : ButtonBox;
        ControlType c2 = (i / 2 == 0) ? PushButton : ButtonBox;
        QCOMPARE(hspacing(c1, c2, size), SIZE(12, 10, 8));
        QCOMPARE(vspacing(c1, c2, size), SIZE(12, 10, 8));
    }

    QCOMPARE(hspacing(Label, RadioButton, size), SIZE(8, 6, 5));
    QCOMPARE(vspacing(RadioButton, RadioButton, size), SIZE(5, 5, 5));  // Builder, guess, AHIG

    QCOMPARE(hspacing(Label, CheckBox, size), SIZE(8, 6, 5));
    QCOMPARE(vspacing(CheckBox, CheckBox, size), SIZE(8, 8, 7));

    QCOMPARE(hspacing(Label, ComboBox, size), SIZE(8, 6, 5));

    QCOMPARE(hspacing(LineEdit, LineEdit, size), SIZE(10, 8, 8));

    /*
        Common sense constraints, for when AHIG makes no sense (e.g., disagrees
        too much with Builder or looks bad).
    */

    // Comboboxes are a special pain, because AHIG and Builder can't agree,
    // and because they can be editable or not, with two totally different looks
    QVERIFY(vspacing(ComboBox, ComboBox, size) >= SIZE(8, 6, 5));
    QVERIFY(vspacing(ComboBox, ComboBox, size) <= SIZE(12, 10, 8));

    // Make sure button boxes get the respect they deserve, when they occur
    // in the bottom or right side of a dialog
    QCOMPARE(hspacing(ButtonBox, LineEdit), SIZE(12, 10, 8));
    QCOMPARE(vspacing(ButtonBox, LineEdit), SIZE(12, 10, 8));

    QCOMPARE(hspacing(LineEdit, ButtonBox), SIZE(20, 10, 10));
    QCOMPARE(vspacing(LineEdit, ButtonBox), SIZE(20, 10, 10));
}

// helper functions

QSize tst_QMacStyle::msh(QWidget *widget)
{
    QWidgetItem item(widget);
    return item.sizeHint();
}

QSize tst_QMacStyle::sh(QWidget *widget)
{
    QWidgetItem item(widget);
    return item.sizeHint();
}

QRect tst_QMacStyle::geo(QWidget *widget)
{
    QWidgetItem item(widget);
    return item.geometry();
}

QSize tst_QMacStyle::gap(QWidget *widget1, QWidget *widget2)
{
    QPoint d = pos(widget2) - pos(widget1);
    QSize s = size(widget1);
    return s + QSize(d.x(), d.y());
}

void tst_QMacStyle::setSize(QWidget *widget, Size size)
{
    switch (size) {
    case Normal:
        QMacStyle::setWidgetSizePolicy(widget, QMacStyle::SizeDefault);
        break;
    case Small:
        QMacStyle::setWidgetSizePolicy(widget, QMacStyle::SizeSmall);
        break;
    case Mini:
        QMacStyle::setWidgetSizePolicy(widget, QMacStyle::SizeMini);
    }
}

int tst_QMacStyle::spacing(ControlType control1, ControlType control2, Qt::Orientation orientation,
                           QStyleOption *option, QWidget *widget)
{
    return QApplication::style()->layoutSpacing(control1, control2, orientation, option, widget);
}

int tst_QMacStyle::hspacing(ControlType control1, ControlType control2, Size size)
{
    QWidget w;
    setSize(&w, size);

    QStyleOption opt;
    opt.initFrom(&w);

    return spacing(control1, control2, Qt::Horizontal, &opt);
}

int tst_QMacStyle::vspacing(ControlType control1, ControlType control2, Size size)
{
    QWidget w;
    setSize(&w, size);

    QStyleOption opt;
    opt.initFrom(&w);

    return spacing(control1, control2, Qt::Vertical, &opt);
}

QTEST_MAIN(tst_QMacStyle)

#else

QTEST_NOOP_MAIN

#endif

#include "tst_qmacstyle.moc"


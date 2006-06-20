/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QStyleOption>


class tst_QStyleOption: public QObject 
{
    Q_OBJECT

private slots:
    void qstyleoptioncast_data();
    void qstyleoptioncast();
    void copyconstructors();
};

// Just a simple container for QStyleOption-pointer 
struct StyleOptionPointerBase
{
    QStyleOption *pointer;

    StyleOptionPointerBase(QStyleOption *p = 0) : pointer(p) { }

    virtual ~StyleOptionPointerBase() { pointer = 0; }
};

template <typename T>
struct StyleOptionPointer: public StyleOptionPointerBase
{
    StyleOptionPointer(T *p = 0): StyleOptionPointerBase(p) {}
    ~StyleOptionPointer() { delete static_cast<T *>(pointer); pointer = 0; }
};

Q_DECLARE_METATYPE(StyleOptionPointerBase*)

template <typename T>
inline StyleOptionPointerBase *stylePtr(T *ptr) { return new StyleOptionPointer<T>(ptr); }

void tst_QStyleOption::qstyleoptioncast_data()
{
    QTest::addColumn<StyleOptionPointerBase *>("testOption");
    QTest::addColumn<bool>("canCastToComplex");

    QTest::newRow("optionDefault") << stylePtr(new QStyleOption) << false;
    QTest::newRow("optionButton") << stylePtr(new QStyleOptionButton) << false;
    QTest::newRow("optionComboBox") << stylePtr(new QStyleOptionComboBox) << true;
    QTest::newRow("optionComplex") << stylePtr(new QStyleOptionComplex) << true;
    QTest::newRow("optionDockWidget") << stylePtr(new QStyleOptionDockWidget) << false;
    QTest::newRow("optionFocusRect") << stylePtr(new QStyleOptionFocusRect) << false;
    QTest::newRow("optionFrame") << stylePtr(new QStyleOptionFrame) << false;
    QTest::newRow("optionHeader") << stylePtr(new QStyleOptionHeader) << false;
    QTest::newRow("optionMenuItem") << stylePtr(new QStyleOptionMenuItem) << false;
    QTest::newRow("optionProgressBar") << stylePtr(new QStyleOptionProgressBar) << false;
    QTest::newRow("optionQ3DockWindow") << stylePtr(new QStyleOptionQ3DockWindow) << false;
    QTest::newRow("optionQ3ListView") << stylePtr(new QStyleOptionQ3ListView) << true;
    QTest::newRow("optionQ3ListViewItem") << stylePtr(new QStyleOptionQ3ListViewItem) << false;
    QTest::newRow("optionSlider") << stylePtr(new QStyleOptionSlider) << true;
    QTest::newRow("optionSpinBox") << stylePtr(new QStyleOptionSpinBox) << true;
    QTest::newRow("optionTab") << stylePtr(new QStyleOptionTab) << false;
    QTest::newRow("optionTitleBar") << stylePtr(new QStyleOptionTitleBar) << true;
    QTest::newRow("optionToolBox") << stylePtr(new QStyleOptionToolBox) << false;
    QTest::newRow("optionToolButton") << stylePtr(new QStyleOptionToolButton) << true;
    QTest::newRow("optionViewItem") << stylePtr(new QStyleOptionViewItem) << false;
}

void tst_QStyleOption::qstyleoptioncast()
{
    QFETCH(StyleOptionPointerBase *, testOption);
    QFETCH(bool, canCastToComplex);

    QVERIFY(testOption->pointer != 0);

    // Cast to common base class
    QStyleOption *castOption = qstyleoption_cast<QStyleOption*>(testOption->pointer);
    QVERIFY(castOption != 0);

    // Cast to complex base class
    castOption = qstyleoption_cast<QStyleOptionComplex*>(testOption->pointer);
    QCOMPARE(canCastToComplex, (castOption != 0));

    // Cast to combo box
    castOption = qstyleoption_cast<QStyleOptionComboBox*>(testOption->pointer);
    QCOMPARE((castOption != 0),(testOption->pointer->type == QStyleOption::SO_ComboBox));

    // Cast to button
    castOption = qstyleoption_cast<QStyleOptionButton*>(testOption->pointer);
    QCOMPARE((castOption != 0),(testOption->pointer->type == QStyleOption::SO_Button));

    // Cast to lower version
    testOption->pointer->version += 1;
    castOption = qstyleoption_cast<QStyleOption*>(testOption->pointer);
    QVERIFY(castOption);

    // Cast a null pointer
    castOption = qstyleoption_cast<QStyleOption*>((QStyleOption*)0);
    QCOMPARE(castOption,(QStyleOption*)0);

    // Deallocate
    delete testOption;
}

void tst_QStyleOption::copyconstructors()
{
    QStyleOptionFrame frame;
    QStyleOptionFrameV2 frame2(frame);
    QCOMPARE(frame2.version, int(QStyleOptionFrameV2::Version));
    frame2 = frame;
    QCOMPARE(frame2.version, int(QStyleOptionFrameV2::Version));

    QStyleOptionProgressBar bar;
    QStyleOptionProgressBarV2 bar2(bar);
    QCOMPARE(bar2.version, int(QStyleOptionProgressBarV2::Version));
    bar2 = bar;
    QCOMPARE(bar2.version, int(QStyleOptionProgressBarV2::Version));
}

QTEST_MAIN(tst_QStyleOption)
#include "tst_qstyleoption.moc"


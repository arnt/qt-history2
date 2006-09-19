/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QLineEdit>
#include <QStyle>
#include <QStyleOptionGroupBox>
#include <QVBoxLayout>
#include <QRadioButton>

#include "qgroupbox.h"

//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qgroupbox.h gui/widgets/qgroupbox.cpp

class tst_QGroupBox : public QObject
{
    Q_OBJECT

public:
    tst_QGroupBox();
    virtual ~tst_QGroupBox();

public slots:
    void toggledHelperSlot(bool on);
    void init();
    void clickTimestampSlot();
    void toggleTimestampSlot();

private slots:
    void setTitle_data();
    void setTitle();
    void setCheckable_data();
    void setCheckable();
    void setChecked_data();
    void setChecked();
    void enabledPropagation();
    void sizeHint();
    void toggled();
    void clicked_data();
    void clicked();
    void toggledVsClicked();
    void childrenAreDisabled();
    
private:
    bool checked;
    qint64 timeStamp;
    qint64 clickTimeStamp;
    qint64 toggleTimeStamp;

};

tst_QGroupBox::tst_QGroupBox()
{
    checked = true;
}

tst_QGroupBox::~tst_QGroupBox()
{

}

void tst_QGroupBox::init()
{
    checked = true;
}

void tst_QGroupBox::setTitle_data()
{
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("expectedTitle");
    QTest::newRow( "empty_title" ) << QString("") << QString("");
    QTest::newRow( "normal_title" ) << QString("Whatisthematrix") << QString("Whatisthematrix");
    QTest::newRow( "special_chars_title" ) << QString("<>%&#/()=") << QString("<>%&#/()=");
    QTest::newRow( "spaces_title" ) << QString("  Hello  ") <<  QString("  Hello  ");
}

void tst_QGroupBox::setCheckable_data()
{
    QTest::addColumn<bool>("checkable");
    QTest::addColumn<bool>("expectedCheckable");
    QTest::newRow( "checkable_true" ) << TRUE << TRUE;
    QTest::newRow( "checkable_false" ) << FALSE << FALSE;
}

void tst_QGroupBox::setChecked_data()
{
    QTest::addColumn<bool>("checkable");
    QTest::addColumn<bool>("checked");
    QTest::addColumn<bool>("expectedChecked");
    QTest::newRow( "checkable_false_checked_true" ) << FALSE << TRUE << FALSE;
    QTest::newRow( "checkable_true_checked_true" ) << TRUE << TRUE << TRUE;
    QTest::newRow( "checkable_true_checked_false" ) << TRUE << FALSE << FALSE;
}

void tst_QGroupBox::setTitle()
{
    QFETCH( QString, title );
    QFETCH( QString, expectedTitle );

    QGroupBox groupBox;

    groupBox.setTitle( title );

    QCOMPARE( groupBox.title() , expectedTitle );
}

void tst_QGroupBox::setCheckable()
{
    QFETCH( bool, checkable );
    QFETCH( bool, expectedCheckable );

    QGroupBox groupBox;

    groupBox.setCheckable( checkable );
    QCOMPARE( groupBox.isCheckable() , expectedCheckable );
}


void tst_QGroupBox::setChecked()
{
    QFETCH( bool, checkable );
    QFETCH( bool, checked );
    QFETCH( bool, expectedChecked );

    QGroupBox groupBox;

    groupBox.setCheckable( checkable );
    groupBox.setChecked( checked );
    QCOMPARE( groupBox.isChecked(), expectedChecked );
}

void tst_QGroupBox::enabledPropagation()
{
    QGroupBox *testWidget = new QGroupBox(0);
    testWidget->setCheckable(TRUE);
    testWidget->setChecked(TRUE);
    QWidget* childWidget = new QWidget( testWidget );
    childWidget->show();
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );

    testWidget->setEnabled( FALSE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );

    testWidget->setDisabled( FALSE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );

    QWidget* grandChildWidget = new QWidget( childWidget );
    QVERIFY( grandChildWidget->isEnabled() );

    testWidget->setDisabled( TRUE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( FALSE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( TRUE );
    testWidget->setEnabled( FALSE );
    childWidget->setDisabled( TRUE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    // Reset state
    testWidget->setEnabled( TRUE );
    childWidget->setEnabled( TRUE );
    grandChildWidget->setEnabled( TRUE );

    // Now check when it's disabled
    testWidget->setChecked(FALSE);
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );

    testWidget->setEnabled( FALSE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );

    testWidget->setDisabled( FALSE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );

    QVERIFY( !grandChildWidget->isEnabled() );

    testWidget->setDisabled( TRUE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( FALSE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( TRUE );
    testWidget->setEnabled( FALSE );
    childWidget->setDisabled( TRUE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    // Reset state
    testWidget->setEnabled( TRUE );
    childWidget->setEnabled( TRUE );
    grandChildWidget->setEnabled( TRUE );

    // Finally enable it again
    testWidget->setChecked(TRUE);
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );

    testWidget->setEnabled( FALSE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );

    testWidget->setDisabled( FALSE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );
    QVERIFY( grandChildWidget->isEnabled() );

    testWidget->setDisabled( TRUE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( FALSE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( TRUE );
    testWidget->setEnabled( FALSE );
    childWidget->setDisabled( TRUE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    delete testWidget;
}


void tst_QGroupBox::sizeHint()
{
    QGroupBox testWidget1(0);
    testWidget1.setTitle("&0&0&0&0&0&0&0&0&0&0");

    QGroupBox testWidget2(0);
    testWidget2.setTitle("0000000000");

    QCOMPARE(testWidget1.sizeHint().width(), testWidget2.sizeHint().width());

    // if the above fails one should maybe test to see like underneath.
    // QVERIFY((QABS(testWidget1->sizeHint().width() - testWidget2->sizeHint().width()) < 10));
}

void tst_QGroupBox::toggledHelperSlot(bool on)
{
    checked = on;
}


void tst_QGroupBox::toggled()
{
    QGroupBox testWidget1(0);
    testWidget1.setCheckable(true);
    connect(&testWidget1, SIGNAL(toggled(bool)), this, SLOT(toggledHelperSlot(bool)));
    QLineEdit *edit = new QLineEdit(&testWidget1);
    QVERIFY(checked);
    testWidget1.setChecked(true);
    QVERIFY(checked);
    QVERIFY(edit->isEnabled());
    testWidget1.setChecked(false);
    QVERIFY(!checked);
    QVERIFY(!edit->isEnabled());
}

void tst_QGroupBox::clicked_data()
{
    QTest::addColumn<bool>("checkable");
    QTest::addColumn<bool>("initialCheck");
    QTest::addColumn<int>("areaToHit");
    QTest::addColumn<int>("clickedCount");
    QTest::addColumn<bool>("finalCheck");

    QTest::newRow("hit nothing, not checkable") << false << false << int(QStyle::SC_None) << 0 << false;
    QTest::newRow("hit frame, not checkable") << false << false << int(QStyle::SC_GroupBoxFrame) << 0 << false;
    QTest::newRow("hit content, not checkable") << false << false << int(QStyle::SC_GroupBoxContents) << 0 << false;
    QTest::newRow("hit label, not checkable") << false << false << int(QStyle::SC_GroupBoxLabel) << 0 << false;
    QTest::newRow("hit checkbox, not checkable") << false << false << int(QStyle::SC_GroupBoxCheckBox) << 0 << false;

    QTest::newRow("hit nothing, checkable") << true << true << int(QStyle::SC_None) << 0 << true;
    QTest::newRow("hit frame, checkable") << true << true << int(QStyle::SC_GroupBoxFrame) << 0 << true;
    QTest::newRow("hit content, checkable") << true << true << int(QStyle::SC_GroupBoxContents) << 0 << true;
    QTest::newRow("hit label, checkable") << true << true << int(QStyle::SC_GroupBoxLabel) << 1 << false;
    QTest::newRow("hit checkbox, checkable") << true << true << int(QStyle::SC_GroupBoxCheckBox) << 1 << false;

    QTest::newRow("hit nothing, checkable, but unchecked") << true << false << int(QStyle::SC_None) << 0 << false;
    QTest::newRow("hit frame, checkable, but unchecked") << true << false << int(QStyle::SC_GroupBoxFrame) << 0 << false;
    QTest::newRow("hit content, checkable, but unchecked") << true << false << int(QStyle::SC_GroupBoxContents) << 0 << false;
    QTest::newRow("hit label, checkable, but unchecked") << true << false << int(QStyle::SC_GroupBoxLabel) << 1 << true;
    QTest::newRow("hit checkbox, checkable, but unchecked") << true << false << int(QStyle::SC_GroupBoxCheckBox) << 1 << true;
}

void tst_QGroupBox::clicked()
{
    QFETCH(bool, checkable);
    QFETCH(bool, initialCheck);
    QFETCH(int, areaToHit);
    QGroupBox testWidget(QLatin1String("Testing Clicked"));
    testWidget.setCheckable(checkable);
    testWidget.setChecked(initialCheck);
    QCOMPARE(testWidget.isChecked(), initialCheck);
    testWidget.resize(200, 200);
    QSignalSpy spy(&testWidget, SIGNAL(clicked(bool)));

    QStyleOptionGroupBox option;
    option.initFrom(&testWidget);
    option.subControls = checkable ? QStyle::SubControls(QStyle::SC_All) : QStyle::SubControls(QStyle::SC_All & ~QStyle::SC_GroupBoxCheckBox);
    option.text = testWidget.title();
    option.textAlignment = testWidget.alignment();

    QRect rect = testWidget.style()->subControlRect(QStyle::CC_GroupBox, &option,
                                                    QStyle::SubControl(areaToHit), &testWidget);

    if (rect.isValid())
        QTest::mouseClick(&testWidget, Qt::LeftButton, 0, rect.center());
    else
        QTest::mouseClick(&testWidget, Qt::LeftButton);

    QTEST(spy.count(), "clickedCount");
    if (spy.count() > 0)
        QTEST(spy.at(0).at(0).toBool(), "finalCheck");
    QTEST(testWidget.isChecked(), "finalCheck");
}

void tst_QGroupBox::toggledVsClicked()
{
    timeStamp = clickTimeStamp = toggleTimeStamp = 0;
    QGroupBox groupBox;
    groupBox.setCheckable(true);
    QSignalSpy toggleSpy(&groupBox, SIGNAL(toggled(bool)));
    QSignalSpy clickSpy(&groupBox, SIGNAL(clicked(bool)));

    groupBox.setChecked(!groupBox.isChecked());
    QCOMPARE(clickSpy.count(), 0);
    QCOMPARE(toggleSpy.count(), 1);
    if (toggleSpy.count() > 0)
        QCOMPARE(toggleSpy.at(0).at(0).toBool(), groupBox.isChecked());

    connect(&groupBox, SIGNAL(clicked(bool)), this, SLOT(clickTimestampSlot()));
    connect(&groupBox, SIGNAL(toggled(bool)), this, SLOT(toggleTimestampSlot()));

    QStyleOptionGroupBox option;
    option.initFrom(&groupBox);
    option.subControls = QStyle::SubControls(QStyle::SC_All);
    QRect rect = groupBox.style()->subControlRect(QStyle::CC_GroupBox, &option,
                                                  QStyle::SC_GroupBoxCheckBox, &groupBox);

    QTest::mouseClick(&groupBox, Qt::LeftButton, 0, rect.center());
    QCOMPARE(clickSpy.count(), 1);
    QCOMPARE(toggleSpy.count(), 2);
    QVERIFY(toggleTimeStamp < clickTimeStamp);
}

void tst_QGroupBox::clickTimestampSlot()
{
    clickTimeStamp = ++timeStamp;
}

void tst_QGroupBox::toggleTimestampSlot()
{
    toggleTimeStamp = ++timeStamp;
}

void tst_QGroupBox::childrenAreDisabled()
{
    QGroupBox box;
    box.setCheckable(true);
    box.setChecked(false);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QRadioButton);
    layout->addWidget(new QRadioButton);
    layout->addWidget(new QRadioButton);
    box.setLayout(layout);

    foreach (QObject *object, box.children()) {
        if (QWidget *widget = qobject_cast<QWidget *>(object)) {
            QVERIFY(!widget->isEnabled());
            QVERIFY(!widget->testAttribute(Qt::WA_ForceDisabled));
        }
    }

    box.setChecked(true);
    foreach (QObject *object, box.children()) {
        if (QWidget *widget = qobject_cast<QWidget *>(object)) {
            QVERIFY(widget->isEnabled());
            QVERIFY(!widget->testAttribute(Qt::WA_ForceDisabled));
        }
    }

    box.setChecked(false);
    foreach (QObject *object, box.children()) {
        if (QWidget *widget = qobject_cast<QWidget *>(object)) {
            QVERIFY(!widget->isEnabled());
            QVERIFY(!widget->testAttribute(Qt::WA_ForceDisabled));
        }
    }
}

QTEST_MAIN(tst_QGroupBox)
#include "tst_qgroupbox.moc"

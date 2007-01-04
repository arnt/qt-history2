/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qapplication.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qwindowsstyle.h>


#include <qdebug.h>

#include <qdatetimeedit.h>
#include <qlocale.h>
#include <qlayout.h>
#include <qeventloop.h>
#include <qstyle.h>
#include <qstyle.h>
#include <QStyleOptionSpinBox>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QList>
#include <QDateTimeEdit>
#include <QWidget>
#include <QLineEdit>
#include <QObject>
#include <QLocale>
#include <QString>
#include <QTest>
#include <QSignalSpy>
#include <QVariantList>
#include <QTestEventList>
#include <QVariant>
#include <QApplication>
#include <QPoint>
#include <QVBoxLayout>
#include <QRect>
#include <QCursor>
#include <QEventLoop>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QTimeEdit>
#include <QMetaType>
#include <QDebug>

#ifdef Q_OS_WIN
# include <windows.h>
#endif


Q_DECLARE_METATYPE(QDate);
Q_DECLARE_METATYPE(QDateTime);
Q_DECLARE_METATYPE(QTime);
Q_DECLARE_METATYPE(QList<int>);


//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qdatetimeedit.h gui/widgets/qdatetimeedit.cpp gui/widgets/qabstractspinbox_p.h gui/widgets/qabstractspinbox.h gui/widgets/qabstractspinbox.cpp

class EditorDateEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    EditorDateEdit(QWidget *parent = 0) : QDateTimeEdit(parent) {}
    QLineEdit *lineEdit() { return QDateTimeEdit::lineEdit(); }
};

class tst_QDateTimeEdit : public QObject
{
    Q_OBJECT
public:
    tst_QDateTimeEdit();
    virtual ~tst_QDateTimeEdit();

public slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();


private slots:
    void getSetCheck();
    void constructor_qwidget();
    void constructor_qdatetime_data();
    void constructor_qdatetime();
    void constructor_qdate_data();
    void constructor_qdate();
    void constructor_qtime_data();
    void constructor_qtime();

    void sectionText_data();
    void sectionText();
    void dateTimeSignalChecking_data();
    void dateTimeSignalChecking();
    void mousePress();
    void stepHourAMPM_data();

    void stepHourAMPM();
    void displayedSections_data();
    void displayedSections();
    void currentSection_data();
    void currentSection();

    void setCurrentSection();
    void setCurrentSection_data();

    void minimumDate_data();
    void minimumDate();
    void maximumDate_data();
    void maximumDate();
    void clearMinimumDate_data();
    void clearMinimumDate();
    void clearMaximumDate_data();
    void clearMaximumDate();
    void displayFormat_data();
    void displayFormat();

    void specialValueText();
    void setRange_data();
    void setRange();

    void selectAndScrollWithKeys();
    void backspaceKey();
    void deleteKey();
    void tabKeyNavigation();
    void tabKeyNavigationWithPrefix();
    void tabKeyNavigationWithSuffix();
    void enterKey();

    void readOnly();

    void wrappingDate_data();
    void wrappingDate();

    void dateSignalChecking_data();
    void dateSignalChecking();

    void wrappingTime_data();
    void wrappingTime();
    void userKeyPress_Time_data();
    void userKeyPress_Time();

    void timeSignalChecking_data();
    void timeSignalChecking();
    void editingFinished();

    void weirdCase();
    void newCase();
    void newCase2();
    void newCase3();
    void newCase4();
    void newCase5();
    void newCase6();

    void task98554();

    void cursorPos();
    void calendarPopup();

    void hour12Test();

#if QT_VERSION >= 0x040200
    void setSelectedSection();
    void reverseTest();
    void setCurrentSectionIndex();
    void setCurrentSectionIndex_data();
    void sectionCount_data();
    void sectionCount();
#endif
#if QT_VERSION >= 0x040300
    void yyTest();
    void separatorKeys();
#endif
private:
    EditorDateEdit* testWidget;
    QWidget *testFocusWidget;
};

typedef QList<QTime> TimeList;
typedef QList<Qt::Key> KeyList;

Q_DECLARE_METATYPE(TimeList)
Q_DECLARE_METATYPE(KeyList)

// Testing get/set functions
void tst_QDateTimeEdit::getSetCheck()
{
    QDateTimeEdit obj1;
    obj1.setDisplayFormat("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z AP");
    // Section QDateTimeEdit::currentSection()
    // void QDateTimeEdit::setCurrentSection(Section)
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::NoSection));
    QVERIFY(QDateTimeEdit::Section(QDateTimeEdit::NoSection) != obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::AmPmSection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::AmPmSection), obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::MSecSection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::MSecSection), obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::SecondSection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::SecondSection), obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::MinuteSection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::MinuteSection), obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::HourSection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::HourSection), obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::DaySection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::DaySection), obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::MonthSection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::MonthSection), obj1.currentSection());
    obj1.setCurrentSection(QDateTimeEdit::Section(QDateTimeEdit::YearSection));
    QCOMPARE(QDateTimeEdit::Section(QDateTimeEdit::YearSection), obj1.currentSection());
}

tst_QDateTimeEdit::tst_QDateTimeEdit()
{
    qRegisterMetaType<QDate>("QDate");
    qRegisterMetaType<QTime>("QTime");
    qRegisterMetaType<QDateTime>("QDateTime");
    qRegisterMetaType<QDateTime>("QList<int>");
}

tst_QDateTimeEdit::~tst_QDateTimeEdit()
{

}

void tst_QDateTimeEdit::initTestCase()
{
    testWidget = new EditorDateEdit(0);
    testFocusWidget = new QWidget(0);
    testFocusWidget->resize(200, 100);
    testFocusWidget->show();
}

void tst_QDateTimeEdit::cleanupTestCase()
{
    delete testFocusWidget;
    testFocusWidget = 0;
    delete testWidget;
    testWidget = 0;
}


void tst_QDateTimeEdit::init()
{
    QLocale::setDefault(QLocale(QLocale::C));
#ifdef Q_OS_WIN
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
#endif
    testWidget->setDisplayFormat("dd/MM/yyyy"); // Nice default to have
    testWidget->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0)));
    testWidget->show();
    testFocusWidget->move(-1000, -1000);
}

void tst_QDateTimeEdit::cleanup()
{
    testWidget->clearMinimumDate();
    testWidget->clearMaximumDate();
    testWidget->clearMinimumTime();
    testWidget->clearMaximumTime();
    testWidget->setSpecialValueText(QString());
    testWidget->setWrapping(false);
}

void tst_QDateTimeEdit::constructor_qwidget()
{
    testWidget->hide();
    QDateTimeEdit dte(0);
    dte.show();
    QCOMPARE(dte.dateTime(), QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0, 0)));
    QCOMPARE(dte.minimumDate(), QDate(1752, 9, 14));
    QCOMPARE(dte.minimumTime(), QTime(0, 0, 0, 0));
    QCOMPARE(dte.maximumDate(), QDate(7999, 12, 31));
    QCOMPARE(dte.maximumTime(), QTime(23, 59, 59, 999));
}

void tst_QDateTimeEdit::constructor_qdatetime_data()
{
    QTest::addColumn<QDateTime>("parameter");
    QTest::addColumn<QDateTime>("displayDateTime");
    QTest::addColumn<QDate>("minimumDate");
    QTest::addColumn<QTime>("minimumTime");
    QTest::addColumn<QDate>("maximumDate");
    QTest::addColumn<QTime>("maximumTime");

    QTest::newRow("normal") << QDateTime(QDate(2004, 6, 16), QTime(13, 46, 32, 764))
			 << QDateTime(QDate(2004, 6, 16), QTime(13, 46, 32, 764))
			 << QDate(1752, 9, 14) << QTime(0, 0, 0, 0)
			 << QDate(7999, 12, 31) << QTime(23, 59, 59, 999);

    QTest::newRow("invalid") << QDateTime(QDate(9999, 99, 99), QTime(13, 46, 32, 764))
			  << QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0, 0))
			  << QDate(1752, 9, 14) << QTime(0, 0, 0, 0)
			  << QDate(7999, 12, 31) << QTime(23, 59, 59, 999);
}



void tst_QDateTimeEdit::constructor_qdatetime()
{
    QFETCH(QDateTime, parameter);
    QFETCH(QDateTime, displayDateTime);
    QFETCH(QDate, minimumDate);
    QFETCH(QTime, minimumTime);
    QFETCH(QDate, maximumDate);
    QFETCH(QTime, maximumTime);

    testWidget->hide();

    QDateTimeEdit dte(parameter);
    dte.show();
    QCOMPARE(dte.dateTime(), displayDateTime);
    QCOMPARE(dte.minimumDate(), minimumDate);
    QCOMPARE(dte.minimumTime(), minimumTime);
    QCOMPARE(dte.maximumDate(), maximumDate);
    QCOMPARE(dte.maximumTime(), maximumTime);
}

void tst_QDateTimeEdit::constructor_qdate_data()
{
    QTest::addColumn<QDate>("parameter");
    QTest::addColumn<QDateTime>("displayDateTime");
    QTest::addColumn<QDate>("minimumDate");
    QTest::addColumn<QTime>("minimumTime");
    QTest::addColumn<QDate>("maximumDate");
    QTest::addColumn<QTime>("maximumTime");

    QTest::newRow("normal") << QDate(2004, 6, 16)
			 << QDateTime(QDate(2004, 6, 16), QTime(0, 0, 0, 0))
			 << QDate(1752, 9, 14) << QTime(0, 0, 0, 0)
			 << QDate(7999, 12, 31) << QTime(23, 59, 59, 999);

    QTest::newRow("invalid") << QDate(9999, 99, 99)
			  << QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0, 0))
			  << QDate(1752, 9, 14) << QTime(0, 0, 0, 0)
			  << QDate(7999, 12, 31) << QTime(23, 59, 59, 999);
}



void tst_QDateTimeEdit::constructor_qdate()
{
    QFETCH(QDate, parameter);
    QFETCH(QDateTime, displayDateTime);
    QFETCH(QDate, minimumDate);
    QFETCH(QTime, minimumTime);
    QFETCH(QDate, maximumDate);
    QFETCH(QTime, maximumTime);

    testWidget->hide();

    QDateTimeEdit dte(parameter);
    dte.show();
    QCOMPARE(dte.dateTime(), displayDateTime);
    QCOMPARE(dte.minimumDate(), minimumDate);
    QCOMPARE(dte.minimumTime(), minimumTime);
    QCOMPARE(dte.maximumDate(), maximumDate);
    QCOMPARE(dte.maximumTime(), maximumTime);
}

void tst_QDateTimeEdit::constructor_qtime_data()
{
    QTest::addColumn<QTime>("parameter");
    QTest::addColumn<QDateTime>("displayDateTime");
    QTest::addColumn<QDate>("minimumDate");
    QTest::addColumn<QTime>("minimumTime");
    QTest::addColumn<QDate>("maximumDate");
    QTest::addColumn<QTime>("maximumTime");

    QTest::newRow("normal") << QTime(13, 46, 32, 764)
			 << QDateTime(QDate(2000, 1, 1), QTime(13, 46, 32, 764))
			 << QDate(2000, 1, 1) << QTime(0, 0, 0, 0)
			 << QDate(2000, 1, 1) << QTime(23, 59, 59, 999);

    QTest::newRow("invalid") << QTime(99, 99, 99, 5000)
			  << QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0, 0))
			  << QDate(2000, 1, 1) << QTime(0, 0, 0, 0)
			  << QDate(2000, 1, 1) << QTime(23, 59, 59, 999);
}



void tst_QDateTimeEdit::constructor_qtime()
{
    QFETCH(QTime, parameter);
    QFETCH(QDateTime, displayDateTime);
    QFETCH(QDate, minimumDate);
    QFETCH(QTime, minimumTime);
    QFETCH(QDate, maximumDate);
    QFETCH(QTime, maximumTime);

    testWidget->hide();

    QDateTimeEdit dte(parameter);
    dte.show();
    QCOMPARE(dte.dateTime(), displayDateTime);
    QCOMPARE(dte.minimumDate(), minimumDate);
    QCOMPARE(dte.minimumTime(), minimumTime);
    QCOMPARE(dte.maximumDate(), maximumDate);
    QCOMPARE(dte.maximumTime(), maximumTime);
}

void tst_QDateTimeEdit::minimumDate_data()
{
    QTest::addColumn<QDate>("minimumDate");
    QTest::addColumn<QDate>("expectedMinDate");

    QTest::newRow("normal-0") << QDate(2004, 5, 10) << QDate(2004, 5, 10);
    QTest::newRow("normal-1") << QDate(2002, 3, 15) << QDate(2002, 3, 15);
    QTest::newRow("normal-2") << QDate(7999, 12, 31) << QDate(7999, 12, 31);
    QTest::newRow("normal-3") << QDate(1753, 1, 1) << QDate(1753, 1, 1);
    QTest::newRow("invalid-0") << QDate(0, 0, 0) << QDate(1752, 9, 14);
}

void tst_QDateTimeEdit::minimumDate()
{
    QFETCH(QDate, minimumDate);
    QFETCH(QDate, expectedMinDate);

    testWidget->setMinimumDate(minimumDate);
    QCOMPARE(testWidget->minimumDate(), expectedMinDate);
}

void tst_QDateTimeEdit::maximumDate_data()
{
    QTest::addColumn<QDate>("maximumDate");
    QTest::addColumn<QDate>("expectedMaxDate");

    QTest::newRow("normal-0") << QDate(2004, 05, 10) << QDate(2004, 5, 10);
    QTest::newRow("normal-1") << QDate(2002, 03, 15) << QDate(2002, 3, 15);
    QTest::newRow("normal-2") << QDate(7999, 12, 31) << QDate(7999, 12, 31);
    QTest::newRow("normal-3") << QDate(1753, 1, 1) << QDate(1753, 1, 1);
    QTest::newRow("invalid-0") << QDate(0, 0, 0) << QDate(7999, 12, 31);
}

void tst_QDateTimeEdit::maximumDate()
{
    QFETCH(QDate, maximumDate);
    QFETCH(QDate, expectedMaxDate);

    testWidget->setMaximumDate(maximumDate);
    QCOMPARE(testWidget->maximumDate(), expectedMaxDate);
}

void tst_QDateTimeEdit::clearMinimumDate_data()
{
    QTest::addColumn<QDate>("minimumDate");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QDate>("expectedMinDateAfterClear");

    QTest::newRow("normal-0") << QDate(2004, 05, 10) << true << QDate(1752, 9, 14);
    QTest::newRow("normal-1") << QDate(2002, 03, 15) << true << QDate(1752, 9, 14);
    QTest::newRow("normal-2") << QDate(7999, 12, 31) << true << QDate(1752, 9, 14);
    QTest::newRow("normal-3") << QDate(1753, 1, 1) << true << QDate(1752, 9, 14);
    QTest::newRow("invalid-0") << QDate(0, 0, 0) << false << QDate(1752, 9, 14);

}

void tst_QDateTimeEdit::clearMinimumDate()
{
    QFETCH(QDate, minimumDate);
    QFETCH(bool, valid);
    QFETCH(QDate, expectedMinDateAfterClear);

    testWidget->setMinimumDate(minimumDate);
    if (valid) {
	testWidget->clearMaximumDate(); // Sanity
	QCOMPARE(testWidget->minimumDate(), minimumDate);
    }
    testWidget->clearMinimumDate();
    QCOMPARE(testWidget->minimumDate(), expectedMinDateAfterClear);
}

void tst_QDateTimeEdit::clearMaximumDate_data()
{
    QTest::addColumn<QDate>("maximumDate");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QDate>("expectedMaxDateAfterClear");

    QTest::newRow("normal-0") << QDate(2004, 05, 10) << true << QDate(7999, 12, 31);
    QTest::newRow("normal-1") << QDate(2002, 03, 15) << true << QDate(7999, 12, 31);
    QTest::newRow("normal-2") << QDate(7999, 12, 31) << true << QDate(7999, 12, 31);
    QTest::newRow("normal-3") << QDate(2000, 1, 1) << true << QDate(7999, 12, 31);
    QTest::newRow("invalid-0") << QDate(0, 0, 0) << false << QDate(7999, 12, 31);
}

void tst_QDateTimeEdit::clearMaximumDate()
{
    QFETCH(QDate, maximumDate);
    QFETCH(bool, valid);
    QFETCH(QDate, expectedMaxDateAfterClear);

    testWidget->setMaximumDate(maximumDate);
    if (valid) {
	testWidget->clearMinimumDate(); // Sanity
	QCOMPARE(testWidget->maximumDate(), maximumDate);
    }
    testWidget->clearMaximumDate();
    QCOMPARE(testWidget->maximumDate(), expectedMaxDateAfterClear);
}

void tst_QDateTimeEdit::displayFormat_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QDateTime>("date");

    const QDateTime dt(QDate(2999, 12, 31), QTime(3, 59, 59, 999));

    QTest::newRow("valid-0") << QString("yyyy MM dd") << true << QString("2999 12 31") << dt;
    QTest::newRow("valid-1") << QString("dd MM yyyy::ss:mm:hh") << true
                             << QString("31 12 2999::59:59:03") << dt;
    QTest::newRow("valid-2") << QString("hh-dd-mm-MM-yy") << true << QString("03-31-59-12-99") << dt;
    QTest::newRow("valid-3") << QString("ddd MM d yyyy::ss:mm:hh") << true
                             << QString("Tue 12 31 2999::59:59:03") << dt;
    QTest::newRow("valid-4") << QString("hh-dd-mm-MM-yyyy") << true << QString("03-31-59-12-2999") << dt;
    QTest::newRow("invalid-0") << QString("yyyy.MM.yy") << true << QString("2999.12.99") << dt;
    QTest::newRow("invalid-1") << QString("y") << false << QString() << dt;
    QTest::newRow("invalid-2") << QString("") << false << QString() << dt;
    QTest::newRow("quoted-1") << QString("'Midday is at:' dd") << true << QString("Midday is at: 31") << dt;
    QTest::newRow("leading1") << QString("h:hh:hhh") << true << QString("3:03:033") << dt;
    QTest::newRow("H1") << QString("HH:hh:ap") << true << QString("03:03:am") << dt;
    QTest::newRow("H2") << QString("HH:hh:ap") << true << QString("23:11:pm")
                        << QDateTime(dt.date(), QTime(23, 0, 0));
}

void tst_QDateTimeEdit::displayFormat()
{
    QFETCH(QString, format);
    QFETCH(bool, valid);
    QFETCH(QString, text);
    QFETCH(QDateTime, date);

    testWidget->setDateTime(date);

    QString compareFormat = format;
    if (!valid)
	compareFormat = testWidget->displayFormat();
    testWidget->setDisplayFormat(format);
    QCOMPARE(testWidget->displayFormat(), compareFormat);
    if (valid)
        QCOMPARE(testWidget->text(), text);
}

void tst_QDateTimeEdit::selectAndScrollWithKeys()
{
    qApp->setActiveWindow(testWidget);
    testWidget->setDate(QDate(2004, 05, 11));
    testWidget->setDisplayFormat("dd/MM/yyyy");
    testWidget->show();
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
#ifdef Q_WS_QWS
    QEXPECT_FAIL(0, "Qt/Embedded does Ctrl-arrow behaviour by default", Abort);
#endif
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("1"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/0"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/2"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/20"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/200"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/2004"));

    // Now the year part should be selected
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->date(), QDate(2005, 5, 11));
    QCOMPARE(testWidget->currentSection(), QDateTimeEdit::YearSection);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("2005"));
    QTest::keyClick(testWidget, Qt::Key_Down);
    QCOMPARE(testWidget->date(), QDate(2004, 5, 11));
    QCOMPARE(testWidget->currentSection(), QDateTimeEdit::YearSection);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("2004"));


#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("4"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("04"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("2004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("/2004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("5/2004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("1/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11/05/2004"));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif

    // Now the day part should be selected
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->date(), QDate(2004, 5, 12));
    QCOMPARE(testWidget->currentSection(), QDateTimeEdit::DaySection);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("12"));
    QTest::keyClick(testWidget, Qt::Key_Down);
    QCOMPARE(testWidget->date(), QDate(2004, 5, 11));
    QCOMPARE(testWidget->currentSection(), QDateTimeEdit::DaySection);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));

#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif

    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
    // Now the day part should be selected
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->date(), QDate(2004, 05, 12));
}

void tst_QDateTimeEdit::backspaceKey()
{
    qApp->setActiveWindow(testWidget);
    testWidget->setDate(QDate(2004, 05, 11));
    testWidget->setDisplayFormat("d/MM/yyyy");
    testWidget->show();
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    QCOMPARE(testWidget->text(), QString("11/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/05/200"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/05/20"));
    // Check that moving into another field reverts it
    for (int i=0;i<3;i++)
	QTest::keyClick(testWidget, Qt::Key_Left);
    QCOMPARE(testWidget->text(), QString("11/05/2004"));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    for (int i=0;i<4;i++) {
	QTest::keyClick(testWidget, Qt::Key_Left, Qt::ShiftModifier);
    }

    QTest::keyClick(testWidget, Qt::Key_Backspace);
#ifdef Q_WS_QWS
    QEXPECT_FAIL(0, "Qt/Embedded does Ctrl-arrow behaviour by default", Abort);
#endif
    QCOMPARE(testWidget->text(), QString("11/05/"));
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/0/2004"));
    testWidget->interpretText();
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/05/200"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/05/20"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/05/2"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/05/"));
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11/0/2004"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("11//2004"));
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("1/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString("/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QCOMPARE(testWidget->text(), QString("1/05/2004"));
}

void tst_QDateTimeEdit::deleteKey()
{
    qApp->setActiveWindow(testWidget);
    testWidget->setDate(QDate(2004, 05, 11));
    testWidget->setDisplayFormat("d/MM/yyyy");
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Delete);
    QCOMPARE(testWidget->text(), QString("1/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Delete);
    QCOMPARE(testWidget->text(), QString("/05/2004"));
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QCOMPARE(testWidget->text(), QString("1/05/2004"));
}

void tst_QDateTimeEdit::tabKeyNavigation()
{
    qApp->setActiveWindow(testWidget);
    testWidget->setDate(QDate(2004, 05, 11));
    testWidget->setDisplayFormat("dd/MM/yyyy");
    testWidget->show();
    testWidget->setCurrentSection(QDateTimeEdit::DaySection);

    QTest::keyClick(testWidget, Qt::Key_Tab);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("05"));
    QTest::keyClick(testWidget, Qt::Key_Tab);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("2004"));
    QTest::keyClick(testWidget, Qt::Key_Tab, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("05"));
    QTest::keyClick(testWidget, Qt::Key_Tab, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
}

void tst_QDateTimeEdit::tabKeyNavigationWithPrefix()
{
    qApp->setActiveWindow(testWidget);
    testWidget->setDate(QDate(2004, 05, 11));
    testWidget->setDisplayFormat("prefix dd/MM/yyyy");

    QTest::keyClick(testWidget, Qt::Key_Tab);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
    QTest::keyClick(testWidget, Qt::Key_Tab);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("05"));
    QTest::keyClick(testWidget, Qt::Key_Tab);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("2004"));
    QTest::keyClick(testWidget, Qt::Key_Tab, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("05"));
    QTest::keyClick(testWidget, Qt::Key_Tab, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
}

void tst_QDateTimeEdit::tabKeyNavigationWithSuffix()
{
    qApp->setActiveWindow(testWidget);
    testWidget->setDate(QDate(2004, 05, 11));
    testWidget->setDisplayFormat("dd/MM/yyyy 'suffix'");

    QTest::keyClick(testWidget, Qt::Key_Tab);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("05"));
    QTest::keyClick(testWidget, Qt::Key_Tab);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("2004"));
    QTest::keyClick(testWidget, Qt::Key_Tab, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("05"));
    QTest::keyClick(testWidget, Qt::Key_Tab, Qt::ShiftModifier);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
}


void tst_QDateTimeEdit::enterKey()
{
    qApp->setActiveWindow(testWidget);
    testWidget->setDate(QDate(2004, 5, 11));
    testWidget->setDisplayFormat("prefix d/MM/yyyy 'suffix'");
    testWidget->lineEdit()->setFocus();

#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QVERIFY(!testWidget->lineEdit()->hasSelectedText());
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QVERIFY(!testWidget->lineEdit()->hasSelectedText());
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Tab);
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
    //static int counter = 0;
    //qDebug() << ++counter << testWidget->lineEdit()->cursorPosition();
    QTest::keyClick(testWidget, Qt::Key_1);
    //qDebug() << ++counter << testWidget->lineEdit()->cursorPosition();
    QTest::keyClick(testWidget, Qt::Key_5);
    //qDebug() << ++counter << testWidget->lineEdit()->cursorPosition();
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Left);

    //qDebug() << ++counter << testWidget->lineEdit()->cursorPosition();
    QTest::keyClick(testWidget, Qt::Key_Enter);
    //qDebug() << ++counter << testWidget->lineEdit()->cursorPosition();
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("15"));
    QCOMPARE(testWidget->date(), QDate(2004, 5, 15));

    QTest::keyClick(testWidget, Qt::Key_9);
    QTest::keyClick(testWidget, Qt::Key_9);
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("9"));
    QCOMPARE(testWidget->date(), QDate(2004, 5, 9));

    QTest::keyClick(testWidget, Qt::Key_0);
    QTest::keyClick(testWidget, Qt::Key_0);
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("9"));
    QCOMPARE(testWidget->date(), QDate(2004, 5, 9));

    QSignalSpy enterSpy(testWidget, SIGNAL(dateChanged(const QDate &)));
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QCOMPARE(enterSpy.count(), 1);

#if 0
    QVariantList list = enterSpy.takeFirst();
    QCOMPARE(list.at(0).toDate(), QDate(2004, 9, 15));
#endif

}

void tst_QDateTimeEdit::specialValueText()
{
    testWidget->setDisplayFormat("dd/MM/yyyy");
    testWidget->setDateRange(QDate(2000, 1, 1), QDate(2001, 1, 1));
    testWidget->setDate(QDate(2000, 1, 2));
    testWidget->setSpecialValueText("foo");
    testWidget->setCurrentSection(QDateTimeEdit::DaySection);
    QCOMPARE(testWidget->date(), QDate(2000, 1, 2));
    QCOMPARE(testWidget->text(), QString("02/01/2000"));
    QTest::keyClick(testWidget, Qt::Key_Down);
    QCOMPARE(testWidget->date(), QDate(2000, 1, 1));
    QCOMPARE(testWidget->text(), QString("foo"));
    QTest::keyClick(testWidget, Qt::Key_Down);
    QCOMPARE(testWidget->date(), QDate(2000, 1, 1));
    QCOMPARE(testWidget->text(), QString("foo"));
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->date(), QDate(2000, 1, 2));
    QCOMPARE(testWidget->text(), QString("02/01/2000"));
    QTest::keyClick(testWidget, Qt::Key_Down);
    QCOMPARE(testWidget->date(), QDate(2000, 1, 1));
    QCOMPARE(testWidget->text(), QString("foo"));

#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->date(), QDate(2000, 1, 2));
    QCOMPARE(testWidget->text(), QString("02/01/2000"));
}


void tst_QDateTimeEdit::setRange_data()
{
    QTest::addColumn<QTime>("minTime");
    QTest::addColumn<QTime>("maxTime");
    QTest::addColumn<QDate>("minDate");
    QTest::addColumn<QDate>("maxDate");
    QTest::addColumn<QDateTime>("expectedMin");
    QTest::addColumn<QDateTime>("expectedMax");

    QTest::newRow("data0") << QTime(0, 0) << QTime(14, 12, 0) << QDate::currentDate() << QDate::currentDate()
                           << QDateTime(QDate::currentDate(), QTime(0, 0)) << QDateTime(QDate::currentDate(),
                                                                                        QTime(14, 12, 0));

    QTest::newRow("data1") << QTime(10, 0) << QTime(1, 12, 0) << QDate::currentDate().addDays(-1)
                           << QDate::currentDate()
                           << QDateTime(QDate::currentDate().addDays(-1), QTime(10, 0))
                           << QDateTime(QDate::currentDate(), QTime(1, 12, 0));
}

void tst_QDateTimeEdit::setRange()
{
    QFETCH(QTime, minTime);
    QFETCH(QTime, maxTime);
    QFETCH(QDate, minDate);
    QFETCH(QDate, maxDate);
    QFETCH(QDateTime, expectedMin);
    QFETCH(QDateTime, expectedMax);
    testWidget->hide();

    QDateTimeEdit dte(0);
    dte.setTimeRange(minTime, maxTime);
    QCOMPARE(dte.minimumTime(), expectedMin.time());
    QCOMPARE(dte.maximumTime(), expectedMax.time());

    dte.setDateRange(minDate, maxDate);

    QCOMPARE(dte.minimumDate(), expectedMin.date());
    QCOMPARE(dte.maximumDate(), expectedMax.date());
    QCOMPARE(dte.minimumTime(), expectedMin.time());
    QCOMPARE(dte.maximumTime(), expectedMax.time());
    dte.hide();

    QDateTimeEdit dte2(0);
    dte2.setDateRange(minDate, maxDate);
    dte2.setTimeRange(minTime, maxTime);

    QCOMPARE(dte2.minimumDate(), expectedMin.date());
    QCOMPARE(dte2.maximumDate(), expectedMax.date());
    QCOMPARE(dte2.minimumTime(), expectedMin.time());
    QCOMPARE(dte2.maximumTime(), expectedMax.time());
    dte2.hide();

    QDateTimeEdit dte3(0);
    dte3.setMinimumTime(minTime);
    dte3.setMaximumTime(maxTime);
    dte3.setMinimumDate(minDate);
    dte3.setMaximumDate(maxDate);

    QCOMPARE(dte3.minimumDate(), expectedMin.date());
    QCOMPARE(dte3.maximumDate(), expectedMax.date());
    QCOMPARE(dte3.minimumTime(), expectedMin.time());
    QCOMPARE(dte3.maximumTime(), expectedMax.time());
    dte3.hide();

    QDateTimeEdit dte4(0);
    dte4.setMinimumDate(minDate);
    dte4.setMaximumDate(maxDate);
    dte4.setMinimumTime(minTime);
    dte4.setMaximumTime(maxTime);

    QCOMPARE(dte4.minimumDate(), expectedMin.date());
    QCOMPARE(dte4.maximumDate(), expectedMax.date());
    QCOMPARE(dte4.minimumTime(), expectedMin.time());
    QCOMPARE(dte4.maximumTime(), expectedMax.time());
    dte4.hide();
}

void tst_QDateTimeEdit::wrappingTime_data()
{
    QTest::addColumn<bool>("startWithMin");
    QTest::addColumn<QTime>("minimumTime");
    QTest::addColumn<QTime>("maximumTime");
    QTest::addColumn<uint>("section");
    QTest::addColumn<QTime>("newTime");

    QTest::newRow("data0") << false << QTime(0,0,0) << QTime(2,2,2) << (uint)QDateTimeEdit::HourSection
                        << QTime(0,2,2);
    QTest::newRow("data1") << true << QTime(0,0,0) << QTime(2,2,2) << (uint)QDateTimeEdit::HourSection
                        << QTime(2,0,0);
    QTest::newRow("data2") << false << QTime(0,0,0) << QTime(2,2,2) << (uint)QDateTimeEdit::MinuteSection
                        << QTime(2,0,2);
    QTest::newRow("data3") << true << QTime(0,0,0) << QTime(2,2,2) << (uint)QDateTimeEdit::MinuteSection
                        << QTime(0,59,0);
    QTest::newRow("data4") << false << QTime(0,0,0) << QTime(2,2,2) << (uint)QDateTimeEdit::SecondSection
                        << QTime(2,2,0);
    QTest::newRow("data5") << true << QTime(0,0,0) << QTime(2,2,2) << (uint)QDateTimeEdit::SecondSection
                        << QTime(0,0,59);
    QTest::newRow("data6") << false << QTime(1,1,1) << QTime(22,22,22) << (uint)QDateTimeEdit::HourSection
                        << QTime(1,22,22);
    QTest::newRow("data7") << true << QTime(1,1,1) << QTime(22,22,22) << (uint)QDateTimeEdit::HourSection
                        << QTime(22,1,1);
    QTest::newRow("data8") << false << QTime(1,1,1) << QTime(22,22,22) << (uint)QDateTimeEdit::MinuteSection
                        << QTime(22,0,22);
    QTest::newRow("data9") << true << QTime(1,1,1) << QTime(22,22,22) << (uint)QDateTimeEdit::MinuteSection
                        << QTime(1,59,1);
    QTest::newRow("data10") << false << QTime(1,1,1) << QTime(22,22,22) << (uint)QDateTimeEdit::SecondSection
                         << QTime(22,22,0);
    QTest::newRow("data11") << true << QTime(1,1,1) << QTime(22,22,22) << (uint)QDateTimeEdit::SecondSection
                         << QTime(1,1,59);
    QTest::newRow("data12") << false << QTime(1,1,1) << QTime(1,2,1) << (uint)QDateTimeEdit::HourSection
                         << QTime(1,2,1);
    QTest::newRow("data13") << true << QTime(1,1,1) << QTime(1,2,1) << (uint)QDateTimeEdit::HourSection
                         << QTime(1,1,1);
    QTest::newRow("data14") << false << QTime(1,1,1) << QTime(1,2,1) << (uint)QDateTimeEdit::MinuteSection
                         << QTime(1,1,1);
    QTest::newRow("data15") << true << QTime(1,1,1) << QTime(1,2,1) << (uint)QDateTimeEdit::MinuteSection
                         << QTime(1,2,1);
    QTest::newRow("data16") << false << QTime(1,1,1) << QTime(1,2,1) << (uint)QDateTimeEdit::SecondSection
                         << QTime(1,2,0);
    QTest::newRow("data17") << true << QTime(1,1,1) << QTime(1,2,1) << (uint)QDateTimeEdit::SecondSection
                         << QTime(1,1,59);
}


void tst_QDateTimeEdit::wrappingTime()
{
    QFETCH(bool, startWithMin);
    QFETCH(QTime, minimumTime);
    QFETCH(QTime, maximumTime);
    QFETCH(uint, section);
    QFETCH(QTime, newTime);

    testWidget->setDisplayFormat("hh:mm:ss");
    testWidget->setMinimumTime(minimumTime);
    testWidget->setMaximumTime(maximumTime);
    testWidget->setWrapping(true);
    testWidget->setCurrentSection((QDateTimeEdit::Section)section);
    if (startWithMin) {
        testWidget->setTime(minimumTime);
        QTest::keyClick(testWidget, Qt::Key_Down);
    } else {
        testWidget->setTime(maximumTime);
        QTest::keyClick(testWidget, Qt::Key_Up);
    }
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QCOMPARE(testWidget->time(), newTime);
}

void tst_QDateTimeEdit::userKeyPress_Time_data()
{
    QTest::addColumn<bool>("ampm");
    QTest::addColumn<QTestEventList>("keys");
    QTest::addColumn<QTime>("expected_time");

    // ***************** test the hours ***************

    // use up/down keys to change hour in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 10, 0, 0 );
        QTest::newRow( "data0" ) << bool(true) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<5; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 6, 0, 0 );
        QTest::newRow( "data1" ) << bool(true) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<10; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 1, 0, 0 );
        QTest::newRow( "data2" ) << bool(true) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<12; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 23, 0, 0 );
        QTest::newRow( "data3" ) << bool(true) << keys << expected;
    }
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Up );
        QTime expected( 12, 0, 0 );
        QTest::newRow( "data4" ) << bool(true) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 13, 0, 0 );
        QTest::newRow( "data5" ) << bool(true) << keys << expected;
    }

    // use up/down keys to change hour in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 10, 0, 0 );
        QTest::newRow( "data6" ) << bool(false) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<5; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 6, 0, 0 );
        QTest::newRow( "data7" ) << bool(false) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<10; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 1, 0, 0 );
        QTest::newRow( "data8" ) << bool(false) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<12; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 23, 0, 0 );
        QTest::newRow( "data9" ) << bool(false) << keys << expected;
    }
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Up );
        QTime expected( 12, 0, 0 );
        QTest::newRow( "data10" ) << bool(false) << keys << expected;
    }
    {
        QTestEventList keys;
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 13, 0, 0 );
        QTest::newRow( "data11" ) << bool(false) << keys << expected;
    }

    // enter a one digit valid hour
    {
        QTestEventList keys;
        keys.addKeyClick( '5' );
        QTime expected( 5, 0, 0 );
        QTest::newRow( "data12" ) << bool(true) << keys << expected;
    }

    // entering a two digit valid hour
    {
        QTestEventList keys;
        keys.addKeyClick( '1' );
        keys.addKeyClick( '1' );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data13" ) << bool(true) << keys << expected;
    }

    // entering an invalid hour
    {
        QTestEventList keys;
        keys.addKeyClick( '2' );
        // the '5' creates an invalid hour (25) so it must be ignored
        keys.addKeyClick( '5' );
        QTime expected( 2, 0, 0 );
        QTest::newRow( "data14" ) << bool(true) << keys << expected;
    }

    // enter a value, in hour which causes a field change
    {
        QTestEventList keys;
        keys.addKeyClick( '0' );
        keys.addKeyClick( '2' );
        keys.addKeyClick( '1' );
        QTime expected( 2, 1, 0 );
        QTest::newRow( "data15" ) << bool(true) << keys << expected;
    }

    // enter a one digit valid hour in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( '5' );
        QTime expected( 5, 0, 0 );
        QTest::newRow( "data16" ) << bool(false) << keys << expected;
    }

    // enter a two digit valid hour in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( '1' );
        keys.addKeyClick( '1' );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data17" ) << bool(false) << keys << expected;
    }

    // enter a two digit valid hour (>12) in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( '1' );
        keys.addKeyClick( '5' );
        QTime expected( 15, 0, 0 );
        QTest::newRow( "data18" ) << bool(false) << keys << expected;
    }

    // enter a two digit valid hour (>20) in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( '2' );
        keys.addKeyClick( '1' );
        QTime expected( 21, 0, 0 );
        QTest::newRow( "data19" ) << bool(false) << keys << expected;
    }

    // enter a two digit invalid hour (>23) in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( '2' );
        keys.addKeyClick( '4' );
        QTime expected( 2, 0, 0 );
        QTest::newRow( "data20" ) << bool(false) << keys << expected;
    }

    // ***************** test the minutes ***************

    // use up/down keys to change the minutes in 12 hour mode
    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 2, 0 );
        QTest::newRow( "data21" ) << bool(true) << keys << expected;
    }
    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<16; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 16, 0 );
        QTest::newRow( "data22" ) << bool(true) << keys << expected;
    }
    { // test maximum value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<59; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 59, 0 );
        QTest::newRow( "data23" ) << bool(true) << keys << expected;
    }
    { // test 'overflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<60; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data24" ) << bool(true) << keys << expected;
    }
    { // test 'underflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 59, 0 );
        QTest::newRow( "data25" ) << bool(true) << keys << expected;
    }
    { // test valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 58, 0 );
        QTest::newRow( "data26" ) << bool(true) << keys << expected;
    }

    // use up/down keys to change the minutes in 24 hour mode

    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 2, 0 );
        QTest::newRow( "data27" ) << bool(false) << keys << expected;
    }
    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<16; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 16, 0 );
        QTest::newRow( "data28" ) << bool(false) << keys << expected;
    }
    { // test maximum value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<59; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 59, 0 );
        QTest::newRow( "data29" ) << bool(false) << keys << expected;
    }
    { // test 'overflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<60; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data30" ) << bool(false) << keys << expected;
    }
    { // test 'underflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 59, 0 );
        QTest::newRow( "data31" ) << bool(false) << keys << expected;
    }
    { // test valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 58, 0 );
        QTest::newRow( "data32" ) << bool(false) << keys << expected;
    }

    // enter a valid one digit minute in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick(Qt::Key_Tab);
        keys.addKeyClick( '2' );
        QTime expected( 11, 2, 0 );
        QTest::newRow( "data33" ) << bool(true) << keys << expected;
    }

    // enter a valid two digit minutes in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick(Qt::Key_Tab);
        keys.addKeyClick( '2' );
        keys.addKeyClick( '4' );
        QTime expected( 11, 24, 0 );
        QTest::newRow( "data34" ) << bool(true) << keys << expected;
    }

    // check the lower limit of the minutes in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick(Qt::Key_Tab);
        keys.addKeyClick( '0' );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data35" ) << bool(true) << keys << expected;
    }

    // check the upper limit of the minutes in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick(Qt::Key_Tab);
        keys.addKeyClick( '5' );
        keys.addKeyClick( '9' );
        QTime expected( 11, 59, 0 );
        QTest::newRow( "data36" ) << bool(true) << keys << expected;
    }

    // enter an invalid two digit minutes in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '6' );
        keys.addKeyClick( '0' );
        QTime expected( 11, 6, 0 );
        QTest::newRow( "data37" ) << bool(true) << keys << expected;
    }

    // test minutes in 24 hour motestWidget-> Behaviour should be exactly the same

    // enter a valid one digit minute in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '2' );
        QTime expected( 11, 2, 0 );
        QTest::newRow( "data38" ) << bool(false) << keys << expected;
    }

    // enter a valid two digit minutes in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '2' );
        keys.addKeyClick( '4' );
        QTime expected( 11, 24, 0 );
        QTest::newRow( "data39" ) << bool(false) << keys << expected;
    }

    // check the lower limit of the minutes in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '0' );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data40" ) << bool(false) << keys << expected;
    }

    // check the upper limit of the minutes in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '5' );
        keys.addKeyClick( '9' );
        QTime expected( 11, 59, 0 );
        QTest::newRow( "data41" ) << bool(false) << keys << expected;
    }

    // enter an invalid two digit minutes in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '6' );
        keys.addKeyClick( '0' );
        QTime expected( 11, 6, 0 );
        QTest::newRow( "data42" ) << bool(false) << keys << expected;
    }

    // ***************** test the seconds ***************

    // use up/down to edit the seconds...

    // use up/down keys to change the seconds in 12 hour mode
    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 2 );
        QTest::newRow( "data43" ) << bool(true) << keys << expected;
    }
    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<16; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 16 );
        QTest::newRow( "data44" ) << bool(true) << keys << expected;
    }
    { // test maximum value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<59; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 59 );
        QTest::newRow( "data45" ) << bool(true) << keys << expected;
    }
    { // test 'overflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<60; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data46" ) << bool(true) << keys << expected;
    }
    { // test 'underflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 0, 59 );
        QTest::newRow( "data47" ) << bool(true) << keys << expected;
    }
    { // test valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 0, 58 );
        QTest::newRow( "data48" ) << bool(true) << keys << expected;
    }

    // use up/down keys to change the seconds in 24 hour mode

    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 2 );
        QTest::newRow( "data49" ) << bool(false) << keys << expected;
    }
    { // test a valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<16; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 16 );
        QTest::newRow( "data50" ) << bool(false) << keys << expected;
    }
    { // test maximum value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<59; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 59 );
        QTest::newRow( "data51" ) << bool(false) << keys << expected;
    }
    { // test 'overflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<60; i++)
            keys.addKeyClick( Qt::Key_Up );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data52" ) << bool(false) << keys << expected;
    }
    { // test 'underflow'
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 0, 59 );
        QTest::newRow( "data53" ) << bool(false) << keys << expected;
    }
    { // test valid value
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        for (uint i=0; i<2; i++)
            keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 0, 58 );
        QTest::newRow( "data54" ) << bool(false) << keys << expected;
    }

    /////////////////
        // enter a valid one digit second in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '2' );
        QTime expected( 11, 0, 2 );
        QTest::newRow( "data55" ) << bool(true) << keys << expected;
    }

    // enter a valid two digit seconds in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '2' );
        keys.addKeyClick( '4' );
        QTime expected( 11, 0, 24 );
        QTest::newRow( "data56" ) << bool(true) << keys << expected;
    }

    // check the lower limit of the seconds in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '0' );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data57" ) << bool(true) << keys << expected;
    }

    // check the upper limit of the seconds in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '5' );
        keys.addKeyClick( '9' );
        QTime expected( 11, 0, 59 );
        QTest::newRow( "data58" ) << bool(true) << keys << expected;
    }

    // enter an invalid two digit seconds in 12 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '6' );
        keys.addKeyClick( '0' );
        QTime expected( 11, 0, 6 );
        QTest::newRow( "data59" ) << bool(true) << keys << expected;
    }

    // test seconds in 24 hour mode. Behaviour should be exactly the same

    // enter a valid one digit minute in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '2' );
        QTime expected( 11, 0, 2 );
        QTest::newRow( "data60" ) << bool(false) << keys << expected;
    }

    // enter a valid two digit seconds in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '2' );
        keys.addKeyClick( '4' );
        QTime expected( 11, 0, 24 );
        QTest::newRow( "data61" ) << bool(false) << keys << expected;
    }

    // check the lower limit of the seconds in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '0' );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data62" ) << bool(false) << keys << expected;
    }

    // check the upper limit of the seconds in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '5' );
        keys.addKeyClick( '9' );
        QTime expected( 11, 0, 59 );
        QTest::newRow( "data63" ) << bool(false) << keys << expected;
    }

    // enter an invalid two digit seconds in 24 h mode
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( '6' );
        keys.addKeyClick( '0' );
        QTime expected( 11, 0, 6 );
        QTest::newRow( "data64" ) << bool(false) << keys << expected;
    }

    // Test the AMPM indicator
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Up );
        QTime expected( 23, 0, 0 );
        QTest::newRow( "data65" ) << bool(true) << keys << expected;
    }
    // Test the AMPM indicator
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 23, 0, 0 );
        QTest::newRow( "data66" ) << bool(true) << keys << expected;
    }
    // Test the AMPM indicator
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Down );
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data67" ) << bool(true) << keys << expected;
    }
    // Test the AMPM indicator
    {
        QTestEventList keys;
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Tab );
        keys.addKeyClick( Qt::Key_Up );
        keys.addKeyClick( Qt::Key_Down );
        QTime expected( 11, 0, 0 );
        QTest::newRow( "data68" ) << bool(true) << keys << expected;
    }
}

void tst_QDateTimeEdit::userKeyPress_Time()
{
    QFETCH(QTestEventList, keys);
    QFETCH(QTime, expected_time);
    QFETCH(bool, ampm);

    if (ampm)
        testWidget->setDisplayFormat("hh:mm:ss ap");
    else
        testWidget->setDisplayFormat("hh:mm:ss");

    testWidget->setTime(QTime(11, 0, 0));
    testWidget->setFocus();

    testWidget->setWrapping(true);

    QTest::keyClick(testWidget, Qt::Key_Enter); // Make sure the first section is now focused
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("11"));
    keys.simulate(testWidget);
    QTest::keyClick(testWidget, Qt::Key_Enter);

    QCOMPARE(testWidget->time(), expected_time);
}

void tst_QDateTimeEdit::wrappingDate_data()
{
    QTest::addColumn<bool>("startWithMin");
    QTest::addColumn<QDate>("minimumDate");
    QTest::addColumn<QDate>("maximumDate");
    QTest::addColumn<uint>("section");
    QTest::addColumn<QDate>("newDate");

    QTest::newRow("data0") << false << QDate(1999, 1, 1) << QDate(1999, 1, 31) << (uint)QDateTimeEdit::DaySection
                        << QDate(1999, 1, 1);
    QTest::newRow("data1") << true << QDate(1999, 1, 1) << QDate(1999, 1, 31) << (uint)QDateTimeEdit::DaySection
                        << QDate(1999, 1, 31);
    QTest::newRow("data2") << false << QDate(1999, 1, 1) << QDate(1999, 1, 31) << (uint)QDateTimeEdit::MonthSection
                        << QDate(1999, 1, 31);
    QTest::newRow("data3") << true << QDate(1999, 1, 1) << QDate(1999, 1, 31) << (uint)QDateTimeEdit::MonthSection
                        << QDate(1999, 1, 1);
    QTest::newRow("data4") << false << QDate(1999, 1, 1) << QDate(1999, 1, 31) << (uint)QDateTimeEdit::YearSection
                        << QDate(1999, 1, 31);
    QTest::newRow("data5") << true << QDate(1999, 1, 1) << QDate(1999, 1, 31) << (uint)QDateTimeEdit::YearSection
                        << QDate(1999, 1, 1);
    QTest::newRow("data6") << false << QDate(1999, 1, 1) << QDate(2000, 1, 31) << (uint)QDateTimeEdit::DaySection
                        << QDate(2000, 1, 1);
    QTest::newRow("data7") << true << QDate(1999, 1, 1) << QDate(2000, 1, 31) << (uint)QDateTimeEdit::DaySection
                        << QDate(1999, 1, 31);
    QTest::newRow("data8") << false << QDate(1999, 1, 1) << QDate(2000, 1, 31) << (uint)QDateTimeEdit::MonthSection
                        << QDate(2000, 1, 31);
    QTest::newRow("data9") << true << QDate(1999, 1, 1) << QDate(2000, 1, 31) << (uint)QDateTimeEdit::MonthSection
                        << QDate(1999, 12, 1);
    QTest::newRow("data10") << false << QDate(1999, 1, 1) << QDate(2000, 1, 31) << (uint)QDateTimeEdit::YearSection
                         << QDate(1999, 1, 31);
    QTest::newRow("data11") << true << QDate(1999, 1, 1) << QDate(2000, 1, 31) << (uint)QDateTimeEdit::YearSection
                         << QDate(2000, 1, 1);
}


void tst_QDateTimeEdit::wrappingDate()
{
    QFETCH(bool, startWithMin);
    QFETCH(QDate, minimumDate);
    QFETCH(QDate, maximumDate);
    QFETCH(uint, section);
    QFETCH(QDate, newDate);

    testWidget->setDisplayFormat("dd/MM/yyyy");
    testWidget->setMinimumDate(minimumDate);
    testWidget->setMaximumDate(maximumDate);
    testWidget->setWrapping(true);
    testWidget->setCurrentSection((QDateTimeEdit::Section)section);

    if (startWithMin) {
        testWidget->setDate(minimumDate);
        QTest::keyClick(testWidget, Qt::Key_Down);
    } else {
        testWidget->setDate(maximumDate);
        QTest::keyClick(testWidget, Qt::Key_Up);
    }
    if (testWidget->currentSection() == QDateTimeEdit::MonthSection)
        QCOMPARE(testWidget->date(), newDate);
}

void tst_QDateTimeEdit::dateSignalChecking_data()
{
    QTest::addColumn<QDate>("originalDate");
    QTest::addColumn<QDate>("newDate");
    QTest::addColumn<int>("timesEmitted");

    QTest::newRow("data0") << QDate(2004, 06, 22) << QDate(2004, 07, 23) << 1;
    QTest::newRow("data1") << QDate(2004, 06, 22) << QDate(2004, 06, 22) << 0;
}

void tst_QDateTimeEdit::dateSignalChecking()
{
    QFETCH(QDate, originalDate);
    QFETCH(QDate, newDate);
    QFETCH(int, timesEmitted);

    testWidget->setDate(originalDate);

    QSignalSpy dateSpy(testWidget, SIGNAL(dateChanged(const QDate &)));
    QSignalSpy dateTimeSpy(testWidget, SIGNAL(dateTimeChanged(const QDateTime &)));
    QSignalSpy timeSpy(testWidget, SIGNAL(timeChanged(const QTime &)));

    testWidget->setDate(newDate);
    QCOMPARE(dateSpy.count(), timesEmitted);

    if (timesEmitted > 0) {
        QList<QVariant> list = dateSpy.takeFirst();
        QDate d;
        d = qVariantValue<QDate>(list.at(0));
        QCOMPARE(d, newDate);
    }
    QCOMPARE(dateTimeSpy.count(), 0);
    QCOMPARE(timeSpy.count(), 0);
}

void tst_QDateTimeEdit::timeSignalChecking_data()
{
    QTest::addColumn<QTime>("originalTime");
    QTest::addColumn<QTime>("newTime");
    QTest::addColumn<int>("timesEmitted");

    QTest::newRow("data0") << QTime(15, 55, 00) << QTime(15, 17, 12) << 1;
    QTest::newRow("data1") << QTime(15, 55, 00) << QTime(15, 55, 00) << 0;
}

void tst_QDateTimeEdit::timeSignalChecking()
{
    QFETCH(QTime, originalTime);
    QFETCH(QTime, newTime);
    QFETCH(int, timesEmitted);

    testWidget->setTime(originalTime);

    testWidget->setDisplayFormat("hh:mm:ss");
    QSignalSpy dateSpy(testWidget, SIGNAL(dateChanged(const QDate &)));
    QSignalSpy dateTimeSpy(testWidget, SIGNAL(dateTimeChanged(const QDateTime &)));
    QSignalSpy timeSpy(testWidget, SIGNAL(timeChanged(const QTime &)));

    testWidget->setTime(newTime);
    QCOMPARE(timeSpy.count(), timesEmitted);

    if (timesEmitted > 0) {
        QList<QVariant> list = timeSpy.takeFirst();
        QTime t;
        t = qVariantValue<QTime>(list.at(0));
        QCOMPARE(t, newTime);
    }
    QCOMPARE(dateTimeSpy.count(), 0);
    QCOMPARE(dateSpy.count(), 0);
}

void tst_QDateTimeEdit::dateTimeSignalChecking_data()
{
    QTest::addColumn<QDateTime>("originalDateTime");
    QTest::addColumn<QDateTime>("newDateTime");
    QTest::addColumn<int>("timesDateEmitted");
    QTest::addColumn<int>("timesTimeEmitted");
    QTest::addColumn<int>("timesDateTimeEmitted");

    QTest::newRow("data0") << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 0))
                        << QDateTime(QDate(2004, 7, 23), QTime(15, 17, 12))
                        << 1 << 1 << 1;
    QTest::newRow("data1") << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 0))
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 17, 12))
                        << 0 << 1 << 1;
    QTest::newRow("data2") << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 0))
                        << QDateTime(QDate(2004, 7, 23), QTime(15, 55, 0))
                        << 1 << 0 << 1;
    QTest::newRow("data3") << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 0))
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 0))
                        << 0 << 0 << 0;
}

void tst_QDateTimeEdit::dateTimeSignalChecking()
{
    QFETCH(QDateTime, originalDateTime);
    QFETCH(QDateTime, newDateTime);
    QFETCH(int, timesDateEmitted);
    QFETCH(int, timesTimeEmitted);
    QFETCH(int, timesDateTimeEmitted);

    testWidget->setDisplayFormat("dd/MM/yyyy hh:mm:ss");
    testWidget->setDateTime(originalDateTime);

    QSignalSpy dateSpy(testWidget, SIGNAL(dateChanged(const QDate &)));
    QSignalSpy timeSpy(testWidget, SIGNAL(timeChanged(const QTime &)));
    QSignalSpy dateTimeSpy(testWidget, SIGNAL(dateTimeChanged(const QDateTime &)));

    testWidget->setDateTime(newDateTime);
    QCOMPARE(dateSpy.count(), timesDateEmitted);
    if (timesDateEmitted > 0) {
        QCOMPARE(timesDateEmitted, 1);
        QList<QVariant> list = dateSpy.takeFirst();
        QDate d;
        d = qVariantValue<QDate>(list.at(0));
        QCOMPARE(d, newDateTime.date());
    }
    QCOMPARE(timeSpy.count(), timesTimeEmitted);
    if (timesTimeEmitted > 0) {
        QList<QVariant> list = timeSpy.takeFirst();
        QTime t;
        t = qVariantValue<QTime>(list.at(0));
        QCOMPARE(t, newDateTime.time());
    }
    QCOMPARE(dateTimeSpy.count(), timesDateTimeEmitted);
    if (timesDateTimeEmitted > 0) {
        QList<QVariant> list = dateTimeSpy.takeFirst();
        QDateTime dt;
        dt = qVariantValue<QDateTime>(list.at(0));
        QCOMPARE(dt, newDateTime);
    }
}


void tst_QDateTimeEdit::sectionText_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<uint>("section");
    QTest::addColumn<QString>("sectionText");

    QTest::newRow("data0") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::NoSection << QString();
    QTest::newRow("data1") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::AmPmSection << QString("pm");
    QTest::newRow("data2") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::MSecSection << QString("789");
    QTest::newRow("data3") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::SecondSection << QString("03");
    QTest::newRow("data4") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::MinuteSection << QString("55");
    QTest::newRow("data5") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::HourSection << QString("03");
    QTest::newRow("data6") << QString("dd/MM/yyyy hh:mm:ss zzz")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::HourSection << QString("15");
    QTest::newRow("data7") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::DaySection << QString("22");
    QTest::newRow("data8") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::MonthSection << QString("06");
    QTest::newRow("data9") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                        << (uint)QDateTimeEdit::YearSection << QString("2004");
    QTest::newRow("data10") << QString("dd/MM/yyyy hh:mm:ss zzz AP")
                         << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                         << (uint)QDateTimeEdit::AmPmSection << QString("PM");
    QTest::newRow("data11") << QString("dd/MM/yyyy hh:mm:ss ap")
                         << QDateTime(QDate(2004, 6, 22), QTime(15, 55, 3, 789))
                         << (uint)QDateTimeEdit::MSecSection << QString();
}

void tst_QDateTimeEdit::sectionText()
{
    QFETCH(QString, format);
    QFETCH(QDateTime, dateTime);
    QFETCH(uint, section);
    QFETCH(QString, sectionText);

    testWidget->setDisplayFormat(format);
    testWidget->setDateTime(dateTime);
    QCOMPARE(testWidget->sectionText((QDateTimeEdit::Section)section), sectionText);
//    QApplication::setLayoutDirection(Qt::RightToLeft);
//    testWidget->setDisplayFormat(format);
//    QCOMPARE(format, testWidget->displayFormat());
//     testWidget->setDateTime(dateTime);
//     QCOMPARE(testWidget->sectionText((QDateTimeEdit::Section)section), sectionText);
//     QApplication::setLayoutDirection(Qt::LeftToRight);
}

void tst_QDateTimeEdit::mousePress()
{
    testWidget->setDate(QDate(2004, 6, 23));
    testWidget->setCurrentSection(QDateTimeEdit::YearSection);
    QCOMPARE(testWidget->currentSection(), QDateTimeEdit::YearSection);
    QTest::mouseClick(testWidget, Qt::LeftButton, 0, QPoint(testWidget->width() - 10, 5));
    QCOMPARE(testWidget->date().year(), 2005);
}

void tst_QDateTimeEdit::stepHourAMPM_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<KeyList>("keys");
    QTest::addColumn<TimeList>("expected");
    QTest::addColumn<QTime>("start");
    QTest::addColumn<QTime>("min");
    QTest::addColumn<QTime>("max");

    {
        KeyList keys;
        TimeList expected;
        keys << Qt::Key_Up;
        expected << QTime(1, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(2, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(3, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(4, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(5, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(6, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(7, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(8, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(9, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(10, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(11, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(12, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(13, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(14, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(15, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(16, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(17, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(18, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(19, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(20, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(21, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(22, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(23, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(22, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(21, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(20, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(19, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(18, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(17, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(16, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(15, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(14, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(13, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(12, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(11, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(10, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(9, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(8, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(7, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(6, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(5, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(4, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(3, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(2, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(1, 0, 0);
        keys << Qt::Key_Down;
        expected << QTime(0, 0, 0);

        QTest::newRow("hh 1") << QString("hh") << keys << expected << QTime(0, 0)
                              << QTime(0, 0) << QTime(23, 59, 59);
        QTest::newRow("hh:ap 1") << QString("hh:ap") << keys << expected
                                 << QTime(0, 0) << QTime(0, 0)
                                 << QTime(23, 59, 59);

        QTest::newRow("HH:ap 2") << QString("HH:ap") << keys << expected
                                 << QTime(0, 0) << QTime(0, 0)
                                 << QTime(23, 59, 59);

    }
    {
        KeyList keys;
        TimeList expected;
        keys << Qt::Key_Down;
        expected << QTime(2, 0, 0);
        QTest::newRow("hh 2") << QString("hh") << keys << expected << QTime(0, 0) << QTime(2, 0, 0) << QTime(23, 59, 59);
        QTest::newRow("hh:ap 2") << QString("hh:ap") << keys << expected << QTime(0, 0) << QTime(2, 0, 0) << QTime(23, 59, 59);
    }
    {
        KeyList keys;
        TimeList expected;
        keys << Qt::Key_Up;
        expected << QTime(23, 0, 0);
        keys << Qt::Key_Up;
        expected << QTime(23, 0, 0);
        QTest::newRow("hh 3") << QString("hh") << keys << expected << QTime(0, 0) << QTime(22, 0, 0)
                              << QTime(23, 59, 59);
        QTest::newRow("hh:ap 3") << QString("hh:ap") << keys << expected << QTime(0, 0)
                                 << QTime(22, 0, 0) << QTime(23, 59, 59);
    }
    {
        KeyList keys;
        TimeList expected;
        keys << Qt::Key_Up;
        expected << QTime(15, 31, 0);
        QTest::newRow("hh:mm:ap 3") << QString("hh:mm:ap") << keys << expected << QTime(15, 31, 0)
                                    << QTime(9, 0, 0) << QTime(16, 0, 0);
        QTest::newRow("hh:mm 3") << QString("hh:mm") << keys << expected << QTime(15, 31, 0)
                                 << QTime(9, 0, 0) << QTime(16, 0, 0);
    }
}

void tst_QDateTimeEdit::stepHourAMPM()
{
    QFETCH(QString, format);
    QFETCH(KeyList, keys);
    QFETCH(TimeList, expected);
    QFETCH(QTime, start);
    QFETCH(QTime, min);
    QFETCH(QTime, max);


    testWidget->setDisplayFormat(format);
    testWidget->setTime(start);
    testWidget->setMinimumTime(min);
    testWidget->setMaximumTime(max);
    if (keys.size() != expected.size()) {
        qWarning("%s:%d Test broken", __FILE__, __LINE__);
        QCOMPARE(keys.size(), expected.size());
    }

    for (int i=0; i<keys.size(); ++i) {
        QTest::keyClick(testWidget, keys.at(i));
        QCOMPARE(testWidget->time(), expected.at(i));
    }
}


void tst_QDateTimeEdit::displayedSections_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<uint>("section");

    QTest::newRow("data0") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)(QDateTimeEdit::DaySection | QDateTimeEdit::MonthSection
                                  | QDateTimeEdit::YearSection | QDateTimeEdit::HourSection
                                  | QDateTimeEdit::MinuteSection | QDateTimeEdit::SecondSection
                                  | QDateTimeEdit::MSecSection | QDateTimeEdit::AmPmSection);
    QTest::newRow("data1") << QString("dd/yyyy hh:mm:ss zzz ap")
                        << (uint)(QDateTimeEdit::DaySection
                                  | QDateTimeEdit::YearSection | QDateTimeEdit::HourSection
                                  | QDateTimeEdit::MinuteSection | QDateTimeEdit::SecondSection
                                  | QDateTimeEdit::MSecSection | QDateTimeEdit::AmPmSection);
    QTest::newRow("data2") << QString("dd/MM/yyyy mm zzz ap")
                        << (uint)(QDateTimeEdit::DaySection | QDateTimeEdit::MonthSection
                                  | QDateTimeEdit::YearSection
                                  | QDateTimeEdit::MinuteSection
                                  | QDateTimeEdit::MSecSection | QDateTimeEdit::AmPmSection);
    QTest::newRow("data3") << QString("dd/MM/yyyy")
                        << (uint)(QDateTimeEdit::DaySection | QDateTimeEdit::MonthSection
                                  | QDateTimeEdit::YearSection);
    QTest::newRow("data4") << QString("hh:mm:ss zzz ap")
                        << (uint)(QDateTimeEdit::HourSection
                                  | QDateTimeEdit::MinuteSection | QDateTimeEdit::SecondSection
                                  | QDateTimeEdit::MSecSection | QDateTimeEdit::AmPmSection);
    QTest::newRow("data5") << QString("dd ap")
                        << (uint)(QDateTimeEdit::DaySection | QDateTimeEdit::AmPmSection);
    QTest::newRow("data6") << QString("zzz")
                        << (uint)QDateTimeEdit::MSecSection;
}

void tst_QDateTimeEdit::displayedSections()
{
    QFETCH(QString, format);
    QFETCH(uint, section);

    testWidget->setDisplayFormat(format);
    QVERIFY((QDateTimeEdit::Section)section == testWidget->displayedSections());
}

void tst_QDateTimeEdit::currentSection_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<uint>("section");
    QTest::addColumn<uint>("currentSection");

    // First is delibrate, this way we can make sure that it is not reset by specifying no section.
    QTest::newRow("data0") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::NoSection << (uint)QDateTimeEdit::YearSection;
    QTest::newRow("data1") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::AmPmSection << (uint)QDateTimeEdit::AmPmSection;
    QTest::newRow("data2") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::MSecSection << (uint)QDateTimeEdit::MSecSection;
    QTest::newRow("data3") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::SecondSection << (uint)QDateTimeEdit::SecondSection;
    QTest::newRow("data4") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::MinuteSection << (uint)QDateTimeEdit::MinuteSection;
    QTest::newRow("data5") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::HourSection << (uint)QDateTimeEdit::HourSection;
    QTest::newRow("data6") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::DaySection << (uint)QDateTimeEdit::DaySection;
    QTest::newRow("data7") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::MonthSection << (uint)QDateTimeEdit::MonthSection;
    QTest::newRow("data8") << QString("dd/MM/yyyy hh:mm:ss zzz ap")
                        << (uint)QDateTimeEdit::YearSection << (uint)QDateTimeEdit::YearSection;
    QTest::newRow("data9") << QString("dd/MM/yyyy hh:mm:ss zzz AP")
                        << (uint)QDateTimeEdit::AmPmSection << (uint)QDateTimeEdit::AmPmSection;
    QTest::newRow("data10") << QString("dd/MM/yyyy hh:mm:ss ap")
                         << (uint)QDateTimeEdit::MSecSection << (uint)QDateTimeEdit::DaySection;
}

void tst_QDateTimeEdit::currentSection()
{
    QFETCH(QString, format);
    QFETCH(uint, section);
    QFETCH(uint, currentSection);

    testWidget->setDisplayFormat(format);
    if ((QDateTimeEdit::Section)section == QDateTimeEdit::NoSection)
        testWidget->setCurrentSection(QDateTimeEdit::YearSection); // Ensure it's not reset (see above)
    testWidget->setCurrentSection((QDateTimeEdit::Section)section);
    QVERIFY((QDateTimeEdit::Section)currentSection == testWidget->currentSection());
}

void tst_QDateTimeEdit::readOnly()
{
    testWidget->hide();
    QDateTimeEdit dt(QDate(2000, 2, 1));
    dt.setDisplayFormat("yyyy.MM.dd");
    dt.show();
    dt.setCurrentSection(QDateTimeEdit::DaySection);
    QTest::keyClick(&dt, Qt::Key_Up);
    QCOMPARE(dt.date(), QDate(2000, 2, 2));
    dt.setReadOnly(true);
    QTest::keyClick(&dt, Qt::Key_Up);
    QCOMPARE(dt.date(), QDate(2000, 2, 2));
    dt.stepBy(1); // stepBy should still work
    QCOMPARE(dt.date(), QDate(2000, 2, 3));
    dt.setReadOnly(false);
    QTest::keyClick(&dt, Qt::Key_Up);
    QCOMPARE(dt.date(), QDate(2000, 2, 4));
}

void tst_QDateTimeEdit::editingFinished()
{
    testFocusWidget->move(10, 10);
    QVBoxLayout *layout = new QVBoxLayout(testFocusWidget);
    QDateTimeEdit *box = new QDateTimeEdit(testFocusWidget);
    layout->addWidget(box);
    QDateTimeEdit *box2 = new QDateTimeEdit(testFocusWidget);
    layout->addWidget(box2);

    box->activateWindow();
    qApp->processEvents();
    QVERIFY(box->hasFocus());

    QSignalSpy editingFinishedSpy1(box, SIGNAL(editingFinished()));
    QSignalSpy editingFinishedSpy2(box2, SIGNAL(editingFinished()));

    QTest::keyClick(box, Qt::Key_Up);
    QTest::keyClick(box, Qt::Key_Up);
    QCOMPARE(editingFinishedSpy1.count(), 0);
    QCOMPARE(editingFinishedSpy2.count(), 0);

    QTest::keyClick(box, Qt::Key_Return);
    QCOMPARE(editingFinishedSpy1.count(), 1);
    QCOMPARE(editingFinishedSpy2.count(), 0);

    QTest::keyClick(box, Qt::Key_Enter);
    QCOMPARE(editingFinishedSpy1.count(), 2);
    QCOMPARE(editingFinishedSpy2.count(), 0);

    box2->setFocus();
    for (int i = 0; i < 10; ++i) {
        if (box2->hasFocus())
            break;
        QTest::qWait(100);
    }
    if (!box2->hasFocus())
        QSKIP("Your window manager is too broken for this test", SkipAll);
    QCOMPARE(editingFinishedSpy1.count(), 3);
    QCOMPARE(editingFinishedSpy2.count(), 0);

    testFocusWidget->hide();
    QCOMPARE(editingFinishedSpy1.count(), 3); // box has already lost focus
    QCOMPARE(editingFinishedSpy2.count(), 1);
}

void tst_QDateTimeEdit::weirdCase()
{
    testWidget->setDateRange(QDate(2005, 1, 1), QDate(2010, 12, 31));
    testWidget->setDisplayFormat("dd//MM//yyyy");
    testWidget->setDate(testWidget->minimumDate());
    QTest::keyClick(testWidget, Qt::Key_Left);
    QVERIFY(!testWidget->lineEdit()->hasSelectedText());
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 0);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(!testWidget->lineEdit()->hasSelectedText());
#ifdef Q_WS_QWS
    QEXPECT_FAIL(0, "Qt/Embedded does Ctrl-arrow behaviour by default", Abort);
#endif
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 8);

    QTest::keyClick(testWidget, Qt::Key_Delete);
    QCOMPARE(testWidget->text(), QString("01//01//005"));
    QTest::keyClick(testWidget, Qt::Key_4);
    QCOMPARE(testWidget->text(), QString("01//01//005"));}


void tst_QDateTimeEdit::newCase()
{
    testWidget->setDisplayFormat("MMMM'a'MbMMMcMM");
    testWidget->setDate(QDate(2005, 6, 1));
    QCOMPARE(testWidget->text(), QString("Junea6bJunc06"));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->text(), QString("Julya7bJulc07"));
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("July"));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString());
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Right);
    QTest::keyClick(testWidget, Qt::Key_Delete);
    QTest::keyClick(testWidget, Qt::Key_Left);

#ifdef Q_WS_QWS
    QEXPECT_FAIL(0, "Qt/Embedded does Ctrl-arrow behaviour by default", Abort);
#endif

    QCOMPARE(testWidget->text(), QString("Jula7bJulc07"));
    QTest::keyClick(testWidget, Qt::Key_Delete);
    QCOMPARE(testWidget->text(), QString("Jua7bJulc07"));
    QTest::keyClick(testWidget, Qt::Key_N);
    QCOMPARE(testWidget->text(), QString("Juna7bJulc07"));
    QTest::keyClick(testWidget, Qt::Key_E);
    QCOMPARE(testWidget->text(), QString("Junea6bJunc06"));
}

void tst_QDateTimeEdit::newCase2()
{
    testWidget->setDisplayFormat("MMMM yyyy-MM-dd MMMM");
    testWidget->setDate(QDate(2005, 8, 8));
    QTest::keyClick(testWidget, Qt::Key_Return);
    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->text(), QString(" 2005-08-08 August"));
}

void tst_QDateTimeEdit::newCase3()
{
    testWidget->setDisplayFormat("dd MMMM yyyy");
    testWidget->setDate(QDate(2000, 1, 1));
    testWidget->setGeometry(QRect(QPoint(0, 0), testWidget->sizeHint()));
    testWidget->setCurrentSection(QDateTimeEdit::MonthSection);
    QTest::keyClick(testWidget, Qt::Key_Return);
    QTest::keyClick(testWidget, Qt::Key_J);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("01 J 2000"));
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 4);
    QTest::keyClick(testWidget, Qt::Key_A);
    QTest::keyClick(testWidget, Qt::Key_N);
    QTest::keyClick(testWidget, Qt::Key_U);
    QTest::keyClick(testWidget, Qt::Key_A);
    QTest::keyClick(testWidget, Qt::Key_R);
}


void tst_QDateTimeEdit::cursorPos()
{
    testWidget->setDisplayFormat("dd MMMM yyyy");
    //testWidget->setGeometry(0, 0, 200, 200);
    testWidget->setCurrentSection(QDateTimeEdit::MonthSection);
    QTest::keyClick(testWidget, Qt::Key_Return);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 10);
    QTest::keyClick(testWidget, Qt::Key_J);
    QTest::keyClick(testWidget, Qt::Key_A);
    QTest::keyClick(testWidget, Qt::Key_N);
    QTest::keyClick(testWidget, Qt::Key_U);
    QTest::keyClick(testWidget, Qt::Key_A);
    QTest::keyClick(testWidget, Qt::Key_R);
    //QCursor::setPos(20, 20);
    //QEventLoop l;
    //l.exec();
    QTest::keyClick(testWidget, Qt::Key_Y);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 11);
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Return);
    QTest::keyClick(testWidget, Qt::Key_3);
    QTest::keyClick(testWidget, Qt::Key_1);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 3);
}

void tst_QDateTimeEdit::newCase4()
{
    testWidget->setDisplayFormat("hh:mm");
    testWidget->setMinimumTime(QTime(3, 3, 0));
    QTest::keyClick(testWidget, Qt::Key_Return);
    QTest::keyClick(testWidget, Qt::Key_0);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("0:03"));
    QTest::keyClick(testWidget, Qt::Key_2);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("0:03"));
    QTest::keyClick(testWidget, Qt::Key_4);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("04:03"));
}

void tst_QDateTimeEdit::newCase5()
{
    testWidget->setDisplayFormat("yyyy-MM-dd hh:mm:ss zzz 'ms'");
    testWidget->setDateTime(QDateTime(QDate(2005, 10, 7), QTime(17, 44, 13, 100)));
    testWidget->show();
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("2005-10-07 17:44:13 100 ms"));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    QTest::keyClick(testWidget, Qt::Key_Backtab, Qt::ShiftModifier);

    QTest::keyClick(testWidget, Qt::Key_Return);
    QTest::keyClick(testWidget, Qt::Key_1);
    QTest::keyClick(testWidget, Qt::Key_2);
    QTest::keyClick(testWidget, Qt::Key_4);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("2005-10-07 17:44:13 124 ms"));

    QTest::keyClick(testWidget, Qt::Key_Backspace);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("2005-10-07 17:44:13 12 ms"));
}

void tst_QDateTimeEdit::newCase6()
{
    testWidget->setDisplayFormat("d-yyyy-MM-dd");
    testWidget->setDate(QDate(2005, 10, 7));
    testWidget->show();
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("7-2005-10-07"));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Return);
    QTest::keyClick(testWidget, Qt::Key_1);
    QTest::keyClick(testWidget, Qt::Key_2);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("12-2005-10-12"));
}


void tst_QDateTimeEdit::task98554()
{
    testWidget->setDisplayFormat("mm.ss.zzz(ms)");
    testWidget->setTime(QTime(0, 0, 9));
    testWidget->setCurrentSection(QDateTimeEdit::SecondSection);
    testWidget->show();
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("00.09.000(09)"));
    QCOMPARE(testWidget->time(), QTime(0, 0, 9, 0));
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("00.10.000(010)"));
    QCOMPARE(testWidget->time(), QTime(0, 0, 10, 0));
}

static QList<int> makeList(int val1, int val2 = -1, int val3 = -1, int val4 = -1,
                           int val5 = -1, int val6 = -1, int val7 = -1,
                           int val8 = -1, int val9 = -1, int val10 = -1,
                           int val11 = -1, int val12 = -1, int val13 = -1, int val14 = -1)
{
    QList<int> ret;
    const int *ints[] = { &val1, &val2, &val3, &val4, &val5, &val6, &val7, &val8,
                          &val9, &val10, &val11, &val12, &val13, &val14, 0 };
    int index = 0;
    while (ints[index] && *ints[index] >= 0) {
        ret.append(*ints[index]);
        ++index;
    }
    return ret;
}



void tst_QDateTimeEdit::setCurrentSection_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QList<int> >("setCurrentSections");
    QTest::addColumn<QList<int> >("expectedCursorPositions");

    QTest::newRow("Day") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z") << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                         << makeList(QDateTimeEdit::DaySection, QDateTimeEdit::DaySection, QDateTimeEdit::DaySection)
                         << makeList(24, 0, 24);
    QTest::newRow("Month") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z") << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                           << makeList(QDateTimeEdit::MonthSection, QDateTimeEdit::MonthSection, QDateTimeEdit::MonthSection)
                           << makeList(3, 26, 3);
    QTest::newRow("Year") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z") << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                          << makeList(QDateTimeEdit::YearSection, QDateTimeEdit::YearSection, QDateTimeEdit::YearSection)
                          << makeList(6, 28, 6);
    QTest::newRow("Hour") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z") << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                          << makeList(QDateTimeEdit::HourSection, QDateTimeEdit::HourSection, QDateTimeEdit::HourSection)
                          << makeList(11, 31, 11);
    QTest::newRow("Minute") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z") << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                            << makeList(QDateTimeEdit::MinuteSection, QDateTimeEdit::MinuteSection, QDateTimeEdit::MinuteSection)
                            << makeList(14, 33, 14);
    QTest::newRow("Second") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z") << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                            << makeList(QDateTimeEdit::SecondSection, QDateTimeEdit::SecondSection, QDateTimeEdit::SecondSection)
                            << makeList(17, 35, 17);
    QTest::newRow("MSec") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z") << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                          << makeList(QDateTimeEdit::MSecSection, QDateTimeEdit::MSecSection, QDateTimeEdit::MSecSection)
                          << makeList(20, 37, 20);
}

void tst_QDateTimeEdit::setCurrentSection()
{
    QFETCH(QString, format);
    QFETCH(QDateTime, dateTime);
    QFETCH(QList<int>, setCurrentSections);
    QFETCH(QList<int>, expectedCursorPositions);

    Q_ASSERT(setCurrentSections.size() == expectedCursorPositions.size());
    testWidget->setDisplayFormat(format);
    testWidget->setDateTime(dateTime);
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif

    testWidget->resize(400, 100);
    for (int i=0; i<setCurrentSections.size(); ++i) {
        testWidget->setCurrentSection((QDateTimeEdit::Section)setCurrentSections.at(i));
        QCOMPARE(testWidget->currentSection(), (QDateTimeEdit::Section)setCurrentSections.at(i));
        QCOMPARE(testWidget->lineEdit()->cursorPosition(), expectedCursorPositions.at(i));
    }
}


#if QT_VERSION >= 0x040200

void tst_QDateTimeEdit::setSelectedSection()
{
    testWidget->setDisplayFormat("mm.ss.zzz('ms') m");
    testWidget->setTime(QTime(0, 0, 9));
    testWidget->show();
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QVERIFY(!testWidget->lineEdit()->hasSelectedText());
    testWidget->setSelectedSection(QDateTimeEdit::MinuteSection);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("00"));
    testWidget->setCurrentSection(QDateTimeEdit::MinuteSection);
    testWidget->setSelectedSection(QDateTimeEdit::MinuteSection);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("0"));
    testWidget->setSelectedSection(QDateTimeEdit::SecondSection);
    QCOMPARE(testWidget->lineEdit()->selectedText(), QString("09"));
    testWidget->setSelectedSection(QDateTimeEdit::NoSection);
    QVERIFY(!testWidget->lineEdit()->hasSelectedText());

}

void tst_QDateTimeEdit::calendarPopup()
{
    testWidget->setDisplayFormat("dd/MM/yyyy");
    testWidget->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0)));
    testWidget->show();
    testWidget->setCalendarPopup(true);
    QCOMPARE(testWidget->calendarPopup(), true);
    QStyle *style = testWidget->style();
    QStyleOptionComboBox opt;
    opt.initFrom(testWidget);
    opt.subControls = QStyle::SC_ComboBoxArrow;
    QRect rect = style->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, testWidget);
    QTest::mouseClick(testWidget, Qt::LeftButton, 0, QPoint(rect.left()+rect.width()/2, rect.top()+rect.height()/2));
    QWidget *wid = testWidget->findChild<QWidget *>("qt_datetimedit_calendar");
    QVERIFY(wid != 0);
    testWidget->hide();

    QTimeEdit timeEdit;
    timeEdit.setCalendarPopup(true);
    timeEdit.show();

    opt.initFrom(&timeEdit);
    opt.subControls = QStyle::SC_ComboBoxArrow;
    rect = style->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, &timeEdit);
    QTest::mouseClick(&timeEdit, Qt::LeftButton, 0, QPoint(rect.left()+rect.width()/2, rect.top()+rect.height()/2));
    QWidget *wid2 = timeEdit.findChild<QWidget *>("qt_datetimedit_calendar");
    QVERIFY(wid2 == 0);
    timeEdit.hide();
}

class RestoreLayoutDirectioner
{
public:
    RestoreLayoutDirectioner(Qt::LayoutDirection was)
        : old(was)
    {}

    ~RestoreLayoutDirectioner()
    {
        QApplication::setLayoutDirection(old);
    }
private:
    const Qt::LayoutDirection old;
};

void tst_QDateTimeEdit::reverseTest()
{
    const RestoreLayoutDirectioner restorer(QApplication::layoutDirection());
    QApplication::setLayoutDirection(Qt::RightToLeft);
    testWidget->setDisplayFormat("dd/MM/yyyy");
    testWidget->setDate(QDate(2001, 3, 30));
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("2001/03/30"));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_End);
#endif
    QCOMPARE(testWidget->currentSection(), QDateTimeEdit::DaySection);
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->date(), QDate(2001, 3, 31));
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("2001/03/31"));
}


void tst_QDateTimeEdit::setCurrentSectionIndex_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QList<int> >("setCurrentSectionsIndex");
    QTest::addColumn<QList<int> >("expectedSectionsAt");
    QTest::addColumn<QList<int> >("expectedCursorPositions");

    QTest::newRow("Test1") << QString("dd/MM/yyyy hh:mm:ss.zzz d/M/yy h:m:s.z")
                           << QDateTime(QDate(2001, 1, 1), QTime(1, 2, 3, 4))
                           << makeList(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13)
                           << makeList(QDateTimeEdit::DaySection,
                                       QDateTimeEdit::MonthSection,
                                       QDateTimeEdit::YearSection,

                                       QDateTimeEdit::HourSection,
                                       QDateTimeEdit::MinuteSection,
                                       QDateTimeEdit::SecondSection,
                                       QDateTimeEdit::MSecSection,

                                       QDateTimeEdit::DaySection,
                                       QDateTimeEdit::MonthSection,
                                       QDateTimeEdit::YearSection,

                                       QDateTimeEdit::HourSection,
                                       QDateTimeEdit::MinuteSection,
                                       QDateTimeEdit::SecondSection,
                                       QDateTimeEdit::MSecSection)
                           << makeList(0, 3, 6,
                                       11, 14, 17, 20,
                                       24, 26, 28,
                                       31, 33, 35, 37);

    QTest::newRow("Test2") << QString("yyyy/123/MM")
                           << QDateTime(QDate(2001, 1, 1), QTime())
                           << makeList(1, 0)
                           << makeList(QDateTimeEdit::MonthSection,
                                       QDateTimeEdit::YearSection)
                           << makeList(9, 0);
}

void tst_QDateTimeEdit::setCurrentSectionIndex()
{
    QFETCH(QString, format);
    QFETCH(QDateTime, dateTime);
    QFETCH(QList<int>, setCurrentSectionsIndex);
    QFETCH(QList<int>, expectedSectionsAt);
    QFETCH(QList<int>, expectedCursorPositions);

    Q_ASSERT(setCurrentSectionsIndex.size() == expectedCursorPositions.size());
    Q_ASSERT(expectedSectionsAt.size() == expectedCursorPositions.size());
    testWidget->setDisplayFormat(format);
    testWidget->setDateTime(dateTime);
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif

    testWidget->resize(400, 100);
    for (int i=0; i<setCurrentSectionsIndex.size(); ++i) {
        testWidget->setCurrentSectionIndex(setCurrentSectionsIndex.at(i));
        QCOMPARE(testWidget->currentSectionIndex(), setCurrentSectionsIndex.at(i));
        QCOMPARE(testWidget->sectionAt(setCurrentSectionsIndex.at(i)), (QDateTimeEdit::Section)expectedSectionsAt.at(i));
        QCOMPARE(testWidget->lineEdit()->cursorPosition(), expectedCursorPositions.at(i));
    }
}

void tst_QDateTimeEdit::sectionCount_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<int>("expectedSectionCount");
    QTest::newRow("yyyy/123/MM") << QString("yyyy/123/MM") << 2;
    QTest::newRow("yyyy/yy/yyyy") << QString("yyyy/yy/yyyy") << 3;
    QTest::newRow("h hh hhh hhhh") << QString("h hh hhh hhhh") << 6;
}

void tst_QDateTimeEdit::sectionCount()
{
    QFETCH(QString, format);
    QFETCH(int, expectedSectionCount);
    testWidget->setDisplayFormat(format);
    QCOMPARE(testWidget->sectionCount(), expectedSectionCount);
}

void tst_QDateTimeEdit::hour12Test()
{
    testWidget->setDisplayFormat("hh a");
    testWidget->setTime(QTime(0, 0, 0));
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("12 am"));
    for (int i=0; i<11; ++i) {
        QTest::keyClick(testWidget, Qt::Key_Up);
    }
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("11 am"));
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("12 pm"));
    for (int i=0; i<11; ++i) {
        QTest::keyClick(testWidget, Qt::Key_Up);
    }
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("11 pm"));
    QTest::keyClick(testWidget, Qt::Key_Up);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("11 pm"));
    for (int i=0; i<12; ++i) {
        QTest::keyClick(testWidget, Qt::Key_Down);
    }
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("11 am"));
    QTest::keyClick(testWidget, Qt::Key_1);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("1 am"));
    QTest::keyClick(testWidget, Qt::Key_3);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("1 am"));
}

#endif

#if QT_VERSION >= 0x040300
void tst_QDateTimeEdit::yyTest()
{
    testWidget->setDisplayFormat("yy");
    const int centuries[2] = {2000, 1900};
    for (int i=0; i<2; ++i) {
        testWidget->setDate(QDate(centuries[i], 1, 1));
        QCOMPARE(testWidget->lineEdit()->displayText(), QString("00"));
        QCOMPARE(testWidget->date(), QDate(centuries[i], 1, 1));
        QTest::keyClick(testWidget, Qt::Key_Up);
        QCOMPARE(testWidget->lineEdit()->displayText(), QString("01"));
        QCOMPARE(testWidget->date(), QDate(centuries[i] + 1, 1, 1));
        QTest::keyClick(testWidget, Qt::Key_Return);
        QTest::keyClick(testWidget, Qt::Key_Delete);
        QCOMPARE(testWidget->lineEdit()->displayText(), QString());
        QTest::keyClick(testWidget, Qt::Key_7);
        QCOMPARE(testWidget->lineEdit()->displayText(), QString("7"));
        QTest::keyClick(testWidget, Qt::Key_1);
        QCOMPARE(testWidget->lineEdit()->displayText(), QString("71"));
        QCOMPARE(testWidget->date(), QDate(centuries[i] + 71, 1, 1));
        QTest::keyClick(testWidget, Qt::Key_1);
        QCOMPARE(testWidget->lineEdit()->displayText(), QString("71"));
        QCOMPARE(testWidget->date(), QDate(centuries[i] + 71, 1, 1));
    }
}

void tst_QDateTimeEdit::separatorKeys()
{
    testWidget->setDisplayFormat("dd/MMM yyyy hh:mm.ss 7 zzz");
    testWidget->setDateTime(QDateTime(QDate(1980, 1, 5), QTime(12, 13, 14, 156)));
#ifdef Q_WS_MAC
    QTest::keyClick(testWidget, Qt::Key_Left, Qt::ControlModifier);
#else
    QTest::keyClick(testWidget, Qt::Key_Home);
#endif
    QTest::keyClick(testWidget, Qt::Key_Enter);
    QTest::keyClick(testWidget, Qt::Key_3);
    QTest::keyClick(testWidget, Qt::Key_Slash);
    QCOMPARE(testWidget->lineEdit()->displayText(), QString("03/Jan 1980 12:13.14 7 156"));
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 6);
    QCOMPARE(testWidget->currentSectionIndex(), 1);

    QTest::keyClick(testWidget, Qt::Key_Space);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 11);
    QCOMPARE(testWidget->currentSectionIndex(), 2);

    QTest::keyClick(testWidget, Qt::Key_Space);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 14);
    QCOMPARE(testWidget->currentSectionIndex(), 3);

    QTest::keyClick(testWidget, Qt::Key_Colon);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 17);
    QCOMPARE(testWidget->currentSectionIndex(), 4);

    QTest::keyClick(testWidget, Qt::Key_Period);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 20);
    QCOMPARE(testWidget->currentSectionIndex(), 5);

    QTest::keyClick(testWidget, Qt::Key_7);
    QCOMPARE(testWidget->lineEdit()->cursorPosition(), 26);
    QCOMPARE(testWidget->currentSectionIndex(), 6);
}

#endif

QTEST_MAIN(tst_QDateTimeEdit)
#include "tst_qdatetimeedit.moc"


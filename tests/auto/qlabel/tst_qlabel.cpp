/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include "qlabel.h"
#include <qapplication.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmovie.h>


//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qlabel.h gui/widgets/qlabel.cpp

class Widget : public QWidget
{
public:
    Widget() { }

    QList<QEvent::Type> events;

protected:
    bool event(QEvent *ev) {
        events.append(ev->type());
        return QWidget::event(ev);
    }

};

class tst_QLabel : public QObject
{
Q_OBJECT

public:
    tst_QLabel();
    virtual ~tst_QLabel();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void text();
    void setText_data();
    void setText();
    void textFormat();
    void setTextFormat();
    void buddy();
    void setBuddy();
    void setFont();
    void setNum();
    void clear();
    void wordWrap();
    void eventPropagation_data();
    void eventPropagation();
    void focusPolicy();

private:
    QLabel *testWidget;
    QPointer<Widget> test_box;
    QPointer<QLabel> test_label;
    QLineEdit *test_edit;
};

// Testing get/set functions
void tst_QLabel::getSetCheck()
{
    QLabel obj1;
    // bool QLabel::wordWrap()
    // void QLabel::setWordWrap(bool)
    obj1.setWordWrap(false);
    QCOMPARE(false, obj1.wordWrap());
    obj1.setWordWrap(true);
    QCOMPARE(true, obj1.wordWrap());

    // QWidget * QLabel::buddy()
    // void QLabel::setBuddy(QWidget *)
    QWidget *var2 = new QWidget();
    obj1.setBuddy(var2);
    QCOMPARE(var2, obj1.buddy());
    obj1.setBuddy((QWidget *)0);
    QCOMPARE((QWidget *)0, obj1.buddy());
    delete var2;

    // QMovie * QLabel::movie()
    // void QLabel::setMovie(QMovie *)
    QMovie *var3 = new QMovie;
    obj1.setMovie(var3);
    QCOMPARE(var3, obj1.movie());
    obj1.setMovie((QMovie *)0);
    QCOMPARE((QMovie *)0, obj1.movie());
    delete var3;
}


tst_QLabel::tst_QLabel(): test_box(0)
{
}

tst_QLabel::~tst_QLabel()
{
}

void tst_QLabel::initTestCase()
{
    // Create the test class
    testWidget = new QLabel(0);
    testWidget->resize( 200, 200 );
    testWidget->show();
}

void tst_QLabel::cleanupTestCase()
{
    delete testWidget;
    testWidget = 0;
    if (test_box)
        delete test_box;
}

void tst_QLabel::init()
{
    testWidget->setTextFormat( Qt::AutoText );
    testWidget->setBuddy( 0 );
    testWidget->setIndent( 0 );
    testWidget->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    testWidget->setScaledContents( FALSE );
}

void tst_QLabel::cleanup()
{
    if (QTest::currentTestFunction() == QLatin1String("setBuddy")) {
	testWidget->show();

	delete test_box; // this should delete tst_labl and test_edit as well.
    }
}

void tst_QLabel::buddy()
{
    DEPENDS_ON( "setBuddy" );
}

void tst_QLabel::setBuddy()
{
#ifdef Q_WS_MAC
    QSKIP("Set buddy doesn't make much sense on Mac OS X", SkipAll);
#endif
    testWidget->hide();

    test_box = new Widget;
    test_label= new QLabel( test_box );
    test_label->setText( "&Test with a buddy" );
    test_edit = new QLineEdit( test_box );
    QVBoxLayout *layout = new QVBoxLayout(test_box);
    layout->addWidget(test_label);
    layout->addWidget(test_edit);
    test_box->show();
    qApp->setActiveWindow(test_box);
    QVERIFY(test_box->isActiveWindow());

    test_label->setBuddy( test_edit );
    test_label->setFocus();
    QVERIFY( !test_edit->hasFocus() );
    QTest::keyClick( test_box, 't', Qt::AltModifier );
    QVERIFY( test_edit->hasFocus() );
}

void tst_QLabel::text()
{
    DEPENDS_ON( "setText" );
}

void tst_QLabel::setText_data()
{
    QTest::addColumn<QString>("txt");
    QTest::addColumn<QString>("font");

    QString prefix = "";
#ifdef Q_OS_WIN32
    prefix = "win32_";
#endif

    QTest::newRow( QString(prefix + "data0").toLatin1() ) << QString("This is a single line") << QString("Helvetica");
    QTest::newRow( QString(prefix + "data1").toLatin1() ) << QString("This is the first line\nThis is the second line") << QString("Courier");
    QTest::newRow( QString(prefix + "data2").toLatin1() ) << QString("This is the first line\nThis is the second line\nThis is the third line") << QString("Helvetica");
    QTest::newRow( QString(prefix + "data3").toLatin1() ) << QString("This is <b>bold</b> richtext") << QString("Courier");
}

void tst_QLabel::setText()
{
    QFETCH( QString, txt );
    QFETCH( QString, font );
    QFont f( font, 8 );
    testWidget->setFont( f );
    testWidget->setText( txt );
    QCOMPARE( testWidget->text(), txt );
}


void tst_QLabel::textFormat()
{
    DEPENDS_ON( "setTextFormat" );
}

void tst_QLabel::setTextFormat()
{
    // lets' start with the simple stuff...
    testWidget->setTextFormat( Qt::PlainText );
    QVERIFY( testWidget->textFormat() == Qt::PlainText );

    testWidget->setTextFormat( Qt::RichText );
    QVERIFY( testWidget->textFormat() == Qt::RichText );

#if QT_VERSION >= 0x030100
    testWidget->setTextFormat( Qt::LogText );
    QVERIFY( testWidget->textFormat() == Qt::LogText );
#endif

    testWidget->setTextFormat( Qt::AutoText );
    QVERIFY( testWidget->textFormat() == Qt::AutoText );
}


void tst_QLabel::setFont()
{
    DEPENDS_ON( "setText" );
}

void tst_QLabel::setNum()
{
    testWidget->setText( "This is a text" );
    testWidget->setNum( 12 );
    QCOMPARE( testWidget->text(), QString("12") );
    testWidget->setNum( 12.345 );
    QCOMPARE( testWidget->text(), QString("12.345") );
}

void tst_QLabel::clear()
{
    const QString TEXT = "blah blah";
    testWidget->setText(TEXT);
    QCOMPARE(testWidget->text(), TEXT);
    testWidget->clear();
    QVERIFY(testWidget->text().isEmpty());
}

void tst_QLabel::wordWrap()
{
    QLabel label;

    QVERIFY(!label.wordWrap());

    label.setText("Plain Text");
    QVERIFY(!label.wordWrap());

    label.setText("<b>rich text</b>");
    QVERIFY(!label.wordWrap());

    label.setWordWrap(false);
    label.setText("<b>rich text</b>");
    QVERIFY(!label.wordWrap());
}

void tst_QLabel::eventPropagation_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("textInteractionFlags");
    QTest::addColumn<int>("focusPolicy");
    QTest::addColumn<bool>("propagation");

    QTest::newRow("plain text1") << QString("plain text") << int(Qt::LinksAccessibleByMouse) << int(Qt::NoFocus) << true;
    QTest::newRow("plain text2") << QString("plain text") << (int)Qt::TextSelectableByKeyboard << (int)Qt::ClickFocus << false;
    QTest::newRow("plain text3") << QString("plain text") << (int)Qt::TextSelectableByMouse << (int)Qt::ClickFocus << false;
    QTest::newRow("plain text4") << QString("plain text") << (int)Qt::NoTextInteraction << (int)Qt::NoFocus << true;
    QTest::newRow("rich text1") << QString("<b>rich text</b>") << (int)Qt::LinksAccessibleByMouse << (int)Qt::NoFocus << false;
    QTest::newRow("rich text2") << QString("<b>rich text</b>") << (int)Qt::TextSelectableByKeyboard << (int)Qt::ClickFocus << false;
    QTest::newRow("rich text3") << QString("<b>rich text</b>") << (int)Qt::TextSelectableByMouse << (int)Qt::ClickFocus << false;
    QTest::newRow("rich text4") << QString("<b>rich text</b>") << (int)Qt::NoTextInteraction << (int)Qt::NoFocus << true;
    QTest::newRow("rich text4") << QString("<b>rich text</b>") << (int)Qt::LinksAccessibleByKeyboard << (int)Qt::StrongFocus << false;

    if (!test_box)
        test_box = new Widget;
    if (!test_label)
        test_label = new QLabel(test_box);
}

void tst_QLabel::eventPropagation()
{
    QFETCH(QString, text);
    QFETCH(int, textInteractionFlags);
    QFETCH(int, focusPolicy);
    QFETCH(bool, propagation);

    // plain text (accepts mouse event _only_ when label selectable by mouse)
    test_label->setText(text);
    test_box->events.clear();
    test_label->setTextInteractionFlags(Qt::TextInteractionFlags(textInteractionFlags));
    QVERIFY(int(test_label->focusPolicy()) == focusPolicy);
    QTest::mousePress(test_label, Qt::LeftButton);
    QVERIFY(test_box->events.contains(QEvent::MouseButtonPress) == propagation); // should have propagated!
}

void tst_QLabel::focusPolicy()
{
    delete test_label;
    test_label = new QLabel;
    QCOMPARE(test_label->focusPolicy(), Qt::NoFocus); // default
    test_label->setFocusPolicy(Qt::StrongFocus);
    test_label->setText("Whatever"); // setting text should not change the focus policy
    QCOMPARE(test_label->focusPolicy(), Qt::StrongFocus);
    test_label->setTextInteractionFlags(Qt::TextSelectableByKeyboard); // this should
    QCOMPARE(test_label->focusPolicy(), Qt::ClickFocus);
    test_label->setFocusPolicy(Qt::StrongFocus);
    test_label->setText("Whatever"); // setting text should not change the focus policy
    QCOMPARE(test_label->focusPolicy(), Qt::StrongFocus);
    test_label->setTextInteractionFlags(Qt::NoTextInteraction);
    QCOMPARE(test_label->focusPolicy(), Qt::NoFocus);
    test_label->setFocusPolicy(Qt::StrongFocus);
    test_label->setTextInteractionFlags(Qt::NoTextInteraction);
    QCOMPARE(test_label->focusPolicy(), Qt::StrongFocus); // is not touched since value didn't change
    delete test_label;
}

QTEST_MAIN(tst_QLabel)
#include "tst_qlabel.moc"

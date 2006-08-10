/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include "qlineedit.h"
#include "qapplication.h"
#include "qstringlist.h"
#include "qstyle.h"
#include "qvalidator.h"


#ifdef Q_WS_MAC
#include <cstdlib> // For the random function.
#endif

#include <qlineedit.h>
#include <qdebug.h>


//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qlineedit.h gui/widgets/qlineedit.cpp

#include "qcommonstyle.h"
#include "qstyleoption.h"
class QPainter;

class StyleOptionTestStyle : public QCommonStyle
{
private:
    bool readOnly;

public:
    inline StyleOptionTestStyle() : QCommonStyle(), readOnly(false)
    {
    }

    inline void setReadOnly(bool readOnly)
    {
        this->readOnly = readOnly;
    }

    inline void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *,
                                 const QWidget *) const
    {
        switch (pe) {
            case PE_PanelLineEdit:
            if (readOnly)
                QVERIFY(opt->state & QStyle::State_ReadOnly);
            else
                QVERIFY(!(opt->state & QStyle::State_ReadOnly));
            break;

            default:
            break;
        }
    }
};

class tst_QLineEdit : public QObject
{
Q_OBJECT

public:
    enum EventStates { Press, Release, Click };

    tst_QLineEdit();
    virtual ~tst_QLineEdit();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void experimental();

    void upperAndLowercase();

    void setInputMask_data();
    void setInputMask();

    void inputMask_data();
    void inputMask();

    void clearInputMask();

    void keypress_inputMask_data();
    void keypress_inputMask();

    void inputMaskAndValidator_data();
    void inputMaskAndValidator();

    void hasAcceptableInputMask_data();
    void hasAcceptableInputMask();

    void hasAcceptableInputValidator();


    void redo_data();
    void redo();
    void isRedoAvailable();

    void undo_data();
    void undo();
    void isUndoAvailable();

    void undo_keypressevents_data();
    void undo_keypressevents();

    void clear();

    void text_data();
    void text();
    void textMask_data();
    void textMask();
    void maskCharacter();
    void maskCharacter_data();
    void setText();

    void displayText_data();
    void displayText();
    void setEchoMode();
    void echoMode();

    void maxLength_mask_data();
    void maxLength_mask();

    void maxLength_data();
    void maxLength();
    void setMaxLength();

    void isReadOnly();
    void setReadOnly();

    void cursorPosition();

    void cursorPositionChanged_data();
    void cursorPositionChanged();

    void selectedText();
    void hasSelectedText();

    void textChangedAndTextEdited();
    void returnPressed();
    void returnPressed_maskvalidator_data();
    void returnPressed_maskvalidator();

    void setValidator();
    void validator();
    void clearValidator();

    void setValidator_QIntValidator_data();
    void setValidator_QIntValidator();

    void frame_data();
    void frame();

    void setAlignment_data();
    void setAlignment();
    void alignment();

    void isModified();
    void edited();
    void setEdited();

    void insert();
    void setSelection_data();
    void setSelection();

    void cut();
    void copy();
    void paste();

    void maxLengthAndInputMask();
    void returnPressedKeyEvent();

    void keepSelectionOnTabFocusIn();

    void readOnlyStyleOption();

    void validateOnFocusOut();

    void editUnvalidText();

protected slots:
#ifdef QT3_SUPPORT
    void lostFocus();
#endif
    void editingFinished();

    void onTextChanged( const QString &newString );
    void onTextEdited( const QString &newString );
    void onReturnPressed();
    void onSelectionChanged();
    void onCursorPositionChanged(int oldpos, int newpos);

private:
    // keyClicks(..) is moved to QtTestCase
    void psKeyClick(QWidget *target, Qt::Key key, Qt::KeyboardModifiers pressState = 0);
    void psKeyClick(QTestEventList &keys, Qt::Key key, Qt::KeyboardModifiers pressState = 0);

    bool validInput;
    QString changed_string;
    int changed_count;
    int edited_count;
    int return_count;
    int selection_count;
    int lastCursorPos;
    int newCursorPos;
    QLineEdit *testWidget;
};

typedef QList<int> IntList;
Q_DECLARE_METATYPE(IntList)
Q_DECLARE_METATYPE(QLineEdit::EchoMode)

// Testing get/set functions
void tst_QLineEdit::getSetCheck()
{
    QLineEdit obj1;
    // const QValidator * QLineEdit::validator()
    // void QLineEdit::setValidator(const QValidator *)
    QIntValidator *var1 = new QIntValidator(0);
    obj1.setValidator(var1);
    QCOMPARE(var1, obj1.validator());
    obj1.setValidator((QValidator *)0);
    QCOMPARE((QValidator *)0, obj1.validator());
    delete var1;

    // bool QLineEdit::dragEnabled()
    // void QLineEdit::setDragEnabled(bool)
    obj1.setDragEnabled(false);
    QCOMPARE(false, obj1.dragEnabled());
    obj1.setDragEnabled(true);
    QCOMPARE(true, obj1.dragEnabled());
}

tst_QLineEdit::tst_QLineEdit()
{
    validInput = FALSE;
}

tst_QLineEdit::~tst_QLineEdit()
{
}

void tst_QLineEdit::initTestCase()
{
    testWidget = new QLineEdit(0);
    testWidget->setObjectName("testWidget");
    connect(testWidget, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(onCursorPositionChanged(int, int)));
    connect(testWidget, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
    connect(testWidget, SIGNAL(textEdited(const QString&)), this, SLOT(onTextEdited(const QString&)));
    connect(testWidget, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
    connect(testWidget, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
    connect(testWidget, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
#ifdef QT3_SUPPORT
    connect(testWidget, SIGNAL(lostFocus()), this, SLOT(lostFocus()));
#endif

    testWidget->resize(200,50);
    testWidget->show();

    changed_count = 0;
    edited_count = 0;
    selection_count = 0;
}

void tst_QLineEdit::cleanupTestCase()
{
    delete testWidget;
}

void tst_QLineEdit::init()
{
    return_count = 0;
    testWidget->clear();
    testWidget->setEchoMode(QLineEdit::Normal);
    testWidget->setMaxLength(32767);
    testWidget->setReadOnly(FALSE);
    testWidget->setText("");
    testWidget->setInputMask("");
    testWidget->setFrame(TRUE);
    testWidget->setValidator(0);
    testWidget->setDragEnabled(TRUE);
}

void tst_QLineEdit::cleanup()
{
}

void tst_QLineEdit::experimental()
{
    QIntValidator intValidator(3, 7, 0);
    testWidget->setValidator(&intValidator);
    testWidget->setText("");


    // test the order of setting these
    testWidget->setInputMask("");
    testWidget->setText("abc123");
    testWidget->setInputMask("000.000.000.000");
    QCOMPARE(testWidget->text(), QString("123..."));
    testWidget->setText("");


}

void tst_QLineEdit::upperAndLowercase()
{
    testWidget->setInputMask("");
    testWidget->setText("");

    QTest::keyClicks(testWidget, "aAzZ`1234567890-=~!@#$%^&*()_+[]{}\\|;:'\",.<>/?");
    qApp->processEvents();
    QCOMPARE(testWidget->text(), QString("aAzZ`1234567890-=~!@#$%^&*()_+[]{}\\|;:'\",.<>/?"));
}

void tst_QLineEdit::setInputMask_data()
{
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<QString>("expectedDisplay");
    QTest::addColumn<bool>("insert_text");

    // both keyboard and insert()
    for (bool insert_text = FALSE; insert_text != TRUE; insert_text = TRUE) {
        QString insert_mode = "keys ";
        if (insert_text)
            insert_mode = "insert ";

	QTest::newRow(QString(insert_mode + "ip_localhost").toLatin1())
	    << QString("000.000.000.000")
	    << QString("127.0.0.1")
	    << QString("127.0.0.1")
	    << QString("127.0  .0  .1  ")
	    << bool(insert_text);
        QTest::newRow(QString(insert_mode + "mac").toLatin1())
            << QString("HH:HH:HH:HH:HH:HH;#")
            << QString("00:E0:81:21:9E:8E")
            << QString("00:E0:81:21:9E:8E")
            << QString("00:E0:81:21:9E:8E")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "mac2").toLatin1())
            << QString("<HH:>HH:!HH:HH:HH:HH;#")
            << QString("AAe081219E8E")
            << QString("aa:E0:81:21:9E:8E")
            << QString("aa:E0:81:21:9E:8E")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "byte").toLatin1())
            << QString("BBBBBBBB;0")
            << QString("11011001")
            << QString("11111")
            << QString("11011001")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "halfbytes").toLatin1())
            << QString("bbbb.bbbb;-")
            << QString("110. 0001")
            << QString("110.0001")
            << QString("110-.0001")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "blank char same type as content").toLatin1())
	    << QString("000.000.000.000;0")
	    << QString("127.0.0.1")
	    << QString("127...1")
	    << QString("127.000.000.100")
	    << bool(insert_text);
	QTest::newRow(QString(insert_mode + "parts of ip_localhost").toLatin1())
	    << QString("000.000.000.000")
	    << QString(".0.0.1")
	    << QString(".0.0.1")
	    << QString("   .0  .0  .1  ")
	    << bool(insert_text);
        QTest::newRow(QString(insert_mode + "ip_null").toLatin1())
            << QString("000.000.000.000")
            << QString()
            << QString("...")
            << QString("   .   .   .   ")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "ip_null_hash").toLatin1())
            << QString("000.000.000.000;#")
            << QString()
            << QString("...")
            << QString("###.###.###.###")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "ip_overflow").toLatin1())
            << QString("000.000.000.000")
            << QString("1234123412341234")
            << QString("123.412.341.234")
            << QString("123.412.341.234")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "uppercase").toLatin1())
            << QString(">AAAA")
            << QString("AbCd")
            << QString("ABCD")
            << QString("ABCD")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "lowercase").toLatin1())
            << QString("<AAAA")
            << QString("AbCd")
            << QString("abcd")
            << QString("abcd")
            << bool(insert_text);

        QTest::newRow(QString(insert_mode + "nocase").toLatin1())
            << QString("!AAAA")
            << QString("AbCd")
            << QString("AbCd")
            << QString("AbCd")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "nocase1").toLatin1())
            << QString("!A!A!A!A")
            << QString("AbCd")
            << QString("AbCd")
            << QString("AbCd")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "nocase2").toLatin1())
            << QString("AAAA")
            << QString("AbCd")
            << QString("AbCd")
            << QString("AbCd")
            << bool(insert_text);

        QTest::newRow(QString(insert_mode + "reserved").toLatin1())
            << QString("{n}[0]")
            << QString("A9")
            << QString("A9")
            << QString("A9")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "escape01").toLatin1())
            << QString("\\N\\n00")
            << QString("9")
            << QString("Nn9")
            << QString("Nn9 ")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "escape02").toLatin1())
            << QString("\\\\00")
            << QString("0")
            << QString("\\0")
            << QString("\\0 ")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "escape03").toLatin1())
            << QString("\\(00\\)")
            << QString("0")
            << QString("(0)")
            << QString("(0 )")
            << bool(insert_text);

        QTest::newRow(QString(insert_mode + "upper_lower_nocase1").toLatin1())
            << QString(">AAAA<AAAA!AAAA")
            << QString("AbCdEfGhIjKl")
            << QString("ABCDefghIjKl")
            << QString("ABCDefghIjKl")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "upper_lower_nocase2").toLatin1())
            << QString(">aaaa<aaaa!aaaa")
            << QString("AbCdEfGhIjKl")
            << QString("ABCDefghIjKl")
            << QString("ABCDefghIjKl")
            << bool(insert_text);

        QTest::newRow(QString(insert_mode + "exact_case1").toLatin1())
            << QString(">A<A<A>A>A<A!A!A")
            << QString("AbCdEFGH")
            << QString("AbcDEfGH")
            << QString("AbcDEfGH")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "exact_case2").toLatin1())
            << QString(">A<A<A>A>A<A!A!A")
            << QString("aBcDefgh")
            << QString("AbcDEfgh")
            << QString("AbcDEfgh")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "exact_case3").toLatin1())
            << QString(">a<a<a>a>a<a!a!a")
            << QString("AbCdEFGH")
            << QString("AbcDEfGH")
            << QString("AbcDEfGH")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "exact_case4").toLatin1())
            << QString(">a<a<a>a>a<a!a!a")
            << QString("aBcDefgh")
            << QString("AbcDEfgh")
            << QString("AbcDEfgh")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "exact_case5").toLatin1())
            << QString(">H<H<H>H>H<H!H!H")
            << QString("aBcDef01")
            << QString("AbcDEf01")
            << QString("AbcDEf01")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "exact_case6").toLatin1())
            << QString(">h<h<h>h>h<h!h!h")
            << QString("aBcDef92")
            << QString("AbcDEf92")
            << QString("AbcDEf92")
            << bool(insert_text);

        QTest::newRow(QString(insert_mode + "illegal_keys1").toLatin1())
            << QString("AAAAAAAA")
            << QString("A2#a;.0!")
            << QString("Aa")
            << QString("Aa      ")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "illegal_keys2").toLatin1())
            << QString("AAAA")
            << QString("f4f4f4f4")
            << QString("ffff")
            << QString("ffff")
            << bool(insert_text);
        QTest::newRow(QString(insert_mode + "blank=input").toLatin1())
            << QString("9999;0")
            << QString("2004")
            << QString("2004")
            << QString("2004")
            << bool(insert_text);
    }
}

void tst_QLineEdit::setInputMask()
{
    QFETCH(QString, mask);
    QFETCH(QString, input);
    QFETCH(QString, expectedText);
    QFETCH(QString, expectedDisplay);
    QFETCH(bool, insert_text);

    QEXPECT_FAIL( "keys blank=input", "To eat blanks or not? Known issue. Task 43172", Abort);

    // First set the input mask
    testWidget->setInputMask(mask);

    // then either insert using insert() or keyboard
    if (insert_text) {
        testWidget->insert(input);
    } else {
        psKeyClick(testWidget, Qt::Key_Home);
        for (int i=0; i<input.length(); i++)
            QTest::keyClick(testWidget, input.at(i).toLatin1());
    }

    QCOMPARE(testWidget->text(), expectedText);
    QCOMPARE(testWidget->displayText(), expectedDisplay);
}

void tst_QLineEdit::inputMask_data()
{
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("expectedMask");

    // if no mask is set a nul string should be returned
    QTest::newRow("") << QString("") << QString();
    QTest::newRow("") << QString() << QString();

    // try different masks
    QTest::newRow("") << QString("000.000.000.000") << QString("000.000.000.000; ");
    QTest::newRow("") << QString("000.000.000.000;#") << QString("000.000.000.000;#");
    QTest::newRow("") << QString("AAA.aa.999.###;") << QString("AAA.aa.999.###; ");
    QTest::newRow("") << QString(">abcdef<GHIJK") << QString(">abcdef<GHIJK; ");
//    QTest::newRow("") << QString() << QString();

    // set an invalid input mask...
    // the current behaviour is that this exact (faulty) string is returned.
    QTest::newRow("") << QString("ABCDEFGHIKLMNOP;") << QString("ABCDEFGHIKLMNOP; ");

    // verify that we can unset the mask again
    QTest::newRow("") << QString("") << QString();
}

void tst_QLineEdit::inputMask()
{
    QFETCH(QString, mask);
    QFETCH(QString, expectedMask);

    testWidget->setInputMask(mask);
    QCOMPARE(testWidget->inputMask(), expectedMask);
}

void tst_QLineEdit::clearInputMask()
{
    testWidget->setInputMask("000.000.000.000");
    QVERIFY(testWidget->inputMask() != QString::null);
    testWidget->setInputMask(QString::null);
    QCOMPARE(testWidget->inputMask(), QString());
}

void tst_QLineEdit::keypress_inputMask_data()
{
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QTestEventList>("keys");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<QString>("expectedDisplayText");

    {
        QTestEventList keys;
        // inserting 'A1.2B'
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_A);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_Period);
        keys.addKeyClick(Qt::Key_2);
        keys.addKeyClick(Qt::Key_B);
        QTest::newRow("jumping on period(separator)") << QString("000.000;_") << keys << QString("1.2") << QString("1__.2__");
    }
    {
        QTestEventList keys;
        // inserting 'A1.2B'
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_0);
        keys.addKeyClick(Qt::Key_Exclam);
        keys.addKeyClick('P');
        keys.addKeyClick(Qt::Key_3);
        QTest::newRow("jumping on input") << QString("D0.AA.XX.AA.00;_") << keys << QString("0..!P..3") << QString("_0.__.!P.__.3_");
    }
    {
        QTestEventList keys;
        // pressing delete
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_Delete);
        QTest::newRow("delete") << QString("000.000;_") << keys << QString(".") << QString("___.___");
    }
    {
        QTestEventList keys;
        // selecting all and delete
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_End, Qt::ShiftModifier);
        keys.addKeyClick(Qt::Key_Delete);
        QTest::newRow("deleting all") << QString("000.000;_") << keys << QString(".") << QString("___.___");
    }
    {
        QTestEventList keys;
        // inserting '12.12' then two backspaces
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
        keys.addKeyClick(Qt::Key_Period);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
        keys.addKeyClick(Qt::Key_Backspace);
        keys.addKeyClick(Qt::Key_Backspace);
        QTest::newRow("backspace") << QString("000.000;_") << keys << QString("12.") << QString("12_.___");
    }
    {
        QTestEventList keys;
        // inserting '12ab'
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
        keys.addKeyClick(Qt::Key_A);
        keys.addKeyClick(Qt::Key_B);
        QTest::newRow("uppercase") << QString("9999 >AA;_") << keys << QString("12 AB") << QString("12__ AB");
    }
}

void tst_QLineEdit::keypress_inputMask()
{
    QFETCH(QString, mask);
    QFETCH(QTestEventList, keys);
    QFETCH(QString, expectedText);
    QFETCH(QString, expectedDisplayText);

    testWidget->setInputMask(mask);
    keys.simulate(testWidget);

    QCOMPARE(testWidget->text(), expectedText);
    QCOMPARE(testWidget->displayText(), expectedDisplayText);
}


void tst_QLineEdit::hasAcceptableInputMask_data()
{
    QTest::addColumn<QString>("optionalMask");
    QTest::addColumn<QString>("requiredMask");
    QTest::addColumn<QString>("invalid");
    QTest::addColumn<QString>("valid");

    QTest::newRow("Alphabetic optional and required")
	<< QString("aaaa") << QString("AAAA") << QString("ab") << QString("abcd");
    QTest::newRow("Alphanumeric optional and require")
	<< QString("nnnn") << QString("NNNN") << QString("R2") << QString("R2D2");
    QTest::newRow("Any optional and required")
	<< QString("xxxx") << QString("XXXX") << QString("+-") << QString("+-*/");
    QTest::newRow("Numeric (0-9) required")
	<< QString("0000") << QString("9999") << QString("11") << QString("1138");
    QTest::newRow("Numeric (1-9) optional and required")
	<< QString("dddd") << QString("DDDD") << QString("12") << QString("1234");
}

void tst_QLineEdit::hasAcceptableInputMask()
{
    QFocusEvent lostFocus(QEvent::FocusOut);
    QFETCH(QString, optionalMask);
    QFETCH(QString, requiredMask);
    QFETCH(QString, invalid);
    QFETCH(QString, valid);

    // test that invalid input (for required) work for optionalMask
    testWidget->setInputMask(optionalMask);
    validInput = FALSE;
    testWidget->setText(invalid);
    qApp->sendEvent(testWidget, &lostFocus);
    QVERIFY(validInput);

    // at the moment we don't strip the blank character if it is valid input, this makes the test between x vs X useless
    QEXPECT_FAIL( "Any optional and required", "To eat blanks or not? Known issue. Task 43172", Abort);

    // test requiredMask
    testWidget->setInputMask(requiredMask);
    validInput = TRUE;
    testWidget->setText(invalid);
    validInput = testWidget->hasAcceptableInput();
    QVERIFY(!validInput);

    validInput = FALSE;
    testWidget->setText(valid);
    qApp->sendEvent(testWidget, &lostFocus);
    QVERIFY(validInput);
}

static const int chars = 8;
class ValidatorWithFixup : public QValidator
{
public:
    ValidatorWithFixup(QWidget *parent = 0)
        : QValidator(parent)
    {}

    QValidator::State validate(QString &str, int &) const
    {
        const int s = str.size();
        if (s < chars) {
            return Intermediate;
        } else if (s > chars) {
            return Invalid;
        }
        return Acceptable;
    }

    void fixup(QString &str) const
    {
        str = str.leftJustified(chars, 'X', true);
    }
};



void tst_QLineEdit::hasAcceptableInputValidator()
{
    QFocusEvent lostFocus(QEvent::FocusOut);
    ValidatorWithFixup val;
    testWidget->setValidator(&val);
    testWidget->setText("foobar");
    qApp->sendEvent(testWidget, &lostFocus);
    QVERIFY(testWidget->hasAcceptableInput());
}



void tst_QLineEdit::maskCharacter_data()
{
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("expectedValid");

    QTest::newRow("Hex") << QString("H")
                         << QString("0123456789abcdefABCDEF") << true;
    QTest::newRow("hex") << QString("h")
                         << QString("0123456789abcdefABCDEF") << true;
    QTest::newRow("HexInvalid") << QString("H")
                                << QString("ghijklmnopqrstuvwxyzGHIJKLMNOPQRSTUVWXYZ")
                                << false;
    QTest::newRow("hexInvalid") << QString("h")
                                << QString("ghijklmnopqrstuvwxyzGHIJKLMNOPQRSTUVWXYZ")
                                << false;
    QTest::newRow("Bin") << QString("B")
                         << QString("01") << true;
    QTest::newRow("bin") << QString("b")
                         << QString("01") << true;
    QTest::newRow("BinInvalid") << QString("B")
                                << QString("23456789qwertyuiopasdfghjklzxcvbnm")
                                << false;
    QTest::newRow("binInvalid") << QString("b")
                                << QString("23456789qwertyuiopasdfghjklzxcvbnm")
                                << false;
}

void tst_QLineEdit::maskCharacter()
{
    QFETCH(QString, mask);
    QFETCH(QString, input);
    QFETCH(bool, expectedValid);

    QFocusEvent lostFocus(QEvent::FocusOut);

    testWidget->setInputMask(mask);
    for (int i = 0; i < input.size(); ++i) {
        QString in = QString(input.at(i));
        QString expected = expectedValid ? in : QString();
        testWidget->setText(QString(input.at(i)));
        qApp->sendEvent(testWidget, &lostFocus);
        QCOMPARE(testWidget->text(), expected);
    }
}

#define NORMAL 0
#define REPLACE_UNTIL_END 1

void tst_QLineEdit::undo_data()
{
    QTest::addColumn<QStringList>("insertString");
    QTest::addColumn<IntList>("insertIndex");
    QTest::addColumn<IntList>("insertMode");
    QTest::addColumn<QStringList>("expectedString");
    QTest::addColumn<bool>("use_keys");

    for (int i=0; i<2; i++) {
        QString keys_str = "keyboard";
        bool use_keys = TRUE;
        if (i==0) {
            keys_str = "insert";
            use_keys = FALSE;
        }

        {
	    IntList insertIndex;
	    IntList insertMode;
	    QStringList insertString;
	    QStringList expectedString;

	    insertIndex << -1;
            insertMode << NORMAL;
	    insertString << "1";

	    insertIndex << -1;
            insertMode << NORMAL;
	    insertString << "5";

            insertIndex << 1;
            insertMode << NORMAL;
	    insertString << "3";

            insertIndex << 1;
            insertMode << NORMAL;
	    insertString << "2";

            insertIndex << 3;
            insertMode << NORMAL;
	    insertString << "4";

	    expectedString << "12345";
	    expectedString << "1235";
	    expectedString << "135";
	    expectedString << "15";
	    expectedString << "";

	    QTest::newRow(QString(keys_str + "_numbers").toLatin1()) <<
                insertString <<
                insertIndex <<
                insertMode <<
                expectedString <<
                bool(use_keys);
        }
        {
	    IntList insertIndex;
	    IntList insertMode;
	    QStringList insertString;
	    QStringList expectedString;

	    insertIndex << -1;
            insertMode << NORMAL;
	    insertString << "World"; // World

            insertIndex << 0;
            insertMode << NORMAL;
	    insertString << "Hello"; // HelloWorld

            insertIndex << 0;
            insertMode << NORMAL;
	    insertString << "Well"; // WellHelloWorld

            insertIndex << 9;
            insertMode << NORMAL;
	    insertString << "There"; // WellHelloThereWorld;

	    expectedString << "WellHelloThereWorld";
	    expectedString << "WellHelloWorld";
	    expectedString << "HelloWorld";
	    expectedString << "World";
	    expectedString << "";

	    QTest::newRow(QString(keys_str + "_helloworld").toLatin1()) <<
                insertString <<
                insertIndex <<
                insertMode <<
                expectedString <<
                bool(use_keys);
        }
        {
	    IntList insertIndex;
	    IntList insertMode;
	    QStringList insertString;
	    QStringList expectedString;

	    insertIndex << -1;
            insertMode << NORMAL;
	    insertString << "Ensuring";

            insertIndex << -1;
            insertMode << NORMAL;
	    insertString << " instan";

	    insertIndex << 9;
            insertMode << NORMAL;
	    insertString << "an ";

            insertIndex << 10;
            insertMode << REPLACE_UNTIL_END;
	    insertString << " unique instance.";

	    expectedString << "Ensuring a unique instance.";
	    expectedString << "Ensuring an instan";
	    expectedString << "Ensuring instan";
	    expectedString << "";

	    QTest::newRow(QString(keys_str + "_patterns").toLatin1()) <<
                insertString <<
                insertIndex <<
                insertMode <<
                expectedString <<
                bool(use_keys);
        }
    }
}

void tst_QLineEdit::undo()
{
    QFETCH(QStringList, insertString);
    QFETCH(IntList, insertIndex);
    QFETCH(IntList, insertMode);
    QFETCH(QStringList, expectedString);
    QFETCH(bool, use_keys);

    QVERIFY(!testWidget->isUndoAvailable());

    int i;

// STEP 1: First build up an undo history by inserting or typing some strings...
    for (i=0; i<insertString.size(); ++i) {
	if (insertIndex[i] > -1)
	    testWidget->setCursorPosition(insertIndex[i]);

 // experimental stuff
        if (insertMode[i] == REPLACE_UNTIL_END) {
            testWidget->setSelection(insertIndex[i], 8);

            // This is what I actually want...
// 	    QTest::keyClick(testWidget, Qt::Key_End, Qt::ShiftModifier);
        }

        if (use_keys)
            QTest::keyClicks(testWidget, insertString[i]);
        else
            testWidget->insert(insertString[i]);
    }

// STEP 2: Next call undo several times and see if we can restore to the previous state
    for (i=0; i<expectedString.size()-1; ++i) {
	QCOMPARE(testWidget->text(), expectedString[i]);
        QVERIFY(testWidget->isUndoAvailable());
	testWidget->undo();
    }

// STEP 3: Verify that we have undone everything
    QVERIFY(!testWidget->isUndoAvailable());
    QVERIFY(testWidget->text().isEmpty());

#ifdef Q_WS_WIN
    // Repeat the test using shortcut instead of undo()
    for (i=0; i<insertString.size(); ++i) {
	if (insertIndex[i] > -1)
	    testWidget->setCursorPosition(insertIndex[i]);
        if (insertMode[i] == REPLACE_UNTIL_END) {
            testWidget->setSelection(insertIndex[i], 8);
        }
        if (use_keys)
            QTest::keyClicks(testWidget, insertString[i]);
        else
            testWidget->insert(insertString[i]);
    }
    for (i=0; i<expectedString.size()-1; ++i) {
	QCOMPARE(testWidget->text(), expectedString[i]);
        QVERIFY(testWidget->isUndoAvailable());
        QTest::keyClick(testWidget, Qt::Key_Backspace, Qt::AltModifier);
    }
#endif
}

void tst_QLineEdit::isUndoAvailable()
{
    DEPENDS_ON("undo");
}

void tst_QLineEdit::redo_data()
{
    QTest::addColumn<QStringList>("insertString");
    QTest::addColumn<IntList>("insertIndex");
    QTest::addColumn<QStringList>("expectedString");

    {
	IntList insertIndex;
	QStringList insertString;
	QStringList expectedString;

	insertIndex << -1;
	insertString << "World"; // World
	insertIndex << 0;
	insertString << "Hello"; // HelloWorld
	insertIndex << 0;
	insertString << "Well"; // WellHelloWorld
	insertIndex << 9;
	insertString << "There"; // WellHelloThereWorld;

	expectedString << "World";
	expectedString << "HelloWorld";
	expectedString << "WellHelloWorld";
	expectedString << "WellHelloThereWorld";

	QTest::newRow("Inserts and setting cursor") << insertString << insertIndex << expectedString;
    }
}

void tst_QLineEdit::redo()
{
    QFETCH(QStringList, insertString);
    QFETCH(IntList, insertIndex);
    QFETCH(QStringList, expectedString);

    QVERIFY(!testWidget->isUndoAvailable());
    QVERIFY(!testWidget->isRedoAvailable());

    int i;
    // inserts the diff strings at diff positions
    for (i=0; i<insertString.size(); ++i) {
	if (insertIndex[i] > -1)
	    testWidget->setCursorPosition(insertIndex[i]);
	testWidget->insert(insertString[i]);
    }

    QVERIFY(!testWidget->isRedoAvailable());

    // undo everything
    while (!testWidget->text().isEmpty())
	testWidget->undo();

    for (i=0; i<expectedString.size(); ++i) {
	QVERIFY(testWidget->isRedoAvailable());
        testWidget->redo();
	QCOMPARE(testWidget->text() , expectedString[i]);
    }

    QVERIFY(!testWidget->isRedoAvailable());

#ifdef Q_WS_WIN
    // repeat test, this time using shortcuts instead of undo()/redo()

    while (!testWidget->text().isEmpty())
        QTest::keyClick(testWidget, Qt::Key_Backspace, Qt::AltModifier);

    for (i = 0; i < expectedString.size(); ++i) {
	QVERIFY(testWidget->isRedoAvailable());
        QTest::keyClick(testWidget, Qt::Key_Backspace,
                        Qt::ShiftModifier | Qt::AltModifier);
	QCOMPARE(testWidget->text() , expectedString[i]);
    }

    QVERIFY(!testWidget->isRedoAvailable());
#endif
}

void tst_QLineEdit::isRedoAvailable()
{
    DEPENDS_ON("redo");
}

void tst_QLineEdit::undo_keypressevents_data()
{
    QTest::addColumn<QTestEventList>("keys");
    QTest::addColumn<QStringList>("expectedString");

    {
	QTestEventList keys;
	QStringList expectedString;

        keys.addKeyClick('A');
        keys.addKeyClick('F');
        keys.addKeyClick('R');
        keys.addKeyClick('A');
        keys.addKeyClick('I');
        keys.addKeyClick('D');
        psKeyClick(keys, Qt::Key_Home);

	keys.addKeyClick('V');
	keys.addKeyClick('E');
	keys.addKeyClick('R');
	keys.addKeyClick('Y');

        keys.addKeyClick(Qt::Key_Left);
	keys.addKeyClick(Qt::Key_Left);
	keys.addKeyClick(Qt::Key_Left);
	keys.addKeyClick(Qt::Key_Left);

	keys.addKeyClick('B');
	keys.addKeyClick('E');
        psKeyClick(keys, Qt::Key_End);

	keys.addKeyClick(Qt::Key_Exclam);

	expectedString << "BEVERYAFRAID!";
	expectedString << "BEVERYAFRAID";
	expectedString << "VERYAFRAID";
        expectedString << "AFRAID";

	QTest::newRow("Inserts and moving cursor") << keys << expectedString;
    }

    {
	QTestEventList keys;
	QStringList expectedString;

	// inserting '1234'
	keys.addKeyClick(Qt::Key_1);
	keys.addKeyClick(Qt::Key_2);
	keys.addKeyClick(Qt::Key_3);
	keys.addKeyClick(Qt::Key_4);
        psKeyClick(keys, Qt::Key_Home);

        // skipping '12'
	keys.addKeyClick(Qt::Key_Right);
	keys.addKeyClick(Qt::Key_Right);

        // selecting '34'
	keys.addKeyClick(Qt::Key_Right, Qt::ShiftModifier);
	keys.addKeyClick(Qt::Key_Right, Qt::ShiftModifier);

        // deleting '34'
	keys.addKeyClick(Qt::Key_Delete);

	expectedString << "12";
	expectedString << "1234";

	QTest::newRow("Inserts,moving,selection and delete") << keys << expectedString;
    }

    {
	QTestEventList keys;
	QStringList expectedString;

	// inserting 'AB12'
	keys.addKeyClick('A');
	keys.addKeyClick('B');

        keys.addKeyClick(Qt::Key_1);
	keys.addKeyClick(Qt::Key_2);

        psKeyClick(keys, Qt::Key_Home);

        // selecting 'AB'
	keys.addKeyClick(Qt::Key_Right, Qt::ShiftModifier);
	keys.addKeyClick(Qt::Key_Right, Qt::ShiftModifier);

        // deleting 'AB'
	keys.addKeyClick(Qt::Key_Delete);

        // undoing deletion of 'AB'
	keys.addKeyClick(Qt::Key_Z, Qt::ControlModifier);

        // selecting '12'
	keys.addKeyClick(Qt::Key_Right, Qt::ShiftModifier);
	keys.addKeyClick(Qt::Key_Right, Qt::ShiftModifier);

        // deleting '12'
	keys.addKeyClick(Qt::Key_Delete);

	expectedString << "AB";
	expectedString << "AB12";

	QTest::newRow("Inserts,moving,selection, delete and undo") << keys << expectedString;
    }

    {
	QTestEventList keys;
	QStringList expectedString;

	// inserting '123'
	keys.addKeyClick(Qt::Key_1);
	keys.addKeyClick(Qt::Key_2);
	keys.addKeyClick(Qt::Key_3);
        psKeyClick(keys, Qt::Key_Home);

	// selecting '123'
        psKeyClick(keys, Qt::Key_End, Qt::ShiftModifier);

        // overwriting '123' with 'ABC'
	keys.addKeyClick('A');
	keys.addKeyClick('B');
	keys.addKeyClick('C');

	expectedString << "ABC";
	// for versions previous to 3.2 we overwrite needed two undo operations
	expectedString << "123";

	QTest::newRow("Inserts,moving,selection and overwriting") << keys << expectedString;
    }
}

void tst_QLineEdit::undo_keypressevents()
{
    QFETCH(QTestEventList, keys);
    QFETCH(QStringList, expectedString);

    keys.simulate(testWidget);

    for (int i=0; i<expectedString.size(); ++i) {
	QCOMPARE(testWidget->text() , expectedString[i]);
	testWidget->undo();
    }
    QVERIFY(testWidget->text().isEmpty());
}

void tst_QLineEdit::clear()
{
    // checking that clear of empty/nullstring doesn't add to undo history
    int max = 5000;
    while (max > 0 && testWidget->isUndoAvailable()) {
        max--;
        testWidget->undo();
    }

    testWidget->clear();
//    QVERIFY(!testWidget->isUndoAvailable());

    // checks that clear actually clears
    testWidget->insert("I am Legend");
    testWidget->clear();
    QVERIFY(testWidget->text().isEmpty());

    // checks that clears can be undone
    testWidget->undo();
    QCOMPARE(testWidget->text(), QString("I am Legend"));
}

#ifdef QT3_SUPPORT
void tst_QLineEdit::lostFocus()
{
    editingFinished();
}
#endif
void tst_QLineEdit::editingFinished()
{
    if (testWidget->hasAcceptableInput())
	validInput = TRUE;
    else
	validInput = FALSE;
}

void tst_QLineEdit::text_data()
{
    QTest::addColumn<QString>("insertString");

    QTest::newRow("Plain text0") << QString("Hello World");
    QTest::newRow("Plain text1") << QString("");
    QTest::newRow("Plain text2") << QString("A");
    QTest::newRow("Plain text3") << QString("ryyryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryry");
    QTest::newRow("Plain text4") << QString("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890`~!@#$%^&*()_-+={[}]|\\:;'?/>.<,\"");
    QTest::newRow("Newlines") << QString("A\nB\nC\n");
    QTest::newRow("Text with nbsp") << QString("Hello") + QChar(0xa0) + "World";
}

void tst_QLineEdit::text()
{
    QFETCH(QString, insertString);
    testWidget->setText(insertString);
    QCOMPARE(testWidget->text(), insertString);
}

void tst_QLineEdit::textMask_data()
{
    QTest::addColumn<QString>("insertString");

    QTest::newRow( "Plain text1" ) << QString( "" );
}

void tst_QLineEdit::textMask()
{
#if (QT_VERSION-0 >= 0x030303)
    QFETCH( QString, insertString );
    testWidget->setInputMask( "#" );
    testWidget->setText( insertString );
    QCOMPARE( testWidget->text(), insertString );
#else
    QSKIP( "This test function tests a problem with masks that was fixed in 3.3", SkipAll);
#endif
}

void tst_QLineEdit::setText()
{
    QSignalSpy editedSpy(testWidget, SIGNAL(textEdited(QString)));
    QSignalSpy changedSpy(testWidget, SIGNAL(textChanged(QString)));
    testWidget->setText("hello");
    QCOMPARE(editedSpy.count(), 0);
    QCOMPARE(changedSpy.value(0).value(0).toString(), QString("hello"));
}

void tst_QLineEdit::displayText_data()
{
    QTest::addColumn<QString>("insertString");
    QTest::addColumn<QString>("expectedString");
    QTest::addColumn<QLineEdit::EchoMode>("mode");
    QTest::addColumn<bool>("use_setText");

    QString s;
    QLineEdit::EchoMode m;

    for (int i=0; i<2; i++) {
        QString key_mode_str;
        bool use_setText;
        if (i==0) {
            key_mode_str = "setText_";
            use_setText = TRUE;
        } else {
            key_mode_str = "useKeys_";
            use_setText = FALSE;
        }
        s = key_mode_str + "Normal";
        m = QLineEdit::Normal;
        QTest::newRow(QString(s + " text0").toLatin1()) << QString("Hello World") <<
				      QString("Hello World") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text1").toLatin1()) << QString("") <<
				      QString("") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text2").toLatin1()) << QString("A") <<
				      QString("A") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text3").toLatin1()) << QString("ryyryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryry") <<
				      QString("ryyryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryry") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text4").toLatin1()) << QString("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890`~!@#$%^&*()_-+={[}]|\\:;'?/>.<,\"") <<
				      QString("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890`~!@#$%^&*()_-+={[}]|\\:;'?/>.<,\"") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text with nbsp").toLatin1()) << QString("Hello") + QChar(0xa0) + "World" <<
					       QString("Hello") + QChar(0xa0) + "World" <<
					       m << bool(use_setText);
        s = key_mode_str + "NoEcho";
        m = QLineEdit::NoEcho;
        QTest::newRow(QString(s + " text0").toLatin1()) << QString("Hello World") <<
				      QString("") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text1").toLatin1()) << QString("") <<
				      QString("") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text2").toLatin1()) << QString("A") <<
				      QString("") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text3").toLatin1()) << QString("ryyryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryry") <<
				      QString("") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text4").toLatin1()) << QString("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890`~!@#$%^&*()_-+={[}]|\\:;'?/>.<,\"") <<
				      QString("") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text with nbsp").toLatin1()) << QString("Hello") + QChar(0xa0) + "World" <<
					       QString("") <<
					       m << bool(use_setText);
        s = key_mode_str + "Password";
        m = QLineEdit::Password;
        QChar passChar = qApp->style()->styleHint(QStyle::SH_LineEdit_PasswordCharacter, 0, testWidget);
        QString input;
        QString pass;
        input = "Hello World";
        pass.resize(input.length());
        pass.fill(passChar);
        QTest::newRow(QString(s + " text0").toLatin1()) << input << pass << m << bool(use_setText);
        QTest::newRow(QString(s + " text1").toLatin1()) << QString("") <<
				      QString("") <<
				      m << bool(use_setText);
        QTest::newRow(QString(s + " text2").toLatin1()) << QString("A") << QString(passChar) << m << bool(use_setText);
        input = QString("ryyryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryryrryryryryryryryryryryryryry");
        pass.resize(input.length());
        pass.fill(passChar);
        QTest::newRow(QString(s + " text3").toLatin1()) << input << pass << m << bool(use_setText);
        input = QString("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890`~!@#$%^&*()_-+={[}]|\\:;'?/>.<,\"");
        pass.fill(passChar, input.length());
        QTest::newRow(QString(s + " text4").toLatin1()) << input << pass << m << bool(use_setText);
        input = QString("Hello") + QChar(0xa0) + "World";
        pass.resize(input.length());
        pass.fill(passChar);
        QTest::newRow(QString(s + " text with nbsp").toLatin1()) << input << pass << m << bool(use_setText);
    }
}

void tst_QLineEdit::displayText()
{
    QFETCH(QString, insertString);
    QFETCH(QString, expectedString);
    QFETCH(QLineEdit::EchoMode, mode);
    //QFETCH(bool, use_setText);  Currently unused.

    testWidget->setEchoMode(mode);
    testWidget->setText(insertString);
    QCOMPARE(testWidget->displayText(), expectedString);
    QVERIFY(testWidget->echoMode() == mode);
}

void tst_QLineEdit::setEchoMode()
{
    DEPENDS_ON("displayText");
}

void tst_QLineEdit::echoMode()
{
    DEPENDS_ON("displayText");
}

void tst_QLineEdit::maxLength_mask_data()
{
    QTest::addColumn<QString>("mask");
    QTest::addColumn<int>("expectedLength");

    QTest::newRow("mask_case") << QString(">000<>00<000") << 8;
    QTest::newRow("mask_nocase") << QString("00000000") << 8;
    QTest::newRow("mask_null") << QString() << 32767;
    QTest::newRow("mask_escape") << QString("\\A\\aAA") << 4;
}

void tst_QLineEdit::maxLength_mask()
{
    QFETCH(QString, mask);
    QFETCH(int, expectedLength);

    testWidget->setInputMask(mask);

    QCOMPARE(testWidget->maxLength(), expectedLength);
}

void tst_QLineEdit::maxLength_data()
{
    QTest::addColumn<QString>("insertString");
    QTest::addColumn<QString>("expectedString");
    QTest::addColumn<int>("length");
    QTest::addColumn<bool>("insertBeforeSettingMaxLength");
    QTest::addColumn<bool>("use_setText");

    QTest::newRow("keyclick before0") << QString("this is a test.") << QString("this is a test.") << 20 << bool(TRUE) << bool(FALSE);
    QTest::newRow("keyclick before1") << QString("this is a test.") << QString("this is a ") << 10 << bool(TRUE) << bool(FALSE);
    QTest::newRow("keyclick after0") << QString("this is a test.") << QString("this is a test.") << 20 << bool(FALSE) << bool(FALSE);
    QTest::newRow("keyclick after1") << QString("this is a test.") << QString("this is a ") << 10 << bool(FALSE) << bool(FALSE);
    QTest::newRow("settext before0") << QString("this is a test.") << QString("this is a test.") << 20 << bool(TRUE) << bool(TRUE);
    QTest::newRow("settext before1") << QString("this is a test.") << QString("this is a ") << 10 << bool(TRUE) << bool(TRUE);
    QTest::newRow("settext after0") << QString("this is a test.") << QString("this is a test.") << 20 << bool(FALSE) << bool(TRUE);
    QTest::newRow("settext after1") << QString("this is a test.") << QString("this is a ") << 10 << bool(FALSE) << bool(TRUE);
}

void tst_QLineEdit::maxLength()
{
    QFETCH(QString, insertString);
    QFETCH(QString, expectedString);
    QFETCH(int, length);
    QFETCH(bool, insertBeforeSettingMaxLength);
    QFETCH(bool, use_setText);

    // in some cases we set the maxLength _before_ entering the text.
    if (!insertBeforeSettingMaxLength)
	testWidget->setMaxLength(length);

    // I expect MaxLength to work BOTH with entering live characters AND with setting the text.
    if (use_setText) {
	// Enter insertString using setText.
	testWidget->setText(insertString);
    } else {
	// Enter insertString as a sequence of keyClicks
	QTest::keyClicks(testWidget, insertString);
    }

    // in all other cases we set the maxLength _after_ entering the text.
    if (insertBeforeSettingMaxLength) {
	changed_count = 0;
	testWidget->setMaxLength(length);

	// Make sure that the textChanged is not emitted unless the text is actually changed
	if (insertString == expectedString) {
	    QVERIFY(changed_count == 0);
	} else {
	    QVERIFY(changed_count == 1);
	}
    }

    // and check if we get the expected string back
    QCOMPARE(testWidget->text(), expectedString);
}

void tst_QLineEdit::setMaxLength()
{
    DEPENDS_ON("maxLength");
}

void tst_QLineEdit::isReadOnly()
{
    QVERIFY(!testWidget->isReadOnly());

    // start with a basic text
    QTest::keyClicks(testWidget, "the quick brown fox");
    QCOMPARE(testWidget->text(), QString("the quick brown fox"));

    // do a quick check to verify that we can indeed edit the text
    testWidget->home(FALSE);
    testWidget->cursorForward(FALSE, 10);
    QTest::keyClicks(testWidget, "dark ");
    QCOMPARE(testWidget->text(), QString("the quick dark brown fox"));

    testWidget->setReadOnly(TRUE);
    QVERIFY(testWidget->isReadOnly());

    // verify that we cannot edit the text anymore
    testWidget->home(FALSE);
    testWidget->cursorForward(FALSE, 10);
    QTest::keyClick(testWidget, Qt::Key_Delete);
    QCOMPARE(testWidget->text(), QString("the quick dark brown fox"));
    testWidget->cursorForward(FALSE, 10);
    QTest::keyClicks(testWidget, "this should not have any effect!! ");
    QCOMPARE(testWidget->text(), QString("the quick dark brown fox"));
}

void tst_QLineEdit::setReadOnly()
{
    DEPENDS_ON("isReadOnly");
}

static void figureOutProperKey(Qt::Key &key, Qt::KeyboardModifiers &pressState)
{
#ifdef Q_WS_MAC
    static bool tst_lineedit_randomized = false;
    // Mac has 3 different ways of accomplishing this (same for moving to the back)
    // So I guess we should just randomly do this. Which may get people mad, but if
    // we fail at one point, it's just a matter of setting roll to the correct value
    // insteadt of random.
    if (!tst_lineedit_randomized) {
        tst_lineedit_randomized = true;
        ::srandom(ulong(time(0)));
    }
    long roll = ::random() % 3;
    switch (roll) {
    case 0:
        key = key == Qt::Key_Home ? Qt::Key_Up : Qt::Key_Down;
        break;
    case 1:
    case 2:
        key = key == Qt::Key_Home ? Qt::Key_Left : Qt::Key_Right;
        pressState |= (roll == 1) ? Qt::ControlModifier : Qt::MetaModifier;
        break;
    }
#else
    // Naively kill the warning.
    key = key;
    pressState = pressState;
#endif
}

// Platform specific move. Home and End do nothing on the Mac,
// so do something a bit smarter than tons of #ifdefs
void tst_QLineEdit::psKeyClick(QWidget *target, Qt::Key key, Qt::KeyboardModifiers pressState)
{
    figureOutProperKey(key, pressState);
    QTest::keyClick(target, key, pressState);
}

void tst_QLineEdit::psKeyClick(QTestEventList &keys, Qt::Key key, Qt::KeyboardModifiers pressState)
{
    figureOutProperKey(key, pressState);
    keys.addKeyClick(key, pressState);
}

void tst_QLineEdit::cursorPosition()
{
    QVERIFY(testWidget->cursorPosition() == 0);

    // start with a basic text
    QTest::keyClicks(testWidget, "The");
    QCOMPARE(testWidget->cursorPosition(), 3);
    QTest::keyClicks(testWidget, " quick");
    QCOMPARE(testWidget->cursorPosition(), 9);
    QTest::keyClicks(testWidget, " brown fox jumps over the lazy dog");
    QCOMPARE(testWidget->cursorPosition(), 43);

    // The text we have now is:
    //           1         2         3         4         5
    // 012345678901234567890123456789012345678901234567890
    // The quick brown fox jumps over the lazy dog

    // next we will check some of the cursor movement functions
    testWidget->end(FALSE);
    QCOMPARE(testWidget->cursorPosition() , 43);
    testWidget->cursorForward(FALSE, -1);
    QCOMPARE(testWidget->cursorPosition() , 42);
    testWidget->cursorBackward(FALSE, 1);
    QCOMPARE(testWidget->cursorPosition() , 41);
    testWidget->home(FALSE);
    QCOMPARE(testWidget->cursorPosition() , 0);
    testWidget->cursorForward(FALSE, 1);
    QCOMPARE(testWidget->cursorPosition() , 1);
    testWidget->cursorForward(FALSE, 9);
    QCOMPARE(testWidget->cursorPosition() , 10);
    testWidget->cursorWordForward(FALSE); // 'fox'
    QCOMPARE(testWidget->cursorPosition(), 16);
    testWidget->cursorWordForward(FALSE); // 'jumps'
    QCOMPARE(testWidget->cursorPosition(), 20);
    testWidget->cursorWordBackward(FALSE); // 'fox'
    QCOMPARE(testWidget->cursorPosition(), 16);
    testWidget->cursorWordBackward(FALSE); // 'brown'
    testWidget->cursorWordBackward(FALSE); // 'quick'
    testWidget->cursorWordBackward(FALSE); // 'The'
    QCOMPARE(testWidget->cursorPosition(), 0);
    testWidget->cursorWordBackward(FALSE); // 'The'
    QCOMPARE(testWidget->cursorPosition(), 0);

    // Cursor position should be 0 here!
    int i;
    for (i=0; i<5; i++)
	QTest::keyClick(testWidget, Qt::Key_Right);
    QCOMPARE(testWidget->cursorPosition(), 5);
    psKeyClick(testWidget, Qt::Key_End);
    QCOMPARE(testWidget->cursorPosition(), 43);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QCOMPARE(testWidget->cursorPosition(), 42);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QCOMPARE(testWidget->cursorPosition(), 41);
    psKeyClick(testWidget, Qt::Key_Home);
    QCOMPARE(testWidget->cursorPosition(), 0);

    // cursorposition when maxlength is set
    int maxLength = 9;
    testWidget->clear();
    testWidget->setMaxLength(maxLength);
    QCOMPARE(testWidget->cursorPosition() , 0);
    QTest::keyClicks(testWidget, "123ABC123");
    QCOMPARE(testWidget->cursorPosition() , maxLength);
    psKeyClick(testWidget, Qt::Key_Home);
    QCOMPARE(testWidget->cursorPosition() , 0);
    psKeyClick(testWidget, Qt::Key_End);
    QCOMPARE(testWidget->cursorPosition() , maxLength);
}

/* // tested in cursorPosition
void tst_QLineEdit::cursorLeft()
void tst_QLineEdit::cursorRight()
void tst_QLineEdit::cursorForward()
void tst_QLineEdit::cursorBackward()
void tst_QLineEdit::cursorWordForward()
void tst_QLineEdit::cursorWordBackward()
void tst_QLineEdit::home()
void tst_QLineEdit::end()
*/

void tst_QLineEdit::cursorPositionChanged_data()
{
    QTest::addColumn<QTestEventList>("input");
    QTest::addColumn<int>("lastPos");
    QTest::addColumn<int>("newPos");

    QTestEventList keys;
    keys.addKeyClick(Qt::Key_A);
    QTest::newRow("a") << keys << 0 << 1;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    psKeyClick(keys, Qt::Key_Home);
    QTest::newRow("abc<home>") << keys << 3 << 0;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Left);
    QTest::newRow("abc<left>") << keys << 3 << 2;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Right);
    QTest::newRow("abc<right>") << keys << 2 << 3;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    psKeyClick(keys, Qt::Key_Home);
    keys.addKeyClick(Qt::Key_Right);
    QTest::newRow("abc<home><right>") << keys << 0 << 1;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Backspace);
    QTest::newRow("abc<backspace>") << keys << 3 << 2;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Delete);
    QTest::newRow("abc<delete>") << keys << 2 << 3;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Left);
    keys.addKeyClick(Qt::Key_Delete);
    QTest::newRow("abc<left><delete>") << keys << 3 << 2;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    psKeyClick(keys, Qt::Key_Home);
    psKeyClick(keys, Qt::Key_End);
    QTest::newRow("abc<home><end>") << keys << 0 << 3;
    keys.clear();

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Space);
    keys.addKeyClick(Qt::Key_D);
    keys.addKeyClick(Qt::Key_E);
    keys.addKeyClick(Qt::Key_F);
    keys.addKeyClick(Qt::Key_Home);
    keys.addKeyClick(Qt::Key_Right, Qt::ControlModifier);
    QTest::newRow("abc efg<home><ctrl-right>") << keys
#ifndef Q_WS_MAC
        << 0 << 4;
#else
        << 6 << 7;
#endif
    keys.clear();

#ifdef Q_WS_MAC
    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Space);
    keys.addKeyClick(Qt::Key_D);
    keys.addKeyClick(Qt::Key_E);
    keys.addKeyClick(Qt::Key_F);
    keys.addKeyClick(Qt::Key_Up);
    keys.addKeyClick(Qt::Key_Right, Qt::AltModifier);
    QTest::newRow("mac equivalent abc efg<up><option-right>") << keys << 0 << 4;
    keys.clear();
#endif

    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Space);
    keys.addKeyClick(Qt::Key_D);
    keys.addKeyClick(Qt::Key_E);
    keys.addKeyClick(Qt::Key_F);
    keys.addKeyClick(Qt::Key_Left, Qt::ControlModifier);
    QTest::newRow("abc efg<ctrl-left>") << keys << 7
#ifndef Q_WS_MAC
        << 4;
#else
        << 0;
#endif
    keys.clear();
#ifdef Q_WS_MAC
    keys.addKeyClick(Qt::Key_A);
    keys.addKeyClick(Qt::Key_B);
    keys.addKeyClick(Qt::Key_C);
    keys.addKeyClick(Qt::Key_Space);
    keys.addKeyClick(Qt::Key_D);
    keys.addKeyClick(Qt::Key_E);
    keys.addKeyClick(Qt::Key_F);
    keys.addKeyClick(Qt::Key_Left, Qt::AltModifier);
    QTest::newRow("mac equivalent abc efg<option-left>") << keys << 7 << 4;
    keys.clear();
#endif
}


void tst_QLineEdit::cursorPositionChanged()
{
    QFETCH(QTestEventList, input);
    QFETCH(int, lastPos);
    QFETCH(int, newPos);

    lastCursorPos = 0;
    newCursorPos = 0;
    input.simulate(testWidget);
    QCOMPARE(lastCursorPos, lastPos);
    QCOMPARE(newCursorPos, newPos);
}

void tst_QLineEdit::selectedText()
{
    int i;

    QString testString = "Abc defg hijklmno, p 'qrst' uvw xyz";

    // start with a basic text
    testWidget->setText(testString);
    selection_count = 0;

    // The text we have now is:
    //           1         2         3         4         5
    // 012345678901234567890123456789012345678901234567890
    // Abc defg hijklmno, p 'qrst' uvw xyz

    testWidget->home(FALSE);
    QVERIFY(!testWidget->hasSelectedText());
    QCOMPARE(testWidget->selectedText(), QString());

    // play a bit with the cursorForward, cursorBackward(), etc
    testWidget->cursorForward(TRUE, 9);
    QVERIFY(testWidget->hasSelectedText());
    QCOMPARE(testWidget->selectedText(), QString("Abc defg "));
    QVERIFY(selection_count == 1);

    testWidget->cursorForward(TRUE, 2);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hi"));
    QVERIFY(selection_count == 2);

    testWidget->cursorWordForward(TRUE);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hijklmno"));
    QVERIFY(selection_count == 3);

    testWidget->cursorWordForward(TRUE);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hijklmno, "));
    QVERIFY(selection_count == 4);

    testWidget->cursorWordForward(TRUE);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hijklmno, p "));
    QVERIFY(selection_count == 5);

    testWidget->cursorWordBackward(TRUE);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hijklmno, "));
    QVERIFY(selection_count == 6);

    testWidget->cursorWordBackward(TRUE);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg "));
    QVERIFY(selection_count == 7);

    testWidget->cursorWordForward(TRUE);
    testWidget->cursorWordForward(TRUE);
    testWidget->cursorWordForward(TRUE);
    testWidget->cursorWordForward(TRUE);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hijklmno, p 'qrst' "));
    QVERIFY(selection_count == 11);

    // reset selection
    testWidget->home(FALSE);
    QVERIFY(!testWidget->hasSelectedText());
    QCOMPARE(testWidget->selectedText(), QString());
    selection_count = 0;

    // now try the keyboard
    QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QVERIFY(testWidget->hasSelectedText());
    QVERIFY(selection_count == 1);

    QCOMPARE(testWidget->selectedText(), QString("A"));
    for (i=0; i<5; i++)
	QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QVERIFY(testWidget->hasSelectedText());
    QVERIFY(selection_count == 6);

    QCOMPARE(testWidget->selectedText(), QString("Abc de"));
    psKeyClick(testWidget, Qt::Key_End, Qt::ShiftModifier);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hijklmno, p 'qrst' uvw xyz"));
    psKeyClick(testWidget, Qt::Key_Home);
    QCOMPARE(testWidget->selectedText(), QString());

    // go somewhere in the middle and select 'backwards'
    for (i=0; i<8; i++)
	QTest::keyClick(testWidget, Qt::Key_Right);
    psKeyClick(testWidget, Qt::Key_Home, Qt::ShiftModifier);
    QCOMPARE(testWidget->selectedText(), QString("Abc defg"));

    // go to the middle and select 'forwards'
    psKeyClick(testWidget, Qt::Key_End);
    for (i=0; i<7; i++)
	QTest::keyClick(testWidget, Qt::Key_Left);
    psKeyClick(testWidget, Qt::Key_End, Qt::ShiftModifier);
    QCOMPARE(testWidget->selectedText(), QString("uvw xyz"));

    // select some text and use backspace
    psKeyClick(testWidget, Qt::Key_Home);
    QVERIFY(!testWidget->hasSelectedText());
    for (i=0; i<8; i++)
	QTest::keyClick(testWidget, Qt::Key_Right);
    for (i=0; i<9; i++)
	QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QVERIFY(testWidget->hasSelectedText());
    testWidget->backspace();
    QVERIFY(!testWidget->hasSelectedText());
    QCOMPARE(testWidget->text(), QString("Abc defg, p 'qrst' uvw xyz"));
    testWidget->backspace();
    QCOMPARE(testWidget->text(), QString("Abc def, p 'qrst' uvw xyz"));

    // select some text and use del
    testWidget->setText(testString);
    psKeyClick(testWidget, Qt::Key_Home);
    QVERIFY(!testWidget->hasSelectedText());
    for (i=0; i<21; i++)
	QTest::keyClick(testWidget, Qt::Key_Right);
    for (i=0; i<7; i++)
	QTest::keyClick(testWidget, Qt::Key_Right, Qt::ShiftModifier);
    QVERIFY(testWidget->hasSelectedText());
    testWidget->del();
    QVERIFY(!testWidget->hasSelectedText());
    QCOMPARE(testWidget->text(), QString("Abc defg hijklmno, p uvw xyz"));
    testWidget->del();
    QCOMPARE(testWidget->text(), QString("Abc defg hijklmno, p vw xyz"));

    testWidget->selectAll();
    QVERIFY(testWidget->hasSelectedText());
    QCOMPARE(testWidget->selectedText(), QString("Abc defg hijklmno, p vw xyz"));

    testWidget->deselect();
    QVERIFY(!testWidget->hasSelectedText());
    QCOMPARE(testWidget->selectedText(), QString());

    // check selectionChanged on Home,End,Left and Right
    testWidget->setText(testString);
    QVERIFY(!testWidget->hasSelectedText());
    // Home
    testWidget->setSelection(0,3);
    QVERIFY(testWidget->hasSelectedText());
    selection_count = 0;
    psKeyClick(testWidget, Qt::Key_Home);
    QVERIFY(selection_count == 1);
    QVERIFY(!testWidget->hasSelectedText());
    // End
    testWidget->setSelection(0,3);
    QVERIFY(testWidget->hasSelectedText());
    selection_count = 0;
    psKeyClick(testWidget, Qt::Key_End);
    QVERIFY(selection_count == 1);
    QVERIFY(!testWidget->hasSelectedText());
    // Left
    testWidget->setSelection(0,3);
    QVERIFY(testWidget->hasSelectedText());
    selection_count = 0;
    QTest::keyClick(testWidget, Qt::Key_Left);
    QVERIFY(selection_count == 1);
    QVERIFY(!testWidget->hasSelectedText());
    // Right
    testWidget->setSelection(0,3);
    QVERIFY(testWidget->hasSelectedText());
    selection_count = 0;
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(selection_count == 1);
    QVERIFY(!testWidget->hasSelectedText());
}

/* // tested in selectedText
void tst_QLineEdit::backspace()
void tst_QLineEdit::del()
void tst_QLineEdit::selectionChanged()
void tst_QLineEdit::selectAll()
void tst_QLineEdit::deselect()
*/

void tst_QLineEdit::onSelectionChanged()
{
    selection_count++;
}

void tst_QLineEdit::hasSelectedText()
{
    DEPENDS_ON("selectedText");
}

void tst_QLineEdit::textChangedAndTextEdited()
{
    changed_count = 0;
    edited_count = 0;

    QTest::keyClick(testWidget, Qt::Key_A);
    QCOMPARE(changed_count, 1);
    QVERIFY(edited_count == changed_count);
    QTest::keyClick(testWidget, 'b');
    QCOMPARE(changed_count, 2);
    QVERIFY(edited_count == changed_count);
    QTest::keyClick(testWidget, 'c');
    QCOMPARE(changed_count, 3);
    QVERIFY(edited_count == changed_count);
    QTest::keyClick(testWidget, ' ');
    QCOMPARE(changed_count, 4);
    QVERIFY(edited_count == changed_count);
    QTest::keyClick(testWidget, 'd');
    QCOMPARE(changed_count, 5);
    QVERIFY(edited_count == changed_count);

    changed_count = 0;
    edited_count = 0;
    changed_string = QString::null;

    testWidget->setText("foo");
    QCOMPARE(changed_count, 1);
    QCOMPARE(edited_count, 0);
    QCOMPARE(changed_string, QString("foo"));

    changed_count = 0;
    edited_count = 0;
    changed_string = QString::null;

    testWidget->setText("");
    QCOMPARE(changed_count, 1);
    QCOMPARE(edited_count, 0);
    QVERIFY(changed_string.isEmpty());
    QVERIFY(!changed_string.isNull());
}

void tst_QLineEdit::onTextChanged(const QString &text)
{
    changed_count++;
    changed_string = text;
}

void tst_QLineEdit::onTextEdited(const QString &/*text*/)
{
    edited_count++;
}


void tst_QLineEdit::onCursorPositionChanged(int oldPos, int newPos)
{
    lastCursorPos = oldPos;
    newCursorPos = newPos;
}

void tst_QLineEdit::returnPressed()
{
    return_count = 0;

    QTest::keyClick(testWidget, Qt::Key_Return);
    QVERIFY(return_count == 1);
    return_count = 0;

    QTest::keyClick(testWidget, 'A');
    QVERIFY(return_count == 0);
    QTest::keyClick(testWidget, 'b');
    QVERIFY(return_count == 0);
    QTest::keyClick(testWidget, 'c');
    QVERIFY(return_count == 0);
    QTest::keyClick(testWidget, ' ');
    QVERIFY(return_count == 0);
    QTest::keyClick(testWidget, 'd');
    QVERIFY(return_count == 0);
    psKeyClick(testWidget, Qt::Key_Home);
    QVERIFY(return_count == 0);
    psKeyClick(testWidget, Qt::Key_End);
    QVERIFY(return_count == 0);
    QTest::keyClick(testWidget, Qt::Key_Escape);
    QVERIFY(return_count == 0);
    QTest::keyClick(testWidget, Qt::Key_Return);
    QVERIFY(return_count == 1);
}

// int validator that fixes all !isNumber to '0'
class QIntFixValidator : public QIntValidator {
public:
    QIntFixValidator(int min, int max, QObject *parent) : QIntValidator(min, max, parent) {}
    void fixup (QString &input) const {
	for (int i=0; i<input.length(); ++i)
	    if (!input.at(i).isNumber()) {
		input[(int)i] = QChar('0');
            }
    }
};

void tst_QLineEdit::returnPressed_maskvalidator_data() {
    QTest::addColumn<QString>("inputMask");
    QTest::addColumn<bool>("hasValidator");
    QTest::addColumn<QTestEventList>("input");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<bool>("returnPressed");

    {
	QTestEventList keys;
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
        keys.addKeyClick(Qt::Key_3);
	keys.addKeyClick(Qt::Key_Return);
	QTest::newRow("no mask, no validator, input '123<cr>'")
	    << QString()
	    << FALSE
	    << keys
	    << QString("123")
	    << TRUE;
    }
    {
	QTestEventList keys;
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
	keys.addKeyClick(Qt::Key_Return);
	QTest::newRow("mask '999', no validator, input '12<cr>'")
	    << QString("999")
	    << FALSE
	    << keys
	    << QString("12")
	    << FALSE;
    }
    {
	QTestEventList keys;
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
	keys.addKeyClick(Qt::Key_3);
	keys.addKeyClick(Qt::Key_Return);
	QTest::newRow("mask '999', no validator, input '123<cr>'")
	    << QString("999")
	    << FALSE
	    << keys
	    << QString("123")
	    << TRUE;
    }
    {
	QTestEventList keys;
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
        keys.addKeyClick(Qt::Key_3);
	keys.addKeyClick(Qt::Key_Return);
	QTest::newRow("no mask, intfix validator(0,999), input '123<cr>'")
	    << QString()
	    << TRUE
	    << keys
	    << QString("123")
	    << TRUE;
    }
    {
	QTestEventList keys;
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_7);
        keys.addKeyClick(Qt::Key_7);
        keys.addKeyClick(Qt::Key_7);
        keys.addKeyClick(Qt::Key_7);
	keys.addKeyClick(Qt::Key_Return);
	QTest::newRow("no mask, intfix validator(0,999), input '7777<cr>'")
	    << QString()
	    << TRUE
	    << keys
	    << QString("777")
	    << TRUE;
    }
    {
	QTestEventList keys;
        keys.addKeyClick(Qt::Key_Home);
        keys.addKeyClick(Qt::Key_1);
        keys.addKeyClick(Qt::Key_2);
	keys.addKeyClick(Qt::Key_Return);
	QTest::newRow("mask '999', intfix validator(0,999), input '12<cr>'")
	    << QString("999")
	    << TRUE
	    << keys
	    << QString("12")
	    << FALSE;
    }
    {
	QTestEventList keys;
        keys.addKeyClick(Qt::Key_Home);
	keys.addKeyClick(Qt::Key_Return);
	QTest::newRow("mask '999', intfix validator(0,999), input '<cr>'")
	    << QString("999")
	    << TRUE
	    << keys
	    << QString("000")
	    << TRUE;
    }
}

void tst_QLineEdit::returnPressed_maskvalidator()
{
    QFETCH(QString, inputMask);
    QFETCH(bool, hasValidator);
    QFETCH(QTestEventList, input);
    QFETCH(QString, expectedText);
    QFETCH(bool, returnPressed);

    QEXPECT_FAIL("mask '999', intfix validator(0,999), input '12<cr>'", "QIntValidator has changed behaviour. Does not accept spaces. Task 43082.", Abort);

    testWidget->setInputMask(inputMask);
    if (hasValidator)
	testWidget->setValidator(new QIntFixValidator(0, 999, testWidget));

    return_count = 0;
    input.simulate(testWidget);

    QCOMPARE(testWidget->text(), expectedText);
    QCOMPARE(return_count , returnPressed ? 1 : 0);
}

void tst_QLineEdit::onReturnPressed()
{
    return_count++;
}

void tst_QLineEdit::setValidator()
{
    // Verify that we can set and re-set a validator.
    QVERIFY(!testWidget->validator());

    QIntValidator iv1(0);
    testWidget->setValidator(&iv1);
    QCOMPARE(testWidget->validator(), static_cast<const QValidator*>(&iv1));

    testWidget->setValidator(0);
    QVERIFY(testWidget->validator() == 0);

    QIntValidator iv2(0, 99, 0);
    testWidget->setValidator(&iv2);
    QCOMPARE(testWidget->validator(), static_cast<const QValidator *>(&iv2));

    testWidget->setValidator(0);
    QVERIFY(testWidget->validator() == 0);
}

void tst_QLineEdit::validator()
{
    DEPENDS_ON("setValidator");
}

void tst_QLineEdit::clearValidator()
{
    DEPENDS_ON("setValidator");
}

void tst_QLineEdit::setValidator_QIntValidator_data()
{
    QTest::addColumn<int>("mini");
    QTest::addColumn<int>("maxi");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<bool>("useKeys");
    QTest::addColumn<bool>("is_valid");

    for (int i=0; i<2; i++) {
        bool useKeys = FALSE;
        QString inputMode = "insert ";
        if (i!=0) {
            inputMode = "useKeys ";
            useKeys = TRUE;
        }

        // valid data
        QTest::newRow(QString(inputMode + "range [0,9] valid '1'").toLatin1())
	    << 0
            << 9
            << QString("1")
            << QString("1")
            << bool(useKeys)
            << bool(TRUE);

        QTest::newRow(QString(inputMode + "range [3,7] valid '3'").toLatin1())
	    << 3
            << 7
            << QString("3")
            << QString("3")
            << bool(useKeys)
            << bool(TRUE);

        QTest::newRow(QString(inputMode + "range [3,7] valid '7'").toLatin1())
	    << 3
            << 7
            << QString("7")
            << QString("7")
            << bool(useKeys)
            << bool(TRUE);

        QTest::newRow(QString(inputMode + "range [0,100] valid '9'").toLatin1())
	    << 0
            << 100
            << QString("9")
            << QString("9")
            << bool(useKeys)
            << bool(TRUE);

        QTest::newRow(QString(inputMode + "range [0,100] valid '12'").toLatin1())
	    << 0
            << 100
            << QString("12")
            << QString("12")
            << bool(useKeys)
            << bool(TRUE);

        QTest::newRow(QString(inputMode + "range [-100,100] valid '-12'").toLatin1())
            << -100
            << 100
            << QString("-12")
            << QString("-12")
            << bool(useKeys)
            << bool(TRUE);

        // invalid data
	// characters not allowed in QIntValidator
        QTest::newRow(QString(inputMode + "range [0,9] inv 'a-a'").toLatin1())
	    << 0
            << 9
            << QString("a")
            << QString("")
            << bool(useKeys)
            << bool(FALSE);

        QTest::newRow(QString(inputMode + "range [0,9] inv 'A'").toLatin1())
	    << 0
            << 9
            << QString("A")
            << QString("")
            << bool(useKeys)
            << bool(FALSE);
	// minus sign only allowed with a range on the negative side
        QTest::newRow(QString(inputMode + "range [0,100] inv '-'").toLatin1())
	    << 0
            << 100
            << QString("-")
            << QString("")
            << bool(useKeys)
            << bool(FALSE);
        QTest::newRow(QString(inputMode + "range [0,100] inv '153'").toLatin1())
	    << 0
            << 100
            << QString("153")
            << QString(useKeys ? "15" : "")
	    << bool(useKeys)
	    << bool(useKeys ? TRUE : FALSE);
        QTest::newRow(QString(inputMode + "range [-100,100] inv '-153'").toLatin1())
            << -100
            << 100
            << QString("-153")
            << QString(useKeys ? "-15" : "")
            << bool(useKeys)
            << bool(useKeys ? TRUE : FALSE);
        QTest::newRow(QString(inputMode + "range [3,7] inv '2'").toLatin1())
	    << 3
            << 7
            << QString("2")
            << QString("2")
            << bool(useKeys)
            << bool(FALSE);

        QTest::newRow(QString(inputMode + "range [3,7] inv '8'").toLatin1())
	    << 3
            << 7
            << QString("8")
            << QString("")
            << bool(useKeys)
            << bool(FALSE);
    }
}

void tst_QLineEdit::setValidator_QIntValidator()
{
    QFETCH(int, mini);
    QFETCH(int, maxi);
    QFETCH(QString, input);
    QFETCH(QString, expectedText);
    QFETCH(bool, useKeys);
    QFETCH(bool, is_valid);

    QIntValidator intValidator(mini, maxi, 0);
    testWidget->setValidator(&intValidator);
    QVERIFY(testWidget->text().isEmpty());
//qDebug("1 input: '" + input + "' Exp: '" + expectedText + "'");

    // tests valid input
    if (!useKeys) {
        testWidget->insert(input);
    } else {
        QTest::keyClicks(testWidget, input);
        return_count = 0;
        QTest::keyClick(testWidget, Qt::Key_Return);
        QCOMPARE(return_count, int(is_valid)); // assuming that is_valid = TRUE equals 1
    }
//qDebug("2 input: '" + input + "' Exp: '" + expectedText + "'");
//    QCOMPARE(testWidget->displayText(), expectedText);
    QCOMPARE(testWidget->text(), expectedText);
}

#define NO_PIXMAP_TESTS

void tst_QLineEdit::frame_data()
{
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTest::addColumn<QPixmap>("noFrame");
    QTest::addColumn<QPixmap>("useFrame");

    QTest::newRow("win");
//#else
//    QTest::newRow("x11");
#endif
#endif
}

void tst_QLineEdit::frame()
{
    testWidget->setFrame(FALSE);
    // verify that the editor is shown without a frame
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "noFrame");
#endif
#endif
    QVERIFY(!testWidget->hasFrame());

    testWidget->setFrame(TRUE);
    // verify that the editor is shown with a frame
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "useFrame");
#endif
#endif
    QVERIFY(testWidget->hasFrame());
}

void tst_QLineEdit::alignment()
{
    DEPENDS_ON("setAlignment");
}

void tst_QLineEdit::setAlignment_data()
{
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTest::addColumn<QPixmap>("left");
    QTest::addColumn<QPixmap>("right");
    QTest::addColumn<QPixmap>("hcenter");
    QTest::addColumn<QPixmap>("auto");

    QTest::newRow("win");
//#else
//    QTest::newRow("x11");
#endif
#endif
}

void tst_QLineEdit::setAlignment()
{
    testWidget->setText("left");
    testWidget->setAlignment(Qt::AlignLeft);
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "left");
#endif
#endif
    QVERIFY(testWidget->alignment() == Qt::AlignLeft);

#ifdef QT3_SUPPORT
    testWidget->setText("auto");
    testWidget->setAlignment(Qt::AlignAuto);
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "auto");
#endif
#endif
#endif

    testWidget->setText("hcenter");
    testWidget->setAlignment(Qt::AlignHCenter);
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "hcenter");
#endif
#endif
    QVERIFY(testWidget->alignment() == Qt::AlignHCenter);

#ifdef QT3_SUPPORT
    testWidget->setText("auto");
    testWidget->setAlignment(Qt::AlignAuto);
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "auto");
#endif
#endif
#endif

    testWidget->setText("right");
    testWidget->setAlignment(Qt::AlignRight);
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "right");
#endif
#endif
    QVERIFY(testWidget->alignment() == Qt::AlignRight);

#ifdef QT3_SUPPORT
    testWidget->setText("auto");
    testWidget->setAlignment(Qt::AlignAuto);
#ifndef NO_PIXMAP_TESTS
#if defined Q_WS_WIN
    QTEST(testWidget, "auto");
#endif
#endif
    QVERIFY(testWidget->alignment() == Qt::AlignAuto);
#endif

    testWidget->setAlignment(Qt::AlignTop);
    QVERIFY(testWidget->alignment() != Qt::AlignTop);

    testWidget->setAlignment(Qt::AlignBottom);
    QVERIFY(testWidget->alignment() != Qt::AlignBottom);

    testWidget->setAlignment(Qt::AlignCenter);
    QVERIFY(testWidget->alignment() != Qt::AlignCenter);
}

void tst_QLineEdit::isModified()
{
    QVERIFY(!testWidget->isModified());
    testWidget->setText("bla");
    QVERIFY(!testWidget->isModified());

    psKeyClick(testWidget, Qt::Key_Home);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Left);
    QVERIFY(!testWidget->isModified());
    psKeyClick(testWidget, Qt::Key_End);
    QVERIFY(!testWidget->isModified());

    QTest::keyClicks(testWidget, "T");
    QVERIFY(testWidget->isModified());
    QTest::keyClicks(testWidget, "his is a string");
    QVERIFY(testWidget->isModified());

    testWidget->setText("");
    QVERIFY(!testWidget->isModified());
    testWidget->setText("foo");
    QVERIFY(!testWidget->isModified());
}

/*
    Obsolete function but as long as we provide it, it needs to work.
*/

void tst_QLineEdit::edited()
{
    QVERIFY(!testWidget->isModified());
    testWidget->setText("bla");
    QVERIFY(!testWidget->isModified());

    psKeyClick(testWidget, Qt::Key_Home);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Right);
    QVERIFY(!testWidget->isModified());
    QTest::keyClick(testWidget, Qt::Key_Left);
    QVERIFY(!testWidget->isModified());
    psKeyClick(testWidget, Qt::Key_End);
    QVERIFY(!testWidget->isModified());

    QTest::keyClicks(testWidget, "T");
    QVERIFY(testWidget->isModified());
    QTest::keyClicks(testWidget, "his is a string");
    QVERIFY(testWidget->isModified());

    testWidget->setModified(false);
    QVERIFY(!testWidget->isModified());

    testWidget->setModified(true);
    QVERIFY(testWidget->isModified());
}

/*
    Obsolete function but as long as we provide it, it needs to work.
*/

void tst_QLineEdit::setEdited()
{
    DEPENDS_ON("edited");
}

void tst_QLineEdit::insert()
{
    testWidget->insert("This");
    testWidget->insert(" is");
    testWidget->insert(" a");
    testWidget->insert(" test");

    QCOMPARE(testWidget->text(), QString("This is a test"));

    testWidget->cursorWordBackward(FALSE);
    testWidget->cursorBackward(FALSE, 1);
    testWidget->insert(" nice");
    QCOMPARE(testWidget->text(), QString("This is a nice test"));

    testWidget->setCursorPosition(-1);
    testWidget->insert("No Crash! ");
    QCOMPARE(testWidget->text(), QString("No Crash! This is a nice test"));
}

void tst_QLineEdit::setSelection_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("length");
    QTest::addColumn<int>("expectedCursor");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<bool>("expectedHasSelectedText");

    QString text = "Abc defg hijklmno, p 'qrst' uvw xyz";
    int start, length, pos;

    start = 0; length = 1; pos = 1;
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString("A") << TRUE;

    start = 0; length = 2; pos = 2;
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString("Ab") << TRUE;

    start = 0; length = 4; pos = 4;
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString("Abc ") << TRUE;

    start = -1; length = 0; pos = text.length();
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString() << FALSE;

    start = 34; length = 1; pos = 35;
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString("z") << TRUE;

    start = 34; length = 2; pos = 35;
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString("z") << TRUE;

    start = 34; length = -1; pos = 33;
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString("y") << TRUE;

    start = 1; length = -2; pos = 0;
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString("A") << TRUE;

    start = -1; length = -1; pos = text.length();
    QTest::newRow(QString("selection start: %1 length: %2").arg(start).arg(length).toLatin1())
	<< text << start << length << pos << QString() << FALSE;
}


void tst_QLineEdit::setSelection()
{
    QFETCH(QString, text);
    QFETCH(int, start);
    QFETCH(int, length);
    QFETCH(int, expectedCursor);
    QFETCH(QString, expectedText);
    QFETCH(bool, expectedHasSelectedText);

    testWidget->setText(text);
    testWidget->setSelection(start, length);
    QCOMPARE(testWidget->hasSelectedText(), expectedHasSelectedText);
    QCOMPARE(testWidget->selectedText(), expectedText);
    if (expectedCursor >= 0)
	QCOMPARE(testWidget->cursorPosition(), expectedCursor);
}

void tst_QLineEdit::cut()
{

    // test newlines in cut'n'paste
    testWidget->setText("A\nB\nC\n");
    testWidget->setSelection(0, 6);
    testWidget->cut();
    psKeyClick(testWidget, Qt::Key_Home);
    testWidget->paste();
    QCOMPARE(testWidget->text(), QString("A\nB\nC\n"));
    //                              1         2         3         4
    //                    01234567890123456789012345678901234567890
    testWidget->setText("Abc defg hijklmno");

    testWidget->setSelection(0, 3);
    testWidget->cut();
    QCOMPARE(testWidget->text(), QString(" defg hijklmno"));

    psKeyClick(testWidget, Qt::Key_End);
    testWidget->paste();
    QCOMPARE(testWidget->text(), QString(" defg hijklmnoAbc"));

    psKeyClick(testWidget, Qt::Key_Home);
    testWidget->del();
    QCOMPARE(testWidget->text(), QString("defg hijklmnoAbc"));

    testWidget->setSelection(0, 4);
    testWidget->copy();
    psKeyClick(testWidget, Qt::Key_End);
    testWidget->paste();
    QCOMPARE(testWidget->text(), QString("defg hijklmnoAbcdefg"));

    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, Qt::Key_Left);
    QTest::keyClick(testWidget, ' ');
    QCOMPARE(testWidget->text(), QString("defg hijklmno Abcdefg"));

    testWidget->setSelection(0, 5);
    testWidget->del();
    QCOMPARE(testWidget->text(), QString("hijklmno Abcdefg"));

    testWidget->end(FALSE);
    QTest::keyClick(testWidget, ' ');
    testWidget->paste();
    QCOMPARE(testWidget->text(), QString("hijklmno Abcdefg defg"));

    testWidget->home(FALSE);
    testWidget->cursorWordForward(TRUE);
    testWidget->cut();
    testWidget->end(FALSE);
    QTest::keyClick(testWidget, ' ');
    testWidget->paste();
    testWidget->cursorBackward(TRUE, 1);
    testWidget->cut();
    QCOMPARE(testWidget->text(), QString("Abcdefg defg hijklmno"));
}

void tst_QLineEdit::copy()
{
    DEPENDS_ON("cut");
}

void tst_QLineEdit::paste()
{
    DEPENDS_ON("cut");
}

class InputMaskValidator : public QValidator
{
public:
    InputMaskValidator(QObject *parent, const char *name = 0) : QValidator(parent) { setObjectName(name); }
    State validate(QString &text, int &pos) const
    {
	InputMaskValidator *that = (InputMaskValidator *)this;
	that->validateText = text;
	that->validatePos = pos;
	return Acceptable;
    }
    QString validateText;
    int validatePos;
};

void tst_QLineEdit::inputMaskAndValidator_data()
{
    QTest::addColumn<QString>("inputMask");
    QTest::addColumn<QTestEventList>("keys");
    QTest::addColumn<QString>("validateText");
    QTest::addColumn<int>("validatePos");

    QTestEventList inputKeys;
    inputKeys.addKeyClick(Qt::Key_1);
    inputKeys.addKeyClick(Qt::Key_2);

    QTest::newRow("task28291") << "000;_" << inputKeys << "12_" << 2;
}

void tst_QLineEdit::inputMaskAndValidator()
{
    QFETCH(QString, inputMask);
    QFETCH(QTestEventList, keys);
    QFETCH(QString, validateText);
    QFETCH(int, validatePos);

    InputMaskValidator imv(testWidget);
    testWidget->setValidator(&imv);

    testWidget->setInputMask(inputMask);
    keys.simulate(testWidget);

    QCOMPARE(imv.validateText, validateText);
    QCOMPARE(imv.validatePos, validatePos);
}

void tst_QLineEdit::maxLengthAndInputMask()
{
    // Really a test for #30447
    QVERIFY(testWidget->inputMask().isNull());
    testWidget->setMaxLength(10);
    QVERIFY(testWidget->maxLength() == 10);
    testWidget->setInputMask(QString::null);
    QVERIFY(testWidget->inputMask().isNull());
    QVERIFY(testWidget->maxLength() == 10);
}


class LineEdit : public QLineEdit
{
public:
    LineEdit() { state = Other; }

    void keyPressEvent(QKeyEvent *e)
    {
        QLineEdit::keyPressEvent(e);
        if (e->key() == Qt::Key_Enter) {
            state = e->isAccepted() ? Accepted : Ignored;
        } else {
            state = Other;
        }

    }
    enum State {
        Accepted,
        Ignored,
        Other
    };

    State state;
};

Q_DECLARE_METATYPE(LineEdit::State);
void tst_QLineEdit::returnPressedKeyEvent()
{
    LineEdit lineedit;
    lineedit.show();
    QCOMPARE((int)lineedit.state, (int)LineEdit::Other);
    QTest::keyClick(&lineedit, Qt::Key_Enter);
    QCOMPARE((int)lineedit.state, (int)LineEdit::Ignored);
    connect(&lineedit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
    QTest::keyClick(&lineedit, Qt::Key_Enter);
    QCOMPARE((int)lineedit.state, (int)LineEdit::Ignored);
    disconnect(&lineedit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
    QTest::keyClick(&lineedit, Qt::Key_Enter);
    QCOMPARE((int)lineedit.state, (int)LineEdit::Ignored);
    QTest::keyClick(&lineedit, Qt::Key_1);
    QCOMPARE((int)lineedit.state, (int)LineEdit::Other);
}

void tst_QLineEdit::keepSelectionOnTabFocusIn()
{
    testWidget->setText("hello world");
    {
        QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);
        QApplication::sendEvent(testWidget, &e);
    }
    QCOMPARE(testWidget->selectedText(), QString("hello world"));
    testWidget->setSelection(0, 5);
    QCOMPARE(testWidget->selectedText(), QString("hello"));
    {
        QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);
        QApplication::sendEvent(testWidget, &e);
    }
    QCOMPARE(testWidget->selectedText(), QString("hello"));
}

void tst_QLineEdit::readOnlyStyleOption()
{
    bool wasReadOnly = testWidget->isReadOnly();
    QStyle *oldStyle = testWidget->style();

    StyleOptionTestStyle myStyle;
    testWidget->setStyle(&myStyle);

    myStyle.setReadOnly(true);
    testWidget->setReadOnly(true);
    testWidget->repaint();
    qApp->processEvents();

    testWidget->setReadOnly(false);
    myStyle.setReadOnly(false);
    testWidget->repaint();
    qApp->processEvents();

    testWidget->setReadOnly(wasReadOnly);
    testWidget->setStyle(oldStyle);
}

void tst_QLineEdit::validateOnFocusOut()
{
    QSignalSpy editingFinishedSpy(testWidget, SIGNAL(editingFinished()));
    testWidget->setValidator(new QIntValidator(100, 999, 0));
    QTest::keyPress(testWidget, '1');
    QTest::keyPress(testWidget, '0');
    QFocusEvent e1(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
    QApplication::sendEvent(testWidget, &e1);
    QCOMPARE(editingFinishedSpy.count(), 0);
    QFocusEvent e2(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
    QApplication::sendEvent(testWidget, &e2);
    QTest::keyPress(testWidget, '0');
    QFocusEvent e3(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
    QApplication::sendEvent(testWidget, &e3);
    QCOMPARE(editingFinishedSpy.count(), 1);
}

void tst_QLineEdit::editUnvalidText()
{
    testWidget->clear();
    testWidget->setValidator(new QIntValidator(0, 120, 0));
    testWidget->setText("1234");

    QVERIFY(!testWidget->hasAcceptableInput());
    QTest::keyPress(testWidget, Qt::Key_Backspace);
    QTest::keyPress(testWidget, Qt::Key_A);
    QTest::keyPress(testWidget, Qt::Key_B);
    QTest::keyPress(testWidget, Qt::Key_C);
    QTest::keyPress(testWidget, Qt::Key_0);
    QVERIFY(!testWidget->hasAcceptableInput());
    QCOMPARE(testWidget->text(), QString("123abc0"));
    testWidget->cursorBackward(false);
    testWidget->cursorBackward(true, 4);
    QTest::keyPress(testWidget, Qt::Key_Delete);
    QVERIFY(testWidget->hasAcceptableInput());
    QCOMPARE(testWidget->text(), QString("120"));
    QTest::keyPress(testWidget, Qt::Key_1);
    QVERIFY(testWidget->hasAcceptableInput());
    QCOMPARE(testWidget->text(), QString("120"));
}

QTEST_MAIN(tst_QLineEdit)
#include "tst_qlineedit.moc"

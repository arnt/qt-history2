/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QString>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <qdebug.h>
#include <qinputdialog.h>

//TESTED_CLASS=QInputDialog
//TESTED_FILES=

class tst_QInputDialog : public QObject
{
    Q_OBJECT
//private:
    QWidget *parent;
    void (*testFunc)(QInputDialog *);
    static void testFuncGetInteger(QInputDialog *dialog);
    static void testFuncGetDouble(QInputDialog *dialog);
    static void testFuncGetText(QInputDialog *dialog);
    static void testFuncGetItem(QInputDialog *dialog);
    void timerEvent(QTimerEvent *event);
    void testGetInteger(int min, int max);
    void testGetDouble(double min, double max, int decimals);
    void testGetText(const QString& value);
    void testGetItem(const QStringList& items, bool editable);
private slots:
    void getInteger_data();
    void getInteger();
    void getDouble_data();
    void getDouble();
    void getText_data();
    void getText();
    void getItem_data();
    void getItem();
};

static QString stripFraction(const QString &s)
{
    if (!s.contains('.')) return s;
    const int period = s.indexOf('.');
    int end;
    for (end = s.size() - 1; end > period && s[end] == '0'; --end) ;
    return s.left(end + (end == period ? 0 : 1));
}

static QString normalizeNumericString(const QString &s)
{
    return stripFraction(s); // assumed to be sufficient
}

template <typename SpinBoxType>
static void testInitialState(
    SpinBoxType* sbox, QPushButton *okButton, QLineEdit *ledit)
{
    QVERIFY(sbox->value() >= sbox->minimum());
    QVERIFY(sbox->value() <= sbox->maximum());
    QVERIFY(sbox->hasAcceptableInput());
    QCOMPARE(
        normalizeNumericString(ledit->selectedText()),
        normalizeNumericString(QString("%1").arg(sbox->value())));
    QVERIFY(okButton->isEnabled());
}

static void testInitialState(QPushButton *okButton, QLineEdit *ledit)
{
    QVERIFY(ledit->hasAcceptableInput());
    QCOMPARE(
        normalizeNumericString(ledit->selectedText()),
        normalizeNumericString(ledit->text()));
    QVERIFY(okButton->isEnabled());
}

template <typename SpinBoxType>
static void testTypingValue(
    SpinBoxType* sbox, QPushButton *okButton, const QString &value)
{
    sbox->selectAll();
    for (int i = 0; i < value.size(); ++i) {
        const QChar valChar = value[i];
        QTest::keyClick(sbox, valChar.toAscii()); // ### always guaranteed to work?
        if (sbox->hasAcceptableInput())
            QVERIFY(okButton->isEnabled());
        else
            QVERIFY(!okButton->isEnabled());
    }
}

static void testTypingValue(QLineEdit *ledit, QPushButton *okButton, const QString &value)
{
    ledit->selectAll();
    for (int i = 0; i < value.size(); ++i) {
        const QChar valChar = value[i];
        QTest::keyClick(ledit, valChar.toAscii()); // ### always guaranteed to work?
        QVERIFY(ledit->hasAcceptableInput());
        QVERIFY(okButton->isEnabled());
    }
}

template <typename SpinBoxType, typename ValueType>
static void testInvalidateAndRestore(
    SpinBoxType* sbox, QPushButton *okButton, QLineEdit *ledit, ValueType * = 0)
{
    const ValueType lastValidValue = sbox->value();

    sbox->selectAll();
    QTest::keyClick(ledit, Qt::Key_Delete);
    QVERIFY(!sbox->hasAcceptableInput());
    QVERIFY(!okButton->isEnabled());

    QTest::keyClick(ledit, Qt::Key_Return); // should work with Qt::Key_Enter too
    QVERIFY(sbox->hasAcceptableInput());
    QVERIFY(okButton->isEnabled());
    QCOMPARE(sbox->value(), lastValidValue);
    QCOMPARE(
        normalizeNumericString(ledit->text()),
        normalizeNumericString(QString("%1").arg(sbox->value())));
}

template <typename WidgetType>
static WidgetType *getChild(QWidget *parent, WidgetType * = 0)
{
    WidgetType *result = 0;
    const int sleepusecs = 10, waitsecs = 10; // tunables
    for (int usecsleft = waitsecs * 1000000; result == 0 && usecsleft > 0;
         usecsleft -= sleepusecs) {
        foreach (QObject *child, parent->children()) {
            if ((result = qobject_cast<WidgetType *>(child)) != 0)
                break;
        }
        if (result == 0)
            usleep(sleepusecs);
    }
    return result;
}

template <typename SpinBoxType, typename ValueType>
static void testGetNumeric(QInputDialog *dialog, SpinBoxType * = 0, ValueType * = 0)
{
    SpinBoxType *sbox = getChild<SpinBoxType>(dialog);
    Q_ASSERT(sbox);

    QLineEdit *ledit = getChild<QLineEdit>(sbox);
    Q_ASSERT(ledit);

    QDialogButtonBox *bbox = getChild<QDialogButtonBox>(dialog);
    Q_ASSERT(bbox);
    QPushButton *okButton = bbox->button(QDialogButtonBox::Ok);
    Q_ASSERT(okButton);

    testInitialState<SpinBoxType>(sbox, okButton, ledit);
    const ValueType origValue = sbox->value();

    testInvalidateAndRestore<SpinBoxType, ValueType>(sbox, okButton, ledit);
    testTypingValue<SpinBoxType>(sbox, okButton, QString("%1").arg(sbox->minimum()));
    testTypingValue<SpinBoxType>(sbox, okButton, QString("%1").arg(sbox->maximum()));
    testTypingValue<SpinBoxType>(sbox, okButton, QString("%1").arg(sbox->minimum() - 1));
    testTypingValue<SpinBoxType>(sbox, okButton, QString("%1").arg(sbox->maximum() + 1));
    testTypingValue<SpinBoxType>(sbox, okButton, "0");
    testTypingValue<SpinBoxType>(sbox, okButton, "0.0");
    testTypingValue<SpinBoxType>(sbox, okButton, "foobar");

    testTypingValue<SpinBoxType>(sbox, okButton, QString("%1").arg(origValue));
}

static void testGetText(QInputDialog *dialog)
{
    QLineEdit *ledit = getChild<QLineEdit>(dialog);
    Q_ASSERT(ledit);

    QDialogButtonBox *bbox = getChild<QDialogButtonBox>(dialog);
    Q_ASSERT(bbox);
    QPushButton *okButton = bbox->button(QDialogButtonBox::Ok);
    Q_ASSERT(okButton);

    testInitialState(okButton, ledit);
    const QString origValue = ledit->text();

    testTypingValue(ledit, okButton, origValue);
}

static void testGetItem(QInputDialog *dialog)
{
    QComboBox *cbox = getChild<QComboBox>(dialog);
    Q_ASSERT(cbox);

    QDialogButtonBox *bbox = getChild<QDialogButtonBox>(dialog);
    Q_ASSERT(bbox);
    QPushButton *okButton = bbox->button(QDialogButtonBox::Ok);
    Q_ASSERT(okButton);

    QVERIFY(okButton->isEnabled());
    const int origIndex = cbox->currentIndex();
    cbox->setCurrentIndex(origIndex - 1);
    cbox->setCurrentIndex(origIndex);
    QVERIFY(okButton->isEnabled());
}

void tst_QInputDialog::testFuncGetInteger(QInputDialog *dialog)
{
    testGetNumeric<QSpinBox, int>(dialog);
}

void tst_QInputDialog::testFuncGetDouble(QInputDialog *dialog)
{
    testGetNumeric<QDoubleSpinBox, double>(dialog);
}

void tst_QInputDialog::testFuncGetText(QInputDialog *dialog)
{
    ::testGetText(dialog);
}

void tst_QInputDialog::testFuncGetItem(QInputDialog *dialog)
{
    ::testGetItem(dialog);
}

void tst_QInputDialog::timerEvent(QTimerEvent *event)
{
    killTimer(event->timerId());

    QInputDialog *dialog = getChild<QInputDialog>(parent);
    Q_ASSERT(dialog);

    testFunc(dialog);

    dialog->done(QDialog::Accepted); // cause static function call to return
}

void tst_QInputDialog::testGetInteger(int min, int max)
{
    Q_ASSERT(min < max);
    parent = new QWidget;
    testFunc = &tst_QInputDialog::testFuncGetInteger;
    startTimer(0);
    bool ok = false;
    const int value = min + (max - min) / 2;
    const int result = QInputDialog::getInteger(parent, "", "", value, min, max, 1, &ok);
    QVERIFY(ok);
    QCOMPARE(result, value);
    delete parent;
}

void tst_QInputDialog::testGetDouble(double min, double max, int decimals)
{
    Q_ASSERT(min < max && decimals >= 0 && decimals <= 13);
    parent = new QWidget;
    testFunc = &tst_QInputDialog::testFuncGetDouble;
    startTimer(0);
    bool ok = false;
    // avoid decimals due to inconsistent roundoff behavior in QInputDialog::getDouble()
    // (at one decimal, 10.25 is rounded off to 10.2, while at two decimals, 10.025 is
    // rounded off to 10.03)
    const double value = static_cast<int>(min + (max - min) / 2);
    const double result =
        QInputDialog::getDouble(parent, "", "", value, min, max, decimals, &ok);
    QVERIFY(ok);
    QCOMPARE(result, value);
    delete parent;
}

void tst_QInputDialog::testGetText(const QString& value)
{
    parent = new QWidget;
    testFunc = &tst_QInputDialog::testFuncGetText;
    startTimer(0);
    bool ok = false;
    const QString result =
        QInputDialog::getText(parent, "", "", QLineEdit::Normal, value, &ok);
    QVERIFY(ok);
    QCOMPARE(result, value);
    delete parent;
}

void tst_QInputDialog::testGetItem(const QStringList& items, bool editable)
{
    parent = new QWidget;
    testFunc = &tst_QInputDialog::testFuncGetItem;
    startTimer(0);
    bool ok = false;
    const int index = items.size() / 2;
    const QString result =
        QInputDialog::getItem(parent, "", "", items, index, editable, &ok);
    QVERIFY(ok);
    QCOMPARE(result, items[index]);
    delete parent;
}

void tst_QInputDialog::getInteger_data()
{
    QTest::addColumn<int>("min");
    QTest::addColumn<int>("max");
    QTest::newRow("getInteger() - -") << -20 << -10;
    QTest::newRow("getInteger() - 0") << -20 <<   0;
    QTest::newRow("getInteger() - +") << -20 <<  20;
    QTest::newRow("getInteger() 0 +") <<   0 <<  20;
    QTest::newRow("getInteger() + +") <<  10 <<  20;
}

void tst_QInputDialog::getInteger()
{
    QFETCH(int, min);
    QFETCH(int, max);
    testGetInteger(min, max);
}

void tst_QInputDialog::getDouble_data()
{
    QTest::addColumn<double>("min");
    QTest::addColumn<double>("max");
    QTest::addColumn<int>("decimals");
    QTest::newRow("getDouble() - - d0") << -20.0  << -10.0  << 0;
    QTest::newRow("getDouble() - 0 d0") << -20.0  <<   0.0  << 0;
    QTest::newRow("getDouble() - + d0") << -20.0  <<  20.0  << 0;
    QTest::newRow("getDouble() 0 + d0") <<   0.0  <<  20.0  << 0;
    QTest::newRow("getDouble() + + d0") <<  10.0  <<  20.0  << 0;
    QTest::newRow("getDouble() - - d1") << -20.5  << -10.5  << 1;
    QTest::newRow("getDouble() - 0 d1") << -20.5  <<   0.0  << 1;
    QTest::newRow("getDouble() - + d1") << -20.5  <<  20.5  << 1;
    QTest::newRow("getDouble() 0 + d1") <<   0.0  <<  20.5  << 1;
    QTest::newRow("getDouble() + + d1") <<  10.5  <<  20.5  << 1;
    QTest::newRow("getDouble() - - d2") << -20.05 << -10.05 << 2;
    QTest::newRow("getDouble() - 0 d2") << -20.05 <<   0.0  << 2;
    QTest::newRow("getDouble() - + d2") << -20.05 <<  20.05 << 2;
    QTest::newRow("getDouble() 0 + d2") <<   0.0  <<  20.05 << 2;
    QTest::newRow("getDouble() + + d2") <<  10.05 <<  20.05 << 2;
}

void tst_QInputDialog::getDouble()
{
    QFETCH(double, min);
    QFETCH(double, max);
    QFETCH(int, decimals);
    testGetDouble(min, max, decimals);
}

void tst_QInputDialog::getText_data()
{
    QTest::addColumn<QString>("text");
    QTest::newRow("getText() 1") << "";
    QTest::newRow("getText() 2") << "foobar";
    QTest::newRow("getText() 3") << "  foobar";
    QTest::newRow("getText() 4") << "foobar  ";
    QTest::newRow("getText() 5") << "aAzZ`1234567890-=~!@#$%^&*()_+[]{}\\|;:'\",.<>/?";
}

void tst_QInputDialog::getText()
{
    QFETCH(QString, text);
    testGetText(text);
}

void tst_QInputDialog::getItem_data()
{
    QTest::addColumn<QStringList>("items");
    QTest::addColumn<bool>("editable");
    QTest::newRow("getItem() 1 true") << (QStringList() << "") << true;
    QTest::newRow("getItem() 2 true") <<
        (QStringList() << "spring" << "summer" << "fall" << "winter") << true;
    QTest::newRow("getItem() 1 false") << (QStringList() << "") << false;
    QTest::newRow("getItem() 2 false") <<
        (QStringList() << "spring" << "summer" << "fall" << "winter") << false;
}

void tst_QInputDialog::getItem()
{
    QFETCH(QStringList, items);
    QFETCH(bool, editable);
    testGetItem(items, editable);
}

QTEST_MAIN(tst_QInputDialog)
#include "tst_qinputdialog.moc"

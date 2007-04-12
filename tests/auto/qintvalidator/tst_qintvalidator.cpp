/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qvalidator.h>

class tst_QIntValidator : public QObject
{
    Q_OBJECT
private slots:
    void validate_data();
    void validate();
    void validateArabic();
};

Q_DECLARE_METATYPE(QValidator::State);
#define INV QValidator::Invalid
#define ITM QValidator::Intermediate
#define ACC QValidator::Acceptable

void tst_QIntValidator::validate_data()
{
    QTest::addColumn<int>("minimum");
    QTest::addColumn<int>("maximum");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QValidator::State>("state");

    QTest::newRow("data0") << 0 << 100 << QString("50") << ACC;
    QTest::newRow("data1") << 0 << 100 << QString("500") << ITM;
    QTest::newRow("data1a") << 0 << 100 << QString("5000") << INV;
    QTest::newRow("data1b") << -100 << 0 << QString("500") << ITM;
    QTest::newRow("data1c") << -100 << 0 << QString("5000") << INV;
    QTest::newRow("data2") << 0 << 100 << QString("-35") << INV;
    QTest::newRow("data3") << 0 << 100 << QString("a") << INV;
    QTest::newRow("data4") << 0 << 100 << QString("-") << INV;
    QTest::newRow("data5") << 0 << 100 << QString("100") << ACC;
    QTest::newRow("data6") << -100 << 100 << QString("-") << ITM;
    QTest::newRow("data7") << -100 << 100 << QString("-500") << ITM;
    QTest::newRow("data8") << -100 << 100 << QString("-100") << ACC;
    QTest::newRow("data9") << -100 << -10 << QString("10") << ITM; // ### previously: INV

    QTest::newRow("data10") << 100 << 999 << QString("") << ITM;
    QTest::newRow("data11") << 100 << 999 << QString("5") << ITM;
    QTest::newRow("data12") << 100 << 999 << QString("50") << ITM;
    QTest::newRow("data13") << 100 << 999 << QString("99") << ITM;
    QTest::newRow("data14") << 100 << 999 << QString("100") << ACC;
    QTest::newRow("data15") << 100 << 999 << QString("101") << ACC;
    QTest::newRow("data16") << 100 << 999 << QString("998") << ACC;
    QTest::newRow("data17") << 100 << 999 << QString("999") << ACC;
    QTest::newRow("data18") << 100 << 999 << QString("1000") << INV;
    QTest::newRow("data19") << 100 << 999 << QString("-10") << INV;

    QTest::newRow("data20") << -999 << -100 << QString("50") << ITM; // ### previously: INV
    QTest::newRow("data21") << -999 << -100 << QString("-") << ITM;
    QTest::newRow("data22") << -999 << -100 << QString("-1") << ITM;
    QTest::newRow("data23") << -999 << -100 << QString("-10") << ITM;
    QTest::newRow("data24") << -999 << -100 << QString("-100") << ACC;
    QTest::newRow("data25") << -999 << -100 << QString("-500") << ACC;
    QTest::newRow("data26") << -999 << -100 << QString("-998") << ACC;
    QTest::newRow("data27") << -999 << -100 << QString("-999") << ACC;
    QTest::newRow("data28") << -999 << -100 << QString("-1000") << INV;
    QTest::newRow("data29") << -999 << -100 << QString("-2000") << INV;

    QTest::newRow("1.1") << 0 << 10 << QString("") << ITM;
    QTest::newRow("1.2") << 10 << 0 << QString("") << ITM;

    QTest::newRow("2.1") << 0 << 10 << QString("-") << INV;
    QTest::newRow("2.2") << 0 << 10 << QString("-0") << INV;
    QTest::newRow("2.3") << -10 << -1 << QString("+") << INV;
    QTest::newRow("2.4") << -10 << 10 << QString("-") << ITM;
    QTest::newRow("2.5") << -10 << 10 << QString("+") << ITM;
    QTest::newRow("2.6") << -10 << 10 << QString("+0") << ACC;
    QTest::newRow("2.7") << -10 << 10 << QString("+1") << ACC;
    QTest::newRow("2.8") << -10 << 10 << QString("+-") << INV;
    QTest::newRow("2.9") << -10 << 10 << QString("-+") << INV;

    QTest::newRow("3.1") << 0 << 10 << QString("12345678901234567890") << INV;
    QTest::newRow("3.2") << 0 << 10 << QString("-12345678901234567890") << INV;
    QTest::newRow("3.3") << 0 << 10 << QString("000000000000000000000") << ACC;
    QTest::newRow("3.4") << 1 << 10 << QString("000000000000000000000") << ITM;
    QTest::newRow("3.5") << 0 << 10 << QString("-000000000000000000000") << INV;
    QTest::newRow("3.6") << -10 << -1 << QString("-000000000000000000000") << ITM;
    QTest::newRow("3.7") << -10 << -1 << QString("-0000000000000000000001") << ACC;

    QTest::newRow("4.1") << 0 << 10 << QString(" ") << INV;
    QTest::newRow("4.2") << 0 << 10 << QString(" 1") << INV;
    QTest::newRow("4.3") << 0 << 10 << QString("1 ") << INV;
    QTest::newRow("4.4") << 0 << 10 << QString("1.0") << INV;
    QTest::newRow("4.5") << 0 << 10 << QString("0.1") << INV;
    QTest::newRow("4.6") << 0 << 10 << QString(".1") << INV;
    QTest::newRow("4.7") << 0 << 10 << QString("-1.0") << INV;

    QTest::newRow("5.1") << 6 << 8 << QString("5") << ITM;
    QTest::newRow("5.2") << 6 << 8 << QString("7") << ACC;
    QTest::newRow("5.3") << 6 << 8 << QString("9") << ITM;
    QTest::newRow("5.3a") << 6 << 8 << QString("19") << INV;
    QTest::newRow("5.4") << -8 << -6 << QString("-5") << ITM;
    QTest::newRow("5.5") << -8 << -6 << QString("-7") << ACC;
    QTest::newRow("5.6") << -8 << -6 << QString("-9") << ITM;
    QTest::newRow("5.6a") << -8 << -6 << QString("-19") << INV;
    QTest::newRow("5.7") << -8 << -6 << QString("5") << ITM;
    QTest::newRow("5.8") << -8 << -6 << QString("7") << ITM;
    QTest::newRow("5.9") << -8 << -6 << QString("9") << ITM;
    QTest::newRow("5.10") << -6 << 8 << QString("-5") << ACC;
    QTest::newRow("5.11") << -6 << 8 << QString("5") << ACC;
    QTest::newRow("5.12") << -6 << 8 << QString("-7") << ITM;
    QTest::newRow("5.13") << -6 << 8 << QString("7") << ACC;
    QTest::newRow("5.14") << -6 << 8 << QString("-9") << ITM;
    QTest::newRow("5.15") << -6 << 8 << QString("9") << ITM;

    QTest::newRow("6.1") << 100 << 102 << QString("11") << ITM;
    QTest::newRow("6.2") << 100 << 102 << QString("-11") << INV;

    QTest::newRow("7.1") << 0 << 10 << QString("100") << INV;
    QTest::newRow("7.2") << 0 << -10 << QString("100") << INV;
    QTest::newRow("7.3") << 0 << -10 << QString("-100") << INV;
    QTest::newRow("7.4") << -100 << 10 << QString("100") << ITM;

    QTest::newRow("8.1") << -100 << -10 << QString("+") << INV;
    QTest::newRow("8.2") << -100 << -10 << QString("+50") << INV;
    QTest::newRow("8.3") << -100 << -10 << QString("50") << ITM;
    QTest::newRow("8.4") << 10 << 100 << QString("-") << INV;
    QTest::newRow("8.5") << 10 << 100 << QString("-50") << INV;
    QTest::newRow("8.6") << 10 << 100 << QString("5") << ITM;
    QTest::newRow("8.7") << -1 << 100 << QString("-") << ITM;
    QTest::newRow("8.8") << -1 << 100 << QString("-50") << ITM;
    QTest::newRow("8.9") << -1 << 100 << QString("5") << ACC;
    QTest::newRow("8.10") << -1 << 100 << QString("+") << ITM;
    QTest::newRow("8.11") << -1 << 100 << QString("+50") << ACC;

/*  I disagree with the test below - the validator will correctly return Intermediate
    for any three-digit number outside the range. This makes it much more intuitive
    when you are editing a number (ie. not just adding digits to the end). It's also
    what the docs currently say - Jacek
      QTest::newRow("lineedittest") << 0 << 100 << QString("153") << INV; */
}

void tst_QIntValidator::validateArabic()
{
    QString arabicNum;
    arabicNum += QChar(1633); // "18" in arabic
    arabicNum += QChar(1640);

    QIntValidator validator(-20, 20, 0);
    validator.setLocale(QLocale(QLocale::Arabic, QLocale::SaudiArabia));
    int i;
    QCOMPARE(validator.validate(arabicNum, i), QValidator::Acceptable);

}

void tst_QIntValidator::validate()
{
    QFETCH(int, minimum);
    QFETCH(int, maximum);
    QFETCH(QString, value);
    QFETCH(QValidator::State, state);

    QIntValidator iv(minimum, maximum, 0);
    int dummy;
    QCOMPARE((int)iv.validate(value, dummy), (int)state);
}

QTEST_MAIN(tst_QIntValidator)
#include "tst_qintvalidator.moc"

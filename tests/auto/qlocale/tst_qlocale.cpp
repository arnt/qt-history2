/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <math.h>
#include <qglobal.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <float.h>

#include <qlocale.h>
#include <private/qnumeric_p.h>

#ifdef Q_OS_LINUX
#    include <fenv.h>
#endif

Q_DECLARE_METATYPE(qlonglong)
Q_DECLARE_METATYPE(QDate)

//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qlocale.h corelib/tools/qlocale.cpp

class tst_QLocale : public QObject
{
    Q_OBJECT

public:
    tst_QLocale();

private slots:
    void windowsDefaultLocale();
    void macDefaultLocale();

    void ctor();
    void unixLocaleName();
    void double_conversion_data();
    void double_conversion();
    void long_long_conversion_data();
    void long_long_conversion();
    void long_long_conversion_extra();
    void testInfAndNan();
    void fpExceptions();
    void negativeZero();
    void dayOfWeek();
    void dayOfWeek_data();
    void formatDate();
    void formatDate_data();
    void formatTime();
    void formatTime_data();
    void negativeNumbers();
    void numberOptions();

private:
    QString m_decimal, m_thousand, m_sdate, m_ldate, m_time;
};

tst_QLocale::tst_QLocale()
{
}

void tst_QLocale::ctor()
{
    QLocale default_locale = QLocale::system();
    QLocale::Language default_lang = default_locale.language();
    QLocale::Country default_country = default_locale.country();

    qDebug("Default: %s/%s", QLocale::languageToString(default_lang).toLatin1().constData(),
            QLocale::countryToString(default_country).toLatin1().constData());

    {
	QLocale l;
	QVERIFY(l.language() == default_lang);
	QVERIFY(l.country() == default_country);
    }

#define TEST_CTOR(req_lang, req_country, exp_lang, exp_country) \
    { \
    	QLocale l(QLocale::req_lang, QLocale::req_country); \
	QCOMPARE(l.language(), exp_lang); \
	QCOMPARE(l.country(), exp_country); \
    }
    TEST_CTOR(C, AnyCountry, QLocale::C, QLocale::AnyCountry)
    TEST_CTOR(Zulu, AnyCountry, default_lang, default_country)
    TEST_CTOR(Zulu, France, default_lang, default_country)

    TEST_CTOR(English, AnyCountry, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, UnitedStates, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, France, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, UnitedKingdom, QLocale::English, QLocale::UnitedKingdom)

    TEST_CTOR(French, France, QLocale::French, QLocale::France)
    TEST_CTOR(C, France, QLocale::C, QLocale::AnyCountry)

    QLocale::setDefault(QLocale(QLocale::English, QLocale::France));

    {
	QLocale l;
	QVERIFY(l.language() == QLocale::English);
	QVERIFY(l.country() == QLocale::UnitedStates);
    }

    TEST_CTOR(French, France, QLocale::French, QLocale::France)
    TEST_CTOR(English, UnitedKingdom, QLocale::English, QLocale::UnitedKingdom)

    TEST_CTOR(French, France, QLocale::French, QLocale::France)
    TEST_CTOR(C, AnyCountry, QLocale::C, QLocale::AnyCountry)
    TEST_CTOR(C, France, QLocale::C, QLocale::AnyCountry)
    TEST_CTOR(Zulu, AnyCountry, QLocale::English, QLocale::UnitedStates)

    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedKingdom));

    {
	QLocale l;
	QVERIFY(l.language() == QLocale::English);
	QVERIFY(l.country() == QLocale::UnitedKingdom);
    }

    TEST_CTOR(French, France, QLocale::French, QLocale::France)
    TEST_CTOR(English, UnitedKingdom, QLocale::English, QLocale::UnitedKingdom)

    TEST_CTOR(C, AnyCountry, QLocale::C, QLocale::AnyCountry)
    TEST_CTOR(C, France, QLocale::C, QLocale::AnyCountry)

    QLocale::setDefault(QLocale(QLocale::Zulu, QLocale::France));

    {
	QLocale l;
	QVERIFY(l.language() == QLocale::English);
	QVERIFY(l.country() == QLocale::UnitedKingdom);
    }

    TEST_CTOR(Zulu, AnyCountry, QLocale::English, QLocale::UnitedKingdom)
    TEST_CTOR(Zulu, France, QLocale::English, QLocale::UnitedKingdom)

    TEST_CTOR(English, AnyCountry, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, UnitedStates, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, France, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, UnitedKingdom, QLocale::English, QLocale::UnitedKingdom)

    TEST_CTOR(French, France, QLocale::French, QLocale::France)
    TEST_CTOR(C, AnyCountry, QLocale::C, QLocale::AnyCountry)
    TEST_CTOR(C, France, QLocale::C, QLocale::AnyCountry)

    QLocale::setDefault(QLocale(QLocale::Zulu, QLocale::AnyCountry));

    {
	QLocale l;
	QVERIFY(l.language() == QLocale::English);
	QVERIFY(l.country() == QLocale::UnitedKingdom);
    }


    TEST_CTOR(Zulu, AnyCountry, QLocale::English, QLocale::UnitedKingdom)
    TEST_CTOR(Zulu, France, QLocale::English, QLocale::UnitedKingdom)

    TEST_CTOR(English, AnyCountry, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, UnitedStates, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, France, QLocale::English, QLocale::UnitedStates)
    TEST_CTOR(English, UnitedKingdom, QLocale::English, QLocale::UnitedKingdom)

    TEST_CTOR(French, France, QLocale::French, QLocale::France)
    TEST_CTOR(C, AnyCountry, QLocale::C, QLocale::AnyCountry)
    TEST_CTOR(C, France, QLocale::C, QLocale::AnyCountry)

    TEST_CTOR(Arabic, AnyCountry, QLocale::Arabic, QLocale::SaudiArabia)
    TEST_CTOR(Dutch, AnyCountry, QLocale::Dutch, QLocale::Netherlands)
    TEST_CTOR(German, AnyCountry, QLocale::German, QLocale::Germany)
    TEST_CTOR(Greek, AnyCountry, QLocale::Greek, QLocale::Greece)
    TEST_CTOR(Malay, AnyCountry, QLocale::Malay, QLocale::Malaysia)
    TEST_CTOR(Persian, AnyCountry, QLocale::Persian, QLocale::Iran)
    TEST_CTOR(Portuguese, AnyCountry, QLocale::Portuguese, QLocale::Portugal)
    TEST_CTOR(Serbian, AnyCountry, QLocale::Serbian, QLocale::SerbiaAndMontenegro)
    TEST_CTOR(Somali, AnyCountry, QLocale::Somali, QLocale::Somalia)
    TEST_CTOR(Spanish, AnyCountry, QLocale::Spanish, QLocale::Spain)
    TEST_CTOR(Swedish, AnyCountry, QLocale::Swedish, QLocale::Sweden)
    TEST_CTOR(Uzbek, AnyCountry, QLocale::Uzbek, QLocale::Uzbekistan)

#undef TEST_CTOR

#define TEST_CTOR(req_lc, exp_lang, exp_country) \
    { \
	QLocale l(req_lc); \
	QVERIFY2(l.language() == QLocale::exp_lang \
		&& l.country() == QLocale::exp_country, \
		QString("requested: \"" + QString(req_lc) + "\", got: " \
		+ QLocale::languageToString(l.language()) \
		+ "/" + QLocale::countryToString(l.country())).toLatin1().constData()); \
    }

    QLocale::setDefault(QLocale(QLocale::C));

    TEST_CTOR("C", C, AnyCountry)
    TEST_CTOR("bla", C, AnyCountry)
    TEST_CTOR("zz", C, AnyCountry)
    TEST_CTOR("zz_zz", C, AnyCountry)
    TEST_CTOR("zz...", C, AnyCountry)
    TEST_CTOR("", C, AnyCountry)
    TEST_CTOR("en/", C, AnyCountry)
    TEST_CTOR(QString::null, C, AnyCountry)
    TEST_CTOR("en", English, UnitedStates)
    TEST_CTOR("en", English, UnitedStates)
    TEST_CTOR("en.", English, UnitedStates)
    TEST_CTOR("en@", English, UnitedStates)
    TEST_CTOR("en.@", English, UnitedStates)
    TEST_CTOR("en_", English, UnitedStates)
    TEST_CTOR("en_.", English, UnitedStates)
    TEST_CTOR("en_.@", English, UnitedStates)
    TEST_CTOR("en.bla", English, UnitedStates)
    TEST_CTOR("en@bla", English, UnitedStates)
    TEST_CTOR("en_blaaa", English, UnitedStates)
    TEST_CTOR("en_zz", English, UnitedStates)
    TEST_CTOR("en_GB", English, UnitedKingdom)
    TEST_CTOR("en_GB.bla", English, UnitedKingdom)
    TEST_CTOR("en_GB@.bla", English, UnitedKingdom)
    TEST_CTOR("en_GB@bla", English, UnitedKingdom)

#undef TEST_CTOR

}

void tst_QLocale::unixLocaleName()
{
#define TEST_NAME(req_lang, req_country, exp_name) \
    { \
    	QLocale l(QLocale::req_lang, QLocale::req_country); \
	QCOMPARE(l.name(), QString(exp_name)); \
    }

    QLocale::setDefault(QLocale(QLocale::C));

    TEST_NAME(C, AnyCountry, "C")
    TEST_NAME(English, AnyCountry, "en_US")
    TEST_NAME(English, UnitedKingdom, "en_GB")
    TEST_NAME(Zulu, UnitedKingdom, "C")

#undef TEST_NAME
}

void tst_QLocale::double_conversion_data()
{
    QTest::addColumn<QString>("locale_name");
    QTest::addColumn<QString>("num_str");
    QTest::addColumn<bool>("good");
    QTest::addColumn<double>("num");

    QTest::newRow("C 1")         << QString("C") << QString("1")          << true  << 1.0;
    QTest::newRow("C 1.0")       << QString("C") << QString("1.0")        << true  << 1.0;
    QTest::newRow("C 1.234")     << QString("C") << QString("1.234")      << true  << 1.234;
    QTest::newRow("C 1.234e-10") << QString("C") << QString("1.234e-10")  << true  << 1.234e-10;
    QTest::newRow("C 1.234E10")  << QString("C") << QString("1.234E10")   << true  << 1.234e10;
    QTest::newRow("C 1e10")      << QString("C") << QString("1e10")       << true  << 1.0e10;
    QTest::newRow("C  1")        << QString("C") << QString(" 1")         << true  << 1.0;
    QTest::newRow("C   1")       << QString("C") << QString("  1")        << true  << 1.0;
    QTest::newRow("C 1 ")        << QString("C") << QString("1 ")         << true  << 1.0;
    QTest::newRow("C 1  ")       << QString("C") << QString("1  ")        << true  << 1.0;

    QTest::newRow("C 1,")	        << QString("C") << QString("1,")	 << false << 0.0;
    QTest::newRow("C 1,2")	        << QString("C") << QString("1,2")	 << false << 0.0;
    QTest::newRow("C 1,23")        << QString("C") << QString("1,23")	 << false << 0.0;
    QTest::newRow("C 1,234")       << QString("C") << QString("1,234")	 << true  << 1234.0;
    QTest::newRow("C 1,234,")      << QString("C") << QString("1,234,")	 << false << 0.0;
    QTest::newRow("C 1,234,5")     << QString("C") << QString("1,234,5")	 << false << 0.0;
    QTest::newRow("C 1,234,56")    << QString("C") << QString("1,234,56")   << false << 0.0;
    QTest::newRow("C 1,234,567")   << QString("C") << QString("1,234,567")  << true  << 1234567.0;
    QTest::newRow("C 1,234,567.")  << QString("C") << QString("1,234,567.") << true  << 1234567.0;
    QTest::newRow("C 1,234,567.8")	<< QString("C") << QString("1,234,567.8")<< true  << 1234567.8;
    QTest::newRow("C 1,234567.8")  << QString("C") << QString("1,234567.8") << false << 0.0;
    QTest::newRow("C 12,34567.8")  << QString("C") << QString("12,34567.8") << false << 0.0;
    QTest::newRow("C 1234,567.8")  << QString("C") << QString("1234,567.8") << false << 0.0;
    QTest::newRow("C 1234567.8")   << QString("C") << QString("1234567.8")  << true  << 1234567.8;
    QTest::newRow("C ,")	    	<< QString("C") << QString(",")	         << false << 0.0;
    QTest::newRow("C ,123")	<< QString("C") << QString(",123")       << false << 0.0;
    QTest::newRow("C ,3")	    	<< QString("C") << QString(",3")         << false << 0.0;
    QTest::newRow("C , 3")	    	<< QString("C") << QString(", 3")        << false << 0.0;
    QTest::newRow("C ,  3")	<< QString("C") << QString(",  3")       << false << 0.0;
    QTest::newRow("C ,  3.2")	<< QString("C") << QString(",  3.2")     << false << 0.0;
    QTest::newRow("C ,  3.2e2")	<< QString("C") << QString(",  3.2e2")   << false << 0.0;
    QTest::newRow("C ,  e2")	<< QString("C") << QString(",  e2")	 << false << 0.0;
    QTest::newRow("C 1,,234")	<< QString("C") << QString("1,,234")	 << false << 0.0;

    QTest::newRow("C empty")     << QString("C") << QString("")           << false << 0.0;
    QTest::newRow("C null")      << QString("C") << QString()         << false << 0.0;
    QTest::newRow("C .")         << QString("C") << QString(".")          << false << 0.0;
    QTest::newRow("C 1e")        << QString("C") << QString("1e")         << false << 0.0;
    QTest::newRow("C 1,0")       << QString("C") << QString("1,0")        << false << 0.0;
    QTest::newRow("C 1,000")     << QString("C") << QString("1,000")      << true  << 1000.0;
    QTest::newRow("C 1,000e-6")  << QString("C") << QString("1,000e-6")   << true  << 1000.0e-6;
    QTest::newRow("C 1e1.0")     << QString("C") << QString("1e1.0")      << false << 0.0;
    QTest::newRow("C 1e+")       << QString("C") << QString("1e+")        << false << 0.0;
    QTest::newRow("C 1e-")       << QString("C") << QString("1e-")        << false << 0.0;

    QTest::newRow("C .1")        << QString("C") << QString(".1")         << true  << 0.1;
    QTest::newRow("C -.1")       << QString("C") << QString("-.1")        << true  << -0.1;
    QTest::newRow("C 1.")        << QString("C") << QString("1.")         << true  << 1.0;
    QTest::newRow("C 1.E10")     << QString("C") << QString("1.E10")      << true  << 1.0e10;
    QTest::newRow("C 1e+10")     << QString("C") << QString("1e+10")      << true  << 1.0e+10;

    QTest::newRow("de_DE 1.")	    << QString("de_DE") << QString("1.")	 << false << 0.0;
    QTest::newRow("de_DE 1.2")	    << QString("de_DE") << QString("1.2")	 << false << 0.0;
    QTest::newRow("de_DE 1.23")        << QString("de_DE") << QString("1.23")       << false << 0.0;
    QTest::newRow("de_DE 1.234")       << QString("de_DE") << QString("1.234")	 << true  << 1234.0;
    QTest::newRow("de_DE 1.234,")      << QString("de_DE") << QString("1.234.")	 << false << 0.0;
    QTest::newRow("de_DE 1.234.5")     << QString("de_DE") << QString("1.234.5")	 << false << 0.0;
    QTest::newRow("de_DE 1.234.56")    << QString("de_DE") << QString("1.234.56")   << false << 0.0;
    QTest::newRow("de_DE 1.234.567")   << QString("de_DE") << QString("1.234.567")  << true  << 1234567.0;
    QTest::newRow("de_DE 1.234.567,")  << QString("de_DE") << QString("1.234.567,") << true  << 1234567.0;
    QTest::newRow("de_DE 1.234.567,8") << QString("de_DE") << QString("1.234.567,8")<< true  << 1234567.8;
    QTest::newRow("de_DE 1.234567,8")  << QString("de_DE") << QString("1.234567,8") << false << 0.0;
    QTest::newRow("de_DE 12.34567,8")  << QString("de_DE") << QString("12.34567,8") << false << 0.0;
    QTest::newRow("de_DE 1234.567,8")  << QString("de_DE") << QString("1234.567,8") << false << 0.0;
    QTest::newRow("de_DE 1234567,8")   << QString("de_DE") << QString("1234567,8")  << true  << 1234567.8;
    QTest::newRow("de_DE .123")	    << QString("de_DE") << QString(".123")       << false << 0.0;
    QTest::newRow("de_DE .3")	    << QString("de_DE") << QString(".3")         << false << 0.0;
    QTest::newRow("de_DE . 3")	    << QString("de_DE") << QString(". 3")        << false << 0.0;
    QTest::newRow("de_DE .  3")	    << QString("de_DE") << QString(".  3")       << false << 0.0;
    QTest::newRow("de_DE .  3,2")	    << QString("de_DE") << QString(".  3,2")     << false << 0.0;
    QTest::newRow("de_DE .  3,2e2")    << QString("de_DE") << QString(".  3,2e2")   << false << 0.0;
    QTest::newRow("de_DE .  e2")	    << QString("de_DE") << QString(".  e2")	 << false << 0.0;
    QTest::newRow("de_DE 1..234")	    << QString("de_DE") << QString("1..234")	 << false << 0.0;

    QTest::newRow("de_DE 1")         << QString("de_DE") << QString("1")          << true  << 1.0;
    QTest::newRow("de_DE 1.0")       << QString("de_DE") << QString("1.0")        << false << 0.0;
    QTest::newRow("de_DE 1.234e-10") << QString("de_DE") << QString("1.234e-10")  << true  << 1234.0e-10;
    QTest::newRow("de_DE 1.234E10")  << QString("de_DE") << QString("1.234E10")   << true  << 1234.0e10;
    QTest::newRow("de_DE 1e10")      << QString("de_DE") << QString("1e10")       << true  << 1.0e10;
    QTest::newRow("de_DE .1")        << QString("de_DE") << QString(".1")         << false << 0.0;
    QTest::newRow("de_DE -.1")       << QString("de_DE") << QString("-.1")        << false << 0.0;
    QTest::newRow("de_DE 1.E10")     << QString("de_DE") << QString("1.E10")      << false << 0.0;
    QTest::newRow("de_DE 1e+10")     << QString("de_DE") << QString("1e+10")      << true  << 1.0e+10;

    QTest::newRow("de_DE 1,0")       << QString("de_DE") << QString("1,0")        << true  << 1.0;
    QTest::newRow("de_DE 1,234")     << QString("de_DE") << QString("1,234")      << true  << 1.234;
    QTest::newRow("de_DE 1,234e-10") << QString("de_DE") << QString("1,234e-10")  << true  << 1.234e-10;
    QTest::newRow("de_DE 1,234E10")  << QString("de_DE") << QString("1,234E10")   << true  << 1.234e10;
    QTest::newRow("de_DE ,1")        << QString("de_DE") << QString(",1")         << true  << 0.1;
    QTest::newRow("de_DE -,1")       << QString("de_DE") << QString("-,1")        << true  << -0.1;
    QTest::newRow("de_DE 1,")        << QString("de_DE") << QString("1,")         << true  << 1.0;
    QTest::newRow("de_DE 1,E10")     << QString("de_DE") << QString("1,E10")      << true  << 1.0e10;

    QTest::newRow("de_DE empty")     << QString("de_DE") << QString("")           << false << 0.0;
    QTest::newRow("de_DE null")      << QString("de_DE") << QString()         << false << 0.0;
    QTest::newRow("de_DE .")         << QString("de_DE") << QString(".")          << false << 0.0;
    QTest::newRow("de_DE 1e")        << QString("de_DE") << QString("1e")         << false << 0.0;
    QTest::newRow("de_DE 1e1.0")     << QString("de_DE") << QString("1e1.0")      << false << 0.0;
    QTest::newRow("de_DE 1e+")       << QString("de_DE") << QString("1e+")        << false << 0.0;
    QTest::newRow("de_DE 1e-")       << QString("de_DE") << QString("1e-")        << false << 0.0;

    QTest::newRow("C 9,876543")      << QString("C") << QString("9,876543")        << false << 0.0;
    QTest::newRow("C 9,876543.2")    << QString("C") << QString("9,876543.2")      << false << 0.0;
    QTest::newRow("C 9,876543e-2")   << QString("C") << QString("9,876543e-2")     << false << 0.0;
    QTest::newRow("C 9,876543.0e-2") << QString("C") << QString("9,876543.0e-2")   << false << 0.0;

    QTest::newRow("de_DE 9.876543")      << QString("de_DE") << QString("9876.543")        << false << 0.0;
    QTest::newRow("de_DE 9.876543,2")    << QString("de_DE") << QString("9.876543,2")      << false << 0.0;
    QTest::newRow("de_DE 9.876543e-2")   << QString("de_DE") << QString("9.876543e-2")     << false << 0.0;
    QTest::newRow("de_DE 9.876543,0e-2") << QString("de_DE") << QString("9.876543,0e-2")   << false << 0.0;
}

void tst_QLocale::double_conversion()
{
#define MY_DOUBLE_EPSILON (2.22045e-16)

    QFETCH(QString, locale_name);
    QFETCH(QString, num_str);
    QFETCH(bool, good);
    QFETCH(double, num);

    QLocale locale(locale_name);
    QCOMPARE(locale.name(), locale_name);

    bool ok;
    double d = locale.toDouble(num_str, &ok);
    QCOMPARE(ok, good);

    if (ok) {
    	double diff = d - num;
	if (diff < 0)
	    diff = -diff;
	QVERIFY(diff <= MY_DOUBLE_EPSILON);
    }
}

void tst_QLocale::long_long_conversion_data()
{
    QTest::addColumn<QString>("locale_name");
    QTest::addColumn<QString>("num_str");
    QTest::addColumn<bool>("good");
    QTest::addColumn<qlonglong>("num");

    QTest::newRow("C null")       << QString("C") << QString()	   << false  << (qlonglong) 0;
    QTest::newRow("C empty")      << QString("C") << QString("")	   << false  << (qlonglong) 0;
    QTest::newRow("C 1")          << QString("C") << "1"	   << true  << (qlonglong) 1;
    QTest::newRow("C 1,")         << QString("C") << "1,"     << false << (qlonglong) 0;
    QTest::newRow("C 1,2")        << QString("C") << "1,2"    << false << (qlonglong) 0;
    QTest::newRow("C 1,23")       << QString("C") << "1,23"   << false << (qlonglong) 0;
    QTest::newRow("C 1,234")      << QString("C") << "1,234"  << true  << (qlonglong) 1234;
    QTest::newRow("C 1234567")    << QString("C") << "1234567"<< true  << (qlonglong) 1234567;
    QTest::newRow("C 1,234567")    << QString("C") << "1,234567"<< false  << (qlonglong) 0;
    QTest::newRow("C 12,34567")    << QString("C") << "12,34567"<< false  << (qlonglong) 0;
    QTest::newRow("C 123,4567")    << QString("C") << "123,4567"<< false  << (qlonglong) 0;
    QTest::newRow("C 1234,567")    << QString("C") << "1234,567"<< false  << (qlonglong) 0;
    QTest::newRow("C 12345,67")    << QString("C") << "12345,67"<< false  << (qlonglong) 0;
    QTest::newRow("C 123456,7")    << QString("C") << "123456,7"<< false  << (qlonglong) 0;
    QTest::newRow("C 1,234,567")    << QString("C")<< "1,234,567"<< true  << (qlonglong) 1234567;

    QTest::newRow("de_DE 1")      << QString("de_DE") << "1"      << true  << (qlonglong) 1;
    QTest::newRow("de_DE 1.")     << QString("de_DE") << "1."     << false << (qlonglong) 0;
    QTest::newRow("de_DE 1.2")    << QString("de_DE") << "1.2"    << false << (qlonglong) 0;
    QTest::newRow("de_DE 1.23")   << QString("de_DE") << "1.23"   << false << (qlonglong) 0;
    QTest::newRow("de_DE 1.234")  << QString("de_DE") << "1.234"  << true  << (qlonglong) 1234;
    QTest::newRow("de_DE 1234567")     << QString("de_DE") << "1234567"<< true  << (qlonglong) 1234567;
    QTest::newRow("de_DE 1.234567")    << QString("de_DE") << "1.234567"<< false  << (qlonglong) 0;
    QTest::newRow("de_DE 12.34567")    << QString("de_DE") << "12.34567"<< false  << (qlonglong) 0;
    QTest::newRow("de_DE 123.4567")    << QString("de_DE") << "123.4567"<< false  << (qlonglong) 0;
    QTest::newRow("de_DE 1234.567")    << QString("de_DE") << "1234.567"<< false  << (qlonglong) 0;
    QTest::newRow("de_DE 12345.67")    << QString("de_DE") << "12345.67"<< false  << (qlonglong) 0;
    QTest::newRow("de_DE 123456.7")    << QString("de_DE") << "123456.7"<< false  << (qlonglong) 0;
    QTest::newRow("de_DE 1.234.567")   << QString("de_DE")<< "1.234.567"<< true  << (qlonglong) 1234567;

    QTest::newRow("C   1234")       << QString("C") << "  1234"   << true  << (qlonglong) 1234;
    QTest::newRow("C 1234  ")       << QString("C") << "1234  "   << true  << (qlonglong) 1234;
    QTest::newRow("C   1234  ")     << QString("C") << "  1234  " << true  << (qlonglong) 1234;
}

void tst_QLocale::long_long_conversion()
{
    QFETCH(QString, locale_name);
    QFETCH(QString, num_str);
    QFETCH(bool, good);
    QFETCH(qlonglong, num);

    QLocale locale(locale_name);
    QCOMPARE(locale.name(), locale_name);

    bool ok;
    qlonglong l = locale.toLongLong(num_str, &ok);
    QCOMPARE(ok, good);

    if (ok) {
	QCOMPARE(l, num);
    }
}

void tst_QLocale::long_long_conversion_extra()
{
    QLocale l(QLocale::C);
    QCOMPARE(l.toString((qlonglong)1), QString("1"));
    QCOMPARE(l.toString((qlonglong)12), QString("12"));
    QCOMPARE(l.toString((qlonglong)123), QString("123"));
    QCOMPARE(l.toString((qlonglong)1234), QString("1,234"));
    QCOMPARE(l.toString((qlonglong)12345), QString("12,345"));
    QCOMPARE(l.toString((qlonglong)-1), QString("-1"));
    QCOMPARE(l.toString((qlonglong)-12), QString("-12"));
    QCOMPARE(l.toString((qlonglong)-123), QString("-123"));
    QCOMPARE(l.toString((qlonglong)-1234), QString("-1,234"));
    QCOMPARE(l.toString((qlonglong)-12345), QString("-12,345"));
    QCOMPARE(l.toString((qulonglong)1), QString("1"));
    QCOMPARE(l.toString((qulonglong)12), QString("12"));
    QCOMPARE(l.toString((qulonglong)123), QString("123"));
    QCOMPARE(l.toString((qulonglong)1234), QString("1,234"));
    QCOMPARE(l.toString((qulonglong)12345), QString("12,345"));
}

/*
void tst_QLocale::languageToString()
{
}

void tst_QLocale::setDefault()
{
}
*/

void tst_QLocale::testInfAndNan()
{
    double neginf = log(0.0);
    double nan = sqrt(-1.0);

#ifdef Q_OS_WIN
    // these causes INVALID floating point exception so we want to clare the status.
    _clear87();
#endif

    QVERIFY(qIsInf(-neginf));
    QVERIFY(!qIsNan(-neginf));
    QVERIFY(!qIsFinite(-neginf));

    QVERIFY(!qIsInf(nan));
    QVERIFY(qIsNan(nan));
    QVERIFY(!qIsFinite(nan));

    QVERIFY(!qIsInf(1.234));
    QVERIFY(!qIsNan(1.234));
    QVERIFY(qIsFinite(1.234));
}

void tst_QLocale::fpExceptions()
{
#ifndef _MCW_EM
#define _MCW_EM 0x0008001F
#endif
#ifndef _EM_INEXACT
#define _EM_INEXACT 0x00000001
#endif

    // check that qdtoa doesn't throw floating point exceptions when they are enabled
#ifdef Q_OS_WIN
    unsigned int oldbits = _control87(0, 0);
    _control87( 0 | _EM_INEXACT, _MCW_EM );
#endif

#ifdef Q_OS_LINUX
    fenv_t envp;
    fegetenv(&envp);
    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
#endif

    QString::number(1000.1245);
    QString::number(1.1);
    QString::number(0.0);

    QVERIFY(true);

#ifdef Q_OS_WIN
    _clear87();
    _control87(oldbits, 0xFFFFF);
#endif

#ifdef Q_OS_LINUX
    fesetenv(&envp);
#endif
}

void tst_QLocale::negativeZero()
{
    double negativeZero( 0.0 ); // Initialise to zero.
    uchar *ptr = (uchar *)&negativeZero;
    ptr[QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 7] = 0x80;

    QString s = QString::number(negativeZero);
    QCOMPARE(s, QString("0"));
}

void tst_QLocale::dayOfWeek_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("shortName");
    QTest::addColumn<QString>("longName");

    QTest::newRow("Sun") << QDate(2006, 1, 1) << "Sun" << "Sunday";
    QTest::newRow("Mon") << QDate(2006, 1, 2) << "Mon" << "Monday";
    QTest::newRow("Tue") << QDate(2006, 1, 3) << "Tue" << "Tuesday";
    QTest::newRow("Wed") << QDate(2006, 1, 4) << "Wed" << "Wednesday";
    QTest::newRow("Thu") << QDate(2006, 1, 5) << "Thu" << "Thursday";
    QTest::newRow("Fri") << QDate(2006, 1, 6) << "Fri" << "Friday";
    QTest::newRow("Sat") << QDate(2006, 1, 7) << "Sat" << "Saturday";
}

void tst_QLocale::dayOfWeek()
{
    QFETCH(QDate, date);
    QFETCH(QString, shortName);
    QFETCH(QString, longName);

    QCOMPARE(QLocale::c().toString(date, "ddd"), shortName);
    QCOMPARE(QLocale::c().toString(date, "dddd"), longName);
}

void tst_QLocale::formatDate_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QString>("result");

    QTest::newRow("1") << QDate(1974, 12, 1) << "d/M/yyyy" << "1/12/1974";
    QTest::newRow("2") << QDate(1974, 12, 1) << "d/M/yyyyy" << "1/12/1974y";
    QTest::newRow("4") << QDate(1974, 1, 1) << "d/M/yyyy" << "1/1/1974";
    QTest::newRow("5") << QDate(1974, 1, 1) << "dd/MM/yyy" << "01/01/74y";
    QTest::newRow("6") << QDate(1974, 12, 1) << "ddd/MMM/yy" << "Sun/Dec/74";
    QTest::newRow("7") << QDate(1974, 12, 1) << "dddd/MMMM/y" << "Sunday/December/y";
    QTest::newRow("8") << QDate(1974, 12, 1) << "ddddd/MMMMM/yy" << "Sunday1/December12/74";
    QTest::newRow("9") << QDate(1974, 12, 1) << "'dddd'/MMMM/yy" << "dddd/December/74";
    QTest::newRow("10") << QDate(1974, 12, 1) << "d'dd'd/MMMM/yyy" << "1dd1/December/74y";
    QTest::newRow("11") << QDate(1974, 12, 1) << "d'dd'd/MMM'M'/yy" << "1dd1/DecM/74";
    QTest::newRow("12") << QDate(1974, 12, 1) << "d'd'dd/M/yy" << "1d01/12/74";

    QTest::newRow("20") << QDate(1974, 12, 1) << "foo" << "foo";
    QTest::newRow("21") << QDate(1974, 12, 1) << "'" << "";
    QTest::newRow("22") << QDate(1974, 12, 1) << "''" << "'";
    QTest::newRow("23") << QDate(1974, 12, 1) << "'''" << "'";
    QTest::newRow("24") << QDate(1974, 12, 1) << "\"" << "\"";
    QTest::newRow("25") << QDate(1974, 12, 1) << "\"\"" << "\"\"";
    QTest::newRow("26") << QDate(1974, 12, 1) << "\"yy\"" << "\"74\"";
    QTest::newRow("27") << QDate(1974, 12, 1) << "'\"yy\"'" << "\"yy\"";
}

void tst_QLocale::formatDate()
{
    QFETCH(QDate, date);
    QFETCH(QString, format);
    QFETCH(QString, result);

    QLocale l(QLocale::C);
    QCOMPARE(l.toString(date, format), result);
}

Q_DECLARE_METATYPE(QTime)

void tst_QLocale::formatTime_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QString>("result");

    QTest::newRow("1") << QTime(1, 2, 3) << "h:m:s" << "1:2:3";
    QTest::newRow("3") << QTime(1, 2, 3) << "H:m:s" << "1:2:3";
    QTest::newRow("4") << QTime(1, 2, 3) << "hh:mm:ss" << "01:02:03";
    QTest::newRow("5") << QTime(1, 2, 3) << "HH:mm:ss" << "01:02:03";
    QTest::newRow("6") << QTime(1, 2, 3) << "hhh:mmm:sss" << "011:022:033";

    QTest::newRow("8") << QTime(14, 2, 3) << "h:m:s" << "14:2:3";
    QTest::newRow("9") << QTime(14, 2, 3) << "H:m:s" << "14:2:3";
    QTest::newRow("10") << QTime(14, 2, 3) << "hh:mm:ss" << "14:02:03";
    QTest::newRow("11") << QTime(14, 2, 3) << "HH:mm:ss" << "14:02:03";
    QTest::newRow("12") << QTime(14, 2, 3) << "hhh:mmm:sss" << "1414:022:033";

    QTest::newRow("14") << QTime(14, 2, 3) << "h:m:s ap" << "2:2:3 pm";
    QTest::newRow("15") << QTime(14, 2, 3) << "H:m:s AP" << "14:2:3 PM";
    QTest::newRow("16") << QTime(14, 2, 3) << "hh:mm:ss aap" << "02:02:03 apm";
    QTest::newRow("17") << QTime(14, 2, 3) << "HH:mm:ss AP aa" << "14:02:03 PM aa";

    QTest::newRow("18") << QTime(1, 2, 3) << "h:m:s ap" << "1:2:3 am";
    QTest::newRow("19") << QTime(1, 2, 3) << "H:m:s AP" << "1:2:3 AM";

    QTest::newRow("20") << QTime(1, 2, 3) << "foo" << "foo";
    QTest::newRow("21") << QTime(1, 2, 3) << "'" << "";
    QTest::newRow("22") << QTime(1, 2, 3) << "''" << "'";
    QTest::newRow("23") << QTime(1, 2, 3) << "'''" << "'";
    QTest::newRow("24") << QTime(1, 2, 3) << "\"" << "\"";
    QTest::newRow("25") << QTime(1, 2, 3) << "\"\"" << "\"\"";
    QTest::newRow("26") << QTime(1, 2, 3) << "\"H\"" << "\"1\"";
    QTest::newRow("27") << QTime(1, 2, 3) << "'\"H\"'" << "\"H\"";

    QTest::newRow("28") << QTime(1, 2, 3, 456) << "H:m:s.z" << "1:2:3.456";
    QTest::newRow("29") << QTime(1, 2, 3, 456) << "H:m:s.zz" << "1:2:3.456456";
    QTest::newRow("30") << QTime(1, 2, 3, 456) << "H:m:s.zzz" << "1:2:3.456";
    QTest::newRow("31") << QTime(1, 2, 3, 4) << "H:m:s.z" << "1:2:3.4";
    QTest::newRow("32") << QTime(1, 2, 3, 4) << "H:m:s.zzz" << "1:2:3.004";
}

void tst_QLocale::formatTime()
{
    QFETCH(QTime, time);
    QFETCH(QString, format);
    QFETCH(QString, result);

    QLocale l(QLocale::C);
    QCOMPARE(l.toString(time, format), result);
}

void tst_QLocale::macDefaultLocale()
{
#ifndef Q_OS_MAC
    QSKIP("This is a Mac OS X-only test", SkipAll);
#endif

    QLocale locale = QLocale::system();
    if (locale.name() != QLatin1String("en_US")) {
        QSKIP("This test only tests for en_US", SkipAll);
    }

    QCOMPARE(locale.decimalPoint(), QChar('.'));
    QCOMPARE(locale.groupSeparator(), QChar(','));
    QCOMPARE(locale.dateFormat(QLocale::ShortFormat), QString("M/d/yy"));
    QCOMPARE(locale.dateFormat(QLocale::LongFormat), QString("MMMM d, yyyy"));
    QCOMPARE(locale.timeFormat(QLocale::ShortFormat), QString("h:mm AP"));
    QCOMPARE(locale.timeFormat(QLocale::LongFormat), QString("h:mm:ss AP t"));

    // make sure we are using the system to parse them
    QCOMPARE(locale.toString(1234.56), QString("1,234.56"));
    QCOMPARE(locale.toString(QDate(1974, 12, 1), QLocale::ShortFormat), QString("12/1/74"));
    QCOMPARE(locale.toString(QDate(1974, 12, 1), QLocale::LongFormat), QString("December 1, 1974"));
    QCOMPARE(locale.toString(QTime(1,2,3), QLocale::ShortFormat), QString("1:02 AM"));
    // ### Can't wait for this to break...
    QCOMPARE(locale.toString(QTime(1,2,3), QLocale::LongFormat), QString("1:02:03 AM GMT+02:00"));
    QCOMPARE(locale.dayName(1), QString("Monday"));
    QCOMPARE(locale.dayName(7), QString("Sunday"));
    QCOMPARE(locale.monthName(1), QString("January"));
    QCOMPARE(locale.monthName(12), QString("December"));

}

#ifdef Q_OS_WIN
#include <qt_windows.h>

static QString getWinLocaleInfo(LCTYPE type)
{
    int cnt = 0;
    QT_WA({
        cnt = GetLocaleInfoW(LOCALE_USER_DEFAULT, type, 0, 0)*2;
    } , {
        cnt = GetLocaleInfoA(LOCALE_USER_DEFAULT, type, 0, 0);
    });

    if (cnt == 0) {
        qWarning("QLocale: empty windows locale info (%d)", type);
        return QString();
    }

    QByteArray buff(cnt, 0);

    QT_WA({
        cnt = GetLocaleInfoW(LOCALE_USER_DEFAULT, type,
                                reinterpret_cast<wchar_t*>(buff.data()),
                                buff.size()/2);
    } , {
        cnt = GetLocaleInfoA(LOCALE_USER_DEFAULT, type,
                                buff.data(), buff.size());
    });

    if (cnt == 0) {
        qWarning("QLocale: empty windows locale info (%d)", type);
        return QString();
    }

    QString result;
    QT_WA({
        result = QString::fromUtf16(reinterpret_cast<ushort*>(buff.data()));
    } , {
        result = QString::fromLocal8Bit(buff.data());
    });
    return result;
}

static void setWinLocaleInfo(LCTYPE type, const QString &value)
{
    QT_WA({
        SetLocaleInfoW(LOCALE_USER_DEFAULT, type, reinterpret_cast<const wchar_t*>(value.utf16()));
    } , {
        SetLocaleInfoA(LOCALE_USER_DEFAULT, type, value.toLocal8Bit());
    });
}

class RestoreLocaleHelper {
public:
    RestoreLocaleHelper() {
        m_decimal = getWinLocaleInfo(LOCALE_SDECIMAL);
        m_thousand = getWinLocaleInfo(LOCALE_STHOUSAND);
        m_sdate = getWinLocaleInfo(LOCALE_SSHORTDATE);
        m_ldate = getWinLocaleInfo(LOCALE_SLONGDATE);
        m_time = getWinLocaleInfo(LOCALE_STIMEFORMAT);
    }

    ~RestoreLocaleHelper() {
        // restore these, or the user will get a surprise
        setWinLocaleInfo(LOCALE_SDECIMAL, m_decimal);
        setWinLocaleInfo(LOCALE_STHOUSAND, m_thousand);
        setWinLocaleInfo(LOCALE_SSHORTDATE, m_sdate);
        setWinLocaleInfo(LOCALE_SLONGDATE, m_ldate);
        setWinLocaleInfo(LOCALE_STIMEFORMAT, m_time);
    }

    QString m_decimal, m_thousand, m_sdate, m_ldate, m_time;

};

#endif

void tst_QLocale::windowsDefaultLocale()
{
#ifndef Q_OS_WIN
    QSKIP("This is a Windows test", SkipAll);
#else
    RestoreLocaleHelper systemLocale;
    // set weird system defaults and make sure we're using them
    setWinLocaleInfo(LOCALE_SDECIMAL, QLatin1String("@"));
    setWinLocaleInfo(LOCALE_STHOUSAND, QLatin1String("?"));
    setWinLocaleInfo(LOCALE_SSHORTDATE, QLatin1String("d*M*yyyy"));
    setWinLocaleInfo(LOCALE_SLONGDATE, QLatin1String("d@M@yyyy"));
    setWinLocaleInfo(LOCALE_STIMEFORMAT, QLatin1String("h^m^s"));
    QLocale locale = QLocale::system();

    // make sure we are seeing the system's format strings
    QCOMPARE(locale.decimalPoint(), QChar('@'));
    QCOMPARE(locale.groupSeparator(), QChar('?'));
    QCOMPARE(locale.dateFormat(QLocale::ShortFormat), QString("d*M*yyyy"));
    QCOMPARE(locale.dateFormat(QLocale::LongFormat), QString("d@M@yyyy"));
    QCOMPARE(locale.timeFormat(QLocale::ShortFormat), QString("h^m^s"));
    QCOMPARE(locale.timeFormat(QLocale::LongFormat), QString("h^m^s"));

    // make sure we are using the system to parse them
    QCOMPARE(locale.toString(1234.56), QString("1?234@56"));
    QCOMPARE(locale.toString(QDate(1974, 12, 1), QLocale::ShortFormat), QString("1*12*1974"));
    QCOMPARE(locale.toString(QDate(1974, 12, 1), QLocale::LongFormat), QString("1@12@1974"));
    QCOMPARE(locale.toString(QTime(1,2,3), QLocale::ShortFormat), QString("1^2^3"));
    QCOMPARE(locale.toString(QTime(1,2,3), QLocale::LongFormat), QString("1^2^3"));

#endif
}

void tst_QLocale::numberOptions()
{
    bool ok;

    QLocale locale(QLocale::C);
    QCOMPARE(locale.numberOptions(), 0);
    QCOMPARE(locale.toInt(QString("12,345"), &ok), 12345);
    QVERIFY(ok);
    QCOMPARE(locale.toInt(QString("12345"), &ok), 12345);
    QVERIFY(ok);
    QCOMPARE(locale.toString(12345), QString("12,345"));

    locale.setNumberOptions(QLocale::OmitGroupSeparator);
    QCOMPARE(locale.numberOptions(), QLocale::OmitGroupSeparator);
    QCOMPARE(locale.toInt(QString("12,345"), &ok), 12345);
    QVERIFY(ok);
    QCOMPARE(locale.toInt(QString("12345"), &ok), 12345);
    QVERIFY(ok);
    QCOMPARE(locale.toString(12345), QString("12345"));

    locale.setNumberOptions(QLocale::RejectGroupSeparator);
    QCOMPARE(locale.numberOptions(), QLocale::RejectGroupSeparator);
    locale.toInt(QString("12,345"), &ok);
    QVERIFY(!ok);
    QCOMPARE(locale.toInt(QString("12345"), &ok), 12345);
    QVERIFY(ok);
    QCOMPARE(locale.toString(12345), QString("12,345"));

    QLocale locale2 = locale;
    QCOMPARE(locale2.numberOptions(), QLocale::RejectGroupSeparator);
}

void tst_QLocale::negativeNumbers()
{
    QLocale locale(QLocale::C);

    bool ok;
    int i;

    i = locale.toInt(QLatin1String("-100"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -100);

    i = locale.toInt(QLatin1String("-1,000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -1000);

    i = locale.toInt(QLatin1String("-1000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -1000);

    i = locale.toInt(QLatin1String("-10,000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -10000);

    i = locale.toInt(QLatin1String("-10000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -10000);

    i = locale.toInt(QLatin1String("-100,000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -100000);

    i = locale.toInt(QLatin1String("-100000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -100000);

    i = locale.toInt(QLatin1String("-1,000,000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -1000000);

    i = locale.toInt(QLatin1String("-1000000"), &ok);
    QVERIFY(ok);
    QCOMPARE(i, -1000000);
}

QTEST_APPLESS_MAIN(tst_QLocale)
#include "tst_qlocale.moc"

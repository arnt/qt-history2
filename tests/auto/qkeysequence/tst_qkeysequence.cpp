/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>



#include <qkeysequence.h>






//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qkeysequence.h gui/kernel/qkeysequence.cpp

class tst_QKeySequence : public QObject
{
    Q_OBJECT

public:
    tst_QKeySequence();
    virtual ~tst_QKeySequence();

private slots:
    void operatorQString_data();
    void operatorQString();
    void compareConstructors_data();
    void compareConstructors();
    void symetricConstructors_data();
    void symetricConstructors();
    void checkMultipleNames();
    void mnemonic();
    void toString_data();
    void toString();
	void streamOperators_data();
    void streamOperators();
    void fromString_data();
    void fromString();
};

// copied from qkeysequence.cpp
const QString MacCtrl = QString(QChar(0x2318));
const QString MacMeta = QString(QChar(0x2303));
const QString MacAlt = QString(QChar(0x2325));
const QString MacShift = QString(QChar(0x21E7));

tst_QKeySequence::tst_QKeySequence()
{
}

tst_QKeySequence::~tst_QKeySequence()
{

}

void tst_QKeySequence::operatorQString_data()
{
    QTest::addColumn<int>("modifiers");
    QTest::addColumn<int>("keycode");
    QTest::addColumn<QString>("keystring");

    QTest::newRow( "No modifier" ) << 0 << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << QString( "\x0c5" );

#ifndef Q_WS_MAC
    QTest::newRow( "Ctrl+Left" ) << int(Qt::CTRL) << int(Qt::Key_Left) << QString( "Ctrl+Left" );
#if QT_VERSION > 0x040100
    QTest::newRow( "Ctrl+," ) << int(Qt::CTRL) << int(Qt::Key_Comma) << QString( "Ctrl+," );
#endif
    QTest::newRow( "Alt+Left" ) << int(Qt::ALT) << int(Qt::Key_Left) << QString( "Alt+Left" );
    QTest::newRow( "Alt+Shift+Left" ) << int(Qt::ALT | Qt::SHIFT) << int(Qt::Key_Left) << QString( "Alt+Shift+Left" );
    QTest::newRow( "Ctrl" ) << int(Qt::CTRL) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << QString( "Ctrl+\x0c5" );
    QTest::newRow( "Alt" ) << int(Qt::ALT) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << QString( "Alt+\x0c5" );
    QTest::newRow( "Shift" ) << int(Qt::SHIFT) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << QString( "Shift+\x0c5" );
    QTest::newRow( "Meta" ) << int(Qt::META) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << QString( "Meta+\x0c5" );
#else
    QTest::newRow( "Ctrl+Left" ) << int(Qt::CTRL) << int(Qt::Key_Left) << MacCtrl + "Left";
#if QT_VERSION > 0x040100
    QTest::newRow( "Ctrl+," ) << int(Qt::CTRL) << int(Qt::Key_Comma) << MacCtrl + ",";
#endif
    QTest::newRow( "Alt+Left" ) << int(Qt::ALT) << int(Qt::Key_Left) << MacAlt + "Left";
    QTest::newRow( "Alt+Shift+Left" ) << int(Qt::ALT | Qt::SHIFT) << int(Qt::Key_Left) << MacAlt + MacShift + "Left";
    QTest::newRow( "Ctrl" ) << int(Qt::CTRL) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << MacCtrl + "\x0c5";
    QTest::newRow( "Alt" ) << int(Qt::ALT) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << MacAlt + "\x0c5";
    QTest::newRow( "Shift" ) << int(Qt::SHIFT) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << MacShift + "\x0c5";
    QTest::newRow( "Meta" ) << int(Qt::META) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL) << MacMeta + "\x0c5";
#endif
}

void tst_QKeySequence::symetricConstructors_data()
{
    QTest::addColumn<int>("modifiers");
    QTest::addColumn<int>("keycode");

    QTest::newRow( "No modifier" ) << 0 << int(Qt::Key_Aring | Qt::UNICODE_ACCEL);
    QTest::newRow( "Ctrl" ) << int(Qt::CTRL) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL);
    QTest::newRow( "Alt" ) << int(Qt::ALT) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL);
    QTest::newRow( "Shift" ) << int(Qt::SHIFT) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL);
    QTest::newRow( "Meta" ) << int(Qt::META) << int(Qt::Key_Aring | Qt::UNICODE_ACCEL);
}

void tst_QKeySequence::compareConstructors_data()
{
    operatorQString_data();
}

// operator QString()
void tst_QKeySequence::operatorQString()
{
    QKeySequence seq;
    QFETCH( int, modifiers );
    QFETCH( int, keycode );
    QFETCH( QString, keystring );

    seq = QKeySequence( modifiers | keycode );

    QCOMPARE( (QString)seq, keystring );
}

// this verifies that the constructors can handle the same strings in and out
void tst_QKeySequence::symetricConstructors()
{
    QFETCH( int, modifiers );
    QFETCH( int, keycode );

    QKeySequence seq1( modifiers | keycode );
    QKeySequence seq2( (QString)seq1 );

    QVERIFY( seq1 == seq2 );
}

/* Compares QKeySequence constructurs with int or QString arguments
   We don't do this for 3.0 since it doesn't support unicode accelerators */
void tst_QKeySequence::compareConstructors()
{
    QFETCH( int, modifiers );
    QFETCH( int, keycode );
    QFETCH( QString, keystring );

    QKeySequence qstringSeq( keystring );
    QKeySequence intSeq( modifiers | keycode );

    QVERIFY( qstringSeq == intSeq );
}

void tst_QKeySequence::checkMultipleNames()
{
    QKeySequence oldK( "Ctrl+Page Up" );
    QKeySequence newK( "Ctrl+PgUp" );
    QVERIFY( oldK == newK );
}

void tst_QKeySequence::mnemonic()
{
#ifdef Q_WS_MAC
    QSKIP("mnemonics are not used on Mac OS X", SkipAll);
#endif
    QKeySequence k = QKeySequence::mnemonic("&Foo");
    QVERIFY(k == QKeySequence("ALT+F"));
    k = QKeySequence::mnemonic("&& &x");
    QVERIFY(k == QKeySequence("ALT+X"));
}

void tst_QKeySequence::toString_data()
{
    QTest::addColumn<QString>("strSequence");
    QTest::addColumn<QString>("neutralString");
    QTest::addColumn<QString>("platformString");


#ifndef Q_WS_MAC
    QTest::newRow("Ctrl+Left") << QString("Ctrl+Left") << QString("Ctrl+Left") << QString("Ctrl+Left");
    QTest::newRow("Alt+Left") << QString("Alt+Left") << QString("Alt+Left") << QString("Alt+Left");
    QTest::newRow("Alt+Shift+Left") << QString("Alt+Shift+Left") << QString("Alt+Shift+Left") << QString("Alt+Shift+Left");
    QTest::newRow("Ctrl") << QString("Ctrl+\x0c5") << QString("Ctrl+\x0c5") << QString("Ctrl+\x0c5");
    QTest::newRow("Alt") << QString("Alt+\x0c5") << QString("Alt+\x0c5") << QString("Alt+\x0c5");
    QTest::newRow("Shift") << QString("Shift+\x0c5") << QString("Shift+\x0c5") << QString("Shift+\x0c5");
    QTest::newRow("Meta") << QString("Meta+\x0c5") << QString("Meta+\x0c5") << QString("Meta+\x0c5");
    QTest::newRow("Ctrl+Plus") << QString("Ctrl++") << QString("Ctrl++") << QString("Ctrl++");
#if QT_VERSION > 0x040100
    QTest::newRow("Ctrl+,") << QString("Ctrl+,") << QString("Ctrl+,") << QString("Ctrl+,");
    QTest::newRow("Ctrl+,,Ctrl+,") << QString("Ctrl+,,Ctrl+,") << QString("Ctrl+,, Ctrl+,") << QString("Ctrl+,, Ctrl+,");
#endif
    QTest::newRow("MultiKey") << QString("Alt+X, Ctrl+Y, Z") << QString("Alt+X, Ctrl+Y, Z")
                           << QString("Alt+X, Ctrl+Y, Z");

    QTest::newRow("Invalid") << QString("Ctrly") << QString("") << QString("");
#else
    QTest::newRow("Ctrl+Left") << MacCtrl + "Left" << QString("Ctrl+Left") << MacCtrl + "Left";
    QTest::newRow("Alt+Left") << MacAlt + "Left" << QString("Alt+Left") << MacAlt + "Left";
    QTest::newRow("Alt+Shift+Left") << MacAlt + MacShift + "Left" << QString("Alt+Shift+Left")
                                 << MacAlt + MacShift + "Left";
    QTest::newRow("Ctrl") << MacCtrl + "\x0c5" << QString("Ctrl+\x0c5") << MacCtrl + "\x0c5";
    QTest::newRow("Alt") << MacAlt + "\x0c5" << QString("Alt+\x0c5") << MacAlt + "\x0c5";
    QTest::newRow("Shift") << MacShift + "\x0c5" << QString("Shift+\x0c5") << MacShift + "\x0c5";
    QTest::newRow("Meta") << MacMeta + "\x0c5" << QString("Meta+\x0c5") << MacMeta + "\x0c5";
    QTest::newRow("Ctrl+Plus") << MacCtrl + "+" << QString("Ctrl++") << MacCtrl + "+";
#if QT_VERSION > 0x040100
    QTest::newRow("Ctrl+,") << MacCtrl + "," << QString("Ctrl+,") << MacCtrl + ",";
    QTest::newRow("Ctrl+,,Ctrl+,") << MacCtrl + ",, " + MacCtrl + "," << QString("Ctrl+,, Ctrl+,") << MacCtrl + ",, " + MacCtrl + ",";
#endif
    QTest::newRow("MultiKey") << MacAlt + "X, " + MacCtrl + "Y, Z" << QString("Alt+X, Ctrl+Y, Z")
                           << MacAlt + "X, " + MacCtrl + "Y, Z";
    QTest::newRow("Invalid") << QString("Ctrly") << QString("") << QString("");
#endif
}

void tst_QKeySequence::toString()
{
    QFETCH(QString, strSequence);
    QFETCH(QString, neutralString);
    QFETCH(QString, platformString);

    QKeySequence ks1(strSequence);

    QCOMPARE(ks1.toString(QKeySequence::NativeText), platformString);
    QCOMPARE(ks1.toString(QKeySequence::PortableText), neutralString);

}

void tst_QKeySequence::streamOperators_data()
{
	operatorQString_data();
}

void tst_QKeySequence::streamOperators()
{
    QFETCH( int, modifiers );
    QFETCH( int, keycode );

	QByteArray data;
	QKeySequence refK( modifiers | keycode );
	QKeySequence orgK( "Ctrl+A" );
	QKeySequence copyOrgK = orgK;
	QVERIFY( copyOrgK == orgK );
	
	QDataStream in(&data, QIODevice::WriteOnly);
	QDataStream out(&data, QIODevice::ReadOnly);

	in << refK;
	out >> orgK;
	QVERIFY( orgK == refK );

	// check if detached
	QVERIFY( orgK != copyOrgK );
}

void tst_QKeySequence::fromString_data()
{
    toString_data();
}

void tst_QKeySequence::fromString()
{
    QFETCH(QString, strSequence);
    QFETCH(QString, neutralString);
    QFETCH(QString, platformString);

    QKeySequence ks1(strSequence);
    QKeySequence ks2 = QKeySequence::fromString(ks1.toString());
    QKeySequence ks3 = QKeySequence::fromString(neutralString, QKeySequence::PortableText);
    QKeySequence ks4 = QKeySequence::fromString(platformString, QKeySequence::NativeText);

    // assume the transitive property exists here.
    QCOMPARE(ks2, ks1);
    QCOMPARE(ks3, ks1);
    QCOMPARE(ks4, ks1);
}

QTEST_MAIN(tst_QKeySequence)
#include "tst_qkeysequence.moc"

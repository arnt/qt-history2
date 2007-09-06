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
#include <quuid.h>



#include <quuid.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/plugin/quuid.h corelib/plugin/quuid.cpp

class tst_QUuid : public QObject
{
    Q_OBJECT

public:
    tst_QUuid();

private slots:
    void toString();
    void isNull();
    void equal();
    void notEqual();

    // Only in Qt > 3.2.x
    void generate();
    void less();
    void more();
    void variants();
    void versions();

public:
    // Variables
    QUuid uuidA;
    QUuid uuidB;
};

tst_QUuid::tst_QUuid()
{
    uuidA = "{fc69b59e-cc34-4436-a43c-ee95d128b8c5}";
    uuidB = "{1ab6e93a-b1cb-4a87-ba47-ec7e99039a7b}";
}


void tst_QUuid::toString()
{
    QCOMPARE(uuidA.toString(), QString("{fc69b59e-cc34-4436-a43c-ee95d128b8c5}"));
}


void tst_QUuid::isNull()
{
    QVERIFY( !uuidA.isNull() );

    QUuid should_be_null_uuid;
    QVERIFY( should_be_null_uuid.isNull() );
}


void tst_QUuid::equal()
{
    QVERIFY( !(uuidA == uuidB) );

    QUuid copy(uuidA);
    QVERIFY(uuidA == copy);

    QUuid assigned;
    assigned = uuidA;
    QVERIFY(uuidA == assigned);
}


void tst_QUuid::notEqual()
{
    QVERIFY( uuidA != uuidB );
}


void tst_QUuid::generate()
{
    QUuid shouldnt_be_null_uuidA = QUuid::createUuid();
    QUuid shouldnt_be_null_uuidB = QUuid::createUuid();
    QVERIFY( !shouldnt_be_null_uuidA.isNull() );
    QVERIFY( !shouldnt_be_null_uuidB.isNull() );
    QVERIFY( shouldnt_be_null_uuidA != shouldnt_be_null_uuidB );
}


void tst_QUuid::less()
{
    QVERIFY( !(uuidA < uuidB) );

    QUuid null_uuid;
    QVERIFY(null_uuid < uuidA); // Null uuid is always less than a valid one
}


void tst_QUuid::more()
{
    QVERIFY( uuidA > uuidB );

    QUuid null_uuid;
    QVERIFY( !(null_uuid > uuidA) ); // Null uuid is always less than a valid one
}


void tst_QUuid::variants()
{
    QVERIFY( uuidA.variant() == QUuid::DCE );
    QVERIFY( uuidB.variant() == QUuid::DCE );

    QUuid NCS = "{3a2f883c-4000-000d-0000-00fb40000000}";
    QVERIFY( NCS.variant() == QUuid::NCS );
}


void tst_QUuid::versions()
{
    QVERIFY( uuidA.version() == QUuid::Random );
    QVERIFY( uuidB.version() == QUuid::Random );

    QUuid DCE_time= "{406c45a0-3b7e-11d0-80a3-0000c08810a7}";
    QVERIFY( DCE_time.version() == QUuid::Time );

    QUuid NCS = "{3a2f883c-4000-000d-0000-00fb40000000}";
    QVERIFY( NCS.version() == QUuid::VerUnknown );
}


QTEST_MAIN(tst_QUuid)
#include "tst_quuid.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qtranslator.h>
#include <qfile.h>




//TESTED_CLASS=
//TESTED_FILES=corelib/kernel/qtranslator.h corelib/kernel/qtranslator.cpp

class tst_QTranslator : public QObject
{
    Q_OBJECT

public:
    tst_QTranslator();
    virtual ~tst_QTranslator();


public slots:
    void init();
    void cleanup();
private slots:
    void load();
    void load2();
    void threadeLoad();
};


tst_QTranslator::tst_QTranslator()
{
}

tst_QTranslator::~tst_QTranslator()
{
}

void tst_QTranslator::init()
{
}

void tst_QTranslator::cleanup()
{
}

void tst_QTranslator::load()
{

    QTranslator tor( 0 );
    tor.load("hellotr_la");
    QVERIFY(!tor.isEmpty());
    QCOMPARE(tor.translate("QPushButton", "Hello world!"), QString::fromLatin1("Hallo Welt!"));
}

void tst_QTranslator::load2()
{
    QTranslator tor( 0 );
    QFile file("hellotr_la.qm");
    file.open(QFile::ReadOnly);
    QByteArray data = file.readAll();
    tor.load((const uchar *)data.constData(), data.length());
    QVERIFY(!tor.isEmpty());
    QCOMPARE(tor.translate("QPushButton", "Hello world!"), QString::fromLatin1("Hallo Welt!"));
}

class TranslatorThread : public QThread
{
    void run() {
        QTranslator tor( 0 );
        tor.load("hellotr_la");

        if (tor.isEmpty())
            qFatal("Could not load translation");
        if (tor.translate("QPushButton", "Hello world!") !=  QString::fromLatin1("Hallo Welt!"))
            qFatal("Test string was not translated correctlys");
    }
};


void tst_QTranslator::threadeLoad()
{
    TranslatorThread thread;
    thread.start();
    QVERIFY(thread.wait(10 * 1000));
}

QTEST_MAIN(tst_QTranslator)
#include "tst_qtranslator.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QWidget>
#include <qtranslator.h>
#include <qfile.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/kernel/qtranslator.h corelib/kernel/qtranslator.cpp

class tst_QTranslator : public QWidget
{
    Q_OBJECT

public:
    tst_QTranslator();
    virtual ~tst_QTranslator();

public slots:
    void init();
    void cleanup();

protected:
    bool event(QEvent *event);

private slots:
    void load();
    void load2();
    void threadLoad();
    void testLanguageChange();
    void plural();

private:
    int languageChangeEventCounter;
};


tst_QTranslator::tst_QTranslator()
    : languageChangeEventCounter(0)
{
    show();
    hide();
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

bool tst_QTranslator::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        ++languageChangeEventCounter;
    return QWidget::event(event);
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


void tst_QTranslator::threadLoad()
{
    TranslatorThread thread;
    thread.start();
    QVERIFY(thread.wait(10 * 1000));
}

void tst_QTranslator::testLanguageChange()
{
    languageChangeEventCounter = 0;

    QTranslator *tor = new QTranslator;
    tor->load("hellotr_la.qm");
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 0);

    tor->load("doesn't exist, same as clearing");
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 0);

    tor->load("hellotr_la.qm");
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 0);

    qApp->installTranslator(tor);
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 1);

    tor->load("doesn't exist, same as clearing");
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 2);

    tor->load("hellotr_la.qm");
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 3);

    qApp->removeTranslator(tor);
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 4);

    tor->load("doesn't exist, same as clearing");
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 4);

    qApp->installTranslator(tor);
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 4);

    tor->load("hellotr_la.qm");
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 5);

    delete tor;
    tor = 0;
    qApp->sendPostedEvents();
    qApp->sendPostedEvents();
    QCOMPARE(languageChangeEventCounter, 6);
}


void tst_QTranslator::plural()
{

    QTranslator tor( 0 );
    tor.load("hellotr_la");
    QVERIFY(!tor.isEmpty());
    QCoreApplication::installTranslator(&tor);
    QCoreApplication::Encoding e = QCoreApplication::UnicodeUTF8;
    QCOMPARE(QCoreApplication::translate("QPushButton", "Hello %n world(s)!", 0, e, 0), QString::fromLatin1("Hallo 0 Welten!"));
    QCOMPARE(QCoreApplication::translate("QPushButton", "Hello %n world(s)!", 0, e, 1), QString::fromLatin1("Hallo 1 Welt!"));
    QCOMPARE(QCoreApplication::translate("QPushButton", "Hello %n world(s)!", 0, e, 2), QString::fromLatin1("Hallo 2 Welten!"));
}

QTEST_MAIN(tst_QTranslator)
#include "tst_qtranslator.moc"

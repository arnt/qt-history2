/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <QIODevice>
#include <QLabel>
#include <QMovie>

//TESTED_FILES=gui/image/qmovie.h gui/image/qmovie.cpp

class tst_QMovie : public QObject
{
    Q_OBJECT

public:
    tst_QMovie();
    virtual ~tst_QMovie();

public slots:
    void init();
    void cleanup();

protected slots:
    void exitLoopSlot();

private slots:
    void getSetCheck();
    void construction();
    void playMovie_data();
    void playMovie();
    void jumpToFrame_data();
    void jumpToFrame();
#if QT_VERSION >= 0x040101
    void changeMovieFile();
#endif // QT_VERSION
};

// Testing get/set functions
void tst_QMovie::getSetCheck()
{
    QMovie obj1;
    // QIODevice * QMovie::device()
    // void QMovie::setDevice(QIODevice *)
    QFile *var1 = new QFile;
    obj1.setDevice(var1);
    QCOMPARE(obj1.device(), (QIODevice *)var1);
    obj1.setDevice((QIODevice *)0);
    QCOMPARE(obj1.device(), (QIODevice *)0);
    delete var1;

    // CacheMode QMovie::cacheMode()
    // void QMovie::setCacheMode(CacheMode)
    obj1.setCacheMode(QMovie::CacheMode(QMovie::CacheNone));
    QCOMPARE(QMovie::CacheMode(QMovie::CacheNone), obj1.cacheMode());
    obj1.setCacheMode(QMovie::CacheMode(QMovie::CacheAll));
    QCOMPARE(QMovie::CacheMode(QMovie::CacheAll), obj1.cacheMode());

    // int QMovie::speed()
    // void QMovie::setSpeed(int)
    obj1.setSpeed(0);
    QCOMPARE(0, obj1.speed());
    obj1.setSpeed(INT_MIN);
    QCOMPARE(INT_MIN, obj1.speed());
    obj1.setSpeed(INT_MAX);
    QCOMPARE(INT_MAX, obj1.speed());
}

tst_QMovie::tst_QMovie()
{
}

tst_QMovie::~tst_QMovie()
{

}

void tst_QMovie::init()
{
}

void tst_QMovie::cleanup()
{
}

void tst_QMovie::exitLoopSlot()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_QMovie::construction()
{
    QMovie movie;
    QCOMPARE(movie.device(), (QIODevice *)0);
    QCOMPARE(movie.fileName(), QString());
    QCOMPARE(movie.state(), QMovie::NotRunning);
}

void tst_QMovie::playMovie_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<int>("frameCount");
#ifdef QTEST_HAVE_MNG
    QTest::newRow("home") << QString("animations/australia1.mng") << 2;
#endif
#ifdef QTEST_HAVE_GIF
    QTest::newRow("comicsecard") << QString("animations/comicsecard.gif") << 5;
    QTest::newRow("trolltech") << QString("animations/trolltech.gif") << 34;
#endif
}

void tst_QMovie::playMovie()
{
    QFETCH(QString, fileName);
    QFETCH(int, frameCount);
    
    QMovie movie(fileName);

    QCOMPARE(movie.state(), QMovie::NotRunning);
    movie.setSpeed(1000);
    movie.start();
    QCOMPARE(movie.state(), QMovie::Running);
    movie.setPaused(true);
    QCOMPARE(movie.state(), QMovie::Paused);
    movie.start();
    QCOMPARE(movie.state(), QMovie::Running);
    movie.stop();
    QCOMPARE(movie.state(), QMovie::NotRunning);
    movie.jumpToFrame(0);
    QCOMPARE(movie.state(), QMovie::NotRunning);
    movie.start();
    QCOMPARE(movie.state(), QMovie::Running);

    connect(&movie, SIGNAL(finished()), this, SLOT(exitLoopSlot()));

    QLabel label;
    label.setMovie(&movie);
    label.show();

    QTestEventLoop::instance().enterLoop(20);
    QVERIFY2(!QTestEventLoop::instance().timeout(),
            "Timed out while waiting for finished() signal");

    QCOMPARE(movie.state(), QMovie::NotRunning);
    QCOMPARE(movie.frameCount(), frameCount);
}

void tst_QMovie::jumpToFrame_data()
{
    playMovie_data();
}

void tst_QMovie::jumpToFrame()
{
    QFETCH(QString, fileName);
    QMovie movie(fileName);
    movie.start();
    movie.stop();
    QVERIFY(movie.jumpToFrame(-1) == false);
    QVERIFY(movie.currentFrameNumber() == 0);
}

#if QT_VERSION >= 0x040101
void tst_QMovie::changeMovieFile()
{
    QMovie movie("animations/comicsecard.gif");
    movie.start();
    movie.stop();
    movie.setFileName("animations/trolltech.gif");
    QVERIFY(movie.currentFrameNumber() == -1);
}

#endif // QT_VERSION

QTEST_MAIN(tst_QMovie)
#include "tst_qmovie.moc"

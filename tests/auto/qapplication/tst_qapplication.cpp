/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//#define QT_TST_QAPP_DEBUG
#include <qdebug.h>

#include <QtTest/QtTest>

#include "qabstracteventdispatcher.h"
#include <QtGui>

#include "private/qstylesheetstyle_p.h"

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qapplication.h gui/kernel/qapplication.cpp

class tst_QApplication : public QObject
{
Q_OBJECT

public:
    tst_QApplication();
    virtual ~tst_QApplication();

public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void staticSetup();

    void alert();

    void multiple_data();
    void multiple();

    void nonGui();

    void setFont_data();
    void setFont();

    void args_data();
    void args();

    void lastWindowClosed();
    void testDeleteLater();
    void testDeleteLaterProcessEvents();

    void libraryPaths();
    void libraryPaths_qt_plugin_path();
    void libraryPaths_qt_plugin_path_2();

    void sendPostedEvents();
    void postEventRace();

    void thread();
    void desktopSettingsAware();

    void setActiveWindow();

    void focusChanged();

    void execAfterExit();

    void wheelScrollLines();

    void task109149();

    void style();

    void allWidgets();

    void setAttribute();
    void windowsCommandLine();
private:
    inline QChar pathSeparator(void)
    {
#ifdef Q_OS_WIN
        return QChar(';');
#else
        return QChar(':');
#endif
    }

};

class MyInputContext : public QInputContext
{
public:
    MyInputContext() : QInputContext() {}
    QString identifierName() { return QString("NoName"); }
    QString language() { return QString("NoLanguage"); }
    void reset() {}
    bool isComposing() const { return false; }
};

// Testing get/set functions
void tst_QApplication::getSetCheck()
{
    int argc = 0;
    QApplication obj1(argc, 0, QApplication::GuiServer);
    // QInputContext * QApplication::inputContext()
    // void QApplication::setInputContext(QInputContext *)
    MyInputContext *var1 = new MyInputContext;
    obj1.setInputContext(var1);
    QCOMPARE((QInputContext *)var1, obj1.inputContext());
    QTest::ignoreMessage(QtWarningMsg, "QApplication::setInputContext: called with 0 input context");
    obj1.setInputContext((QInputContext *)0);
    QCOMPARE((QInputContext *)var1, obj1.inputContext());
    // delete var1; // No delete, since QApplication takes ownership
}

static  char *argv0;

tst_QApplication::tst_QApplication()
{
}

tst_QApplication::~tst_QApplication()
{

}

void tst_QApplication::init()
{
// TODO: Add initialization code here.
// This will be executed immediately before each test is run.
}

void tst_QApplication::cleanup()
{
// TODO: Add cleanup code here.
// This will be executed immediately after each test is run.
}

void tst_QApplication::staticSetup()
{
    QVERIFY(!qApp);

    QStyle *style = QStyleFactory::create(QLatin1String("Windows"));
    QVERIFY(style);
    QApplication::setStyle(style);

    QPalette pal;
    QApplication::setPalette(pal);

    /*QFont font;
    QApplication::setFont(font);*/

    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);
}


// QApp subclass that exits the event loop after 150ms
class TestApplication : public QApplication
{
public:
    TestApplication( int &argc, char **argv )
	: QApplication( argc, argv, QApplication::GuiServer )
    {
	startTimer( 150 );
    }

    void timerEvent( QTimerEvent * )
    {
        quit();
    }
};

void tst_QApplication::alert()
{
    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);
    app.alert(0, 0);

    QWidget widget;
    QWidget widget2;
    app.alert(&widget, 100);
    widget.show();
    widget2.show();
#ifdef Q_WS_X11
    extern void qt_x11_wait_for_window_manager( QWidget* w );
    qt_x11_wait_for_window_manager(&widget);
    qt_x11_wait_for_window_manager(&widget2);
#endif
    QTest::qWait(100);
    app.alert(&widget, -1);
    app.alert(&widget, 250);
    widget2.activateWindow();
    QApplication::setActiveWindow(&widget2);
    app.alert(&widget, 0);
    widget.activateWindow();
    QApplication::setActiveWindow(&widget);
    app.alert(&widget, 200);
    app.syncX();
}

void tst_QApplication::multiple_data()
{
    QTest::addColumn<QStringList>("features");

    // return a list of things to try
    QTest::newRow( "data0" ) << QStringList( "" );
    QTest::newRow( "data1" ) << QStringList( "QFont" );
    QTest::newRow( "data2" ) << QStringList( "QPixmap" );
    QTest::newRow( "data3" ) << QStringList( "QWidget" );
}

void tst_QApplication::multiple()
{
    QFETCH(QStringList,features);

    int i = 0;
    int argc = 0;
    while ( i++ < 5 ) {
	TestApplication app( argc, 0 );

	if ( features.contains( "QFont" ) ) {
	    // create font and force loading
	    QFont font( "Arial", 12 );
	    QFontInfo finfo( font );
	    finfo.exactMatch();
	}
	if ( features.contains( "QPixmap" ) ) {
	    QPixmap pix( 100, 100 );
	    pix.fill( Qt::black );
	}
	if ( features.contains( "QWidget" ) ) {
	    QWidget widget;
	}

	QVERIFY(!app.exec());
    }
}

void tst_QApplication::nonGui()
{
#ifdef Q_OS_HPUX
    // ### This is only to allow us to generate a test report for now.
    QSKIP("This test shuts down the window manager on HP-UX.", SkipAll);
#endif

    int argc = 0;
    QApplication app(argc, 0, false);
    QCOMPARE(qApp, &app);
}

void tst_QApplication::setFont_data()
{
    QTest::addColumn<QString>("family");
    QTest::addColumn<int>("pointsize");
    QTest::addColumn<bool>("beforeAppConstructor");

    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer); // Needed for QFontDatabase

    int cnt = 0;
    QFontDatabase fdb;
    QStringList families = fdb.families();
    for (QStringList::const_iterator itr = families.begin();
	 itr != families.end();
	 ++itr) {
	if (cnt < 3) {
	    QString family = *itr;
	    QStringList styles = fdb.styles(family);
	    if (styles.size() > 0) {
		QString style = styles.first();
		QList<int> sizes = fdb.pointSizes(family, style);
		if (!sizes.size())
		    sizes = fdb.standardSizes();
		if (sizes.size() > 0) {
		    QTest::newRow(QString("data%1a").arg(cnt).toLatin1().constData())
			<< family
			<< sizes.first()
                        << false;
		    QTest::newRow(QString("data%1b").arg(cnt).toLatin1().constData())
			<< family
			<< sizes.first()
                        << true;
                }
	    }
	}
	++cnt;
    }

    QTest::newRow("nonexistingfont") << "nosuchfont_probably_quiteunlikely"
        << 0 << false;
    QTest::newRow("nonexistingfont") << "nosuchfont_probably_quiteunlikely"
        << 0 << true;

    QTest::newRow("largescaleable") << "smoothtimes" << 100 << false;
    QTest::newRow("largescaleable") << "smoothtimes" << 100 << true;

    QTest::newRow("largeunscaleale") << "helvetica" << 100 << false;
    QTest::newRow("largeunscaleale") << "helvetica" << 100 << true;
}

void tst_QApplication::setFont()
{
    QFETCH( QString, family );
    QFETCH( int, pointsize );
    QFETCH( bool, beforeAppConstructor );

    QFont font( family, pointsize );
    if (beforeAppConstructor)
        QApplication::setFont( font );

    int argc = 0;
    QApplication app( argc, 0, QApplication::GuiServer );
    if (!beforeAppConstructor)
        QApplication::setFont( font );

    QCOMPARE( app.font(), font );
}

void tst_QApplication::args_data()
{
    QTest::addColumn<int>("argc_in");
    QTest::addColumn<QString>("args_in");
    QTest::addColumn<int>("argc_out");
    QTest::addColumn<QString>("args_out");

    QTest::newRow( "App name" ) << 1 << "/usr/bin/appname" << 1 << "/usr/bin/appname";
    QTest::newRow( "No arguments" ) << 0 << QString() << 0 << QString();
    QTest::newRow( "App name, style" ) << 3 << "/usr/bin/appname -style motif" << 1 << "/usr/bin/appname";
    QTest::newRow( "App name, style, arbitrary, reverse" ) << 5 << "/usr/bin/appname -style motif -arbitrary -reverse"
							<< 2 << "/usr/bin/appname -arbitrary";
}

void tst_QApplication::task109149()
{
    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);
    QApplication::setFont(QFont("helvetica", 100));

    QWidget w;
    w.setWindowTitle("hello");
    w.show();

    app.processEvents();
}

static char ** QString2cstrings( const QString &args )
{
    static QList<QByteArray> cache;

    int i;
    char **argarray = 0;
    QStringList list = args.split(' ');;
    argarray = new char*[list.count()+1];

    for (i = 0; i < (int)list.count(); ++i ) {
        QByteArray l1 = list[i].toLatin1();
        argarray[i] = l1.data();
        cache.append(l1);
    }
    argarray[i] = 0;

    return argarray;
}

static QString cstrings2QString( char **args )
{
    QString string;
    if ( !args )
	return string;

    int i = 0;
    while ( args[i] ) {
	string += args[i];
	if ( args[i+1] )
	    string += " ";
	++i;
    }
    return string;
}

void tst_QApplication::args()
{
    QFETCH( int, argc_in );
    QFETCH( QString, args_in );
    QFETCH( int, argc_out );
    QFETCH( QString, args_out );

    char **argv = QString2cstrings( args_in );

    QApplication app( argc_in, argv, QApplication::GuiServer );
    QString argv_out = cstrings2QString(argv);

    QCOMPARE( argc_in, argc_out );
    QCOMPARE( argv_out, args_out );

    delete [] argv;
}

class CloseWidget : public QWidget
{
    Q_OBJECT
public:
    CloseWidget()
    {
        startTimer(500);
    }

protected:
    void timerEvent(QTimerEvent *)
    {
        close();
    }

};

void tst_QApplication::lastWindowClosed()
{
    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);

    QSignalSpy spy(&app, SIGNAL(lastWindowClosed()));

    QPointer<QDialog> dialog = new QDialog;
    QVERIFY(dialog->testAttribute(Qt::WA_QuitOnClose));
    QTimer::singleShot(1000, dialog, SLOT(accept()));
    dialog->exec();
    QVERIFY(dialog);
    QCOMPARE(spy.count(), 0);

    QPointer<CloseWidget>widget = new CloseWidget;
    QVERIFY(widget->testAttribute(Qt::WA_QuitOnClose));
    QObject::connect(&app, SIGNAL(lastWindowClosed()), widget, SLOT(deleteLater()));
    app.exec();
    QVERIFY(!widget);
    QCOMPARE(spy.count(), 1);
    spy.clear();

#if 0
    // everything is closed, so doing this should not emit lastWindowClosed() again
    QMetaObject::invokeMethod(dialog, "close", Qt::QueuedConnection);
    QTimer::singleShot(1000, &app, SLOT(quit()));
    app.exec();
    QCOMPARE(spy.count(), 0);
#endif

    delete dialog;

    // show 3 windows, close them, should only get lastWindowClosed once
    QWidget w1;
    QWidget w2;
    QWidget w3;
    w1.show();
    w2.show();
    w3.show();

    QTimer::singleShot(1000, &app, SLOT(closeAllWindows()));
    app.exec();
    QCOMPARE(spy.count(), 1);
}

#define QT_TST_QAPP_DEBUG
void tst_QApplication::libraryPaths()
{
    int argc = 1;
    QApplication app(argc, &argv0, QApplication::GuiServer);

#ifdef QT_TST_QAPP_DEBUG
    qDebug() << "Initial library path:" << app.libraryPaths();
#endif
    int count = app.libraryPaths().count();
    QString installPathPlugins =  QLibraryInfo::location(QLibraryInfo::PluginsPath);
    app.addLibraryPath(installPathPlugins);
#ifdef QT_TST_QAPP_DEBUG
    qDebug() << "installPathPlugins" << installPathPlugins;
    qDebug() << "After adding plugins path:" << app.libraryPaths();
#endif
    QCOMPARE(app.libraryPaths().count(), count);

    QString appDirPath = app.applicationDirPath();

    app.addLibraryPath(appDirPath);
    app.addLibraryPath(appDirPath + "/..");
#ifdef QT_TST_QAPP_DEBUG
    qDebug() << "appDirPath" << appDirPath;
    qDebug() << "After adding appDirPath && appDirPath + /..:" << app.libraryPaths();
#endif
    QCOMPARE(app.libraryPaths().count(), count + 1);
#ifdef Q_OS_MAC
    app.addLibraryPath(appDirPath + "/../MacOS");
#else
    app.addLibraryPath(appDirPath + "/tmp/..");
#endif
#ifdef QT_TST_QAPP_DEBUG
    qDebug() << "After adding appDirPath + /tmp/..:" << app.libraryPaths();
#endif
    QCOMPARE(app.libraryPaths().count(), count + 1);


}

void tst_QApplication::libraryPaths_qt_plugin_path()
{
    int argc = 1;

    QApplication app(argc, &argv0, QApplication::GuiServer);
    QString appDirPath = app.applicationDirPath();

    // Our hook into libraryPaths() initialization: Set the QT_PLUGIN_PATH environment variable
    QString installPathPluginsDeCanon = QString::fromLatin1("QT_PLUGIN_PATH=") + appDirPath + QString::fromLatin1("/tmp/..");
    QByteArray ascii = installPathPluginsDeCanon.toAscii();
    putenv(strdup(ascii.data()));

    QVERIFY(!app.libraryPaths().contains(appDirPath + QString::fromLatin1("/tmp/..")));
}

void tst_QApplication::libraryPaths_qt_plugin_path_2()
{
    int argc = 1;

    QApplication app(argc, &argv0, QApplication::GuiServer);
    QString appDirPath = app.applicationDirPath();

    // Our hook into libraryPaths() initialization: Set the QT_PLUGIN_PATH environment variable
    QString installPathPluginsDeCanon = QString::fromLatin1("QT_PLUGIN_PATH=") + appDirPath + QString::fromLatin1("/tmp/..") +
                                                                        pathSeparator() + appDirPath + QString::fromLatin1("/..");
    QByteArray ascii = installPathPluginsDeCanon.toAscii();
    putenv(strdup(ascii.data()));

    QVERIFY(!app.libraryPaths().contains(appDirPath + QString::fromLatin1("/tmp/..")));
    QVERIFY(app.libraryPaths().contains( QDir(appDirPath + QString::fromLatin1("/..")).canonicalPath() ));
}

class SendPostedEventsTester : public QObject
{
    Q_OBJECT
public:
    QList<int> eventSpy;
    bool event(QEvent *e);
private slots:
    void doTest();
};

bool SendPostedEventsTester::event(QEvent *e)
{
    eventSpy.append(e->type());
    return QObject::event(e);
}

void SendPostedEventsTester::doTest()
{
    QPointer<SendPostedEventsTester> p = this;
    QApplication::postEvent(this, new QEvent(QEvent::User));
    // DeferredDelete should not be delivered until returning from this function
    QApplication::postEvent(this, new QEvent(QEvent::DeferredDelete));

    QEventLoop eventLoop;
    QMetaObject::invokeMethod(&eventLoop, "quit", Qt::QueuedConnection);
    eventLoop.exec();
    QVERIFY(p != 0);

    QCOMPARE(eventSpy.count(), 2);
    QCOMPARE(eventSpy.at(0), int(QEvent::MetaCall));
    QCOMPARE(eventSpy.at(1), int(QEvent::User));
    eventSpy.clear();
}

void tst_QApplication::sendPostedEvents()
{
    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);
    SendPostedEventsTester *tester = new SendPostedEventsTester;
    QMetaObject::invokeMethod(tester, "doTest", Qt::QueuedConnection);
    QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
    QPointer<SendPostedEventsTester> p = tester;
    (void) app.exec();
    QVERIFY(p == 0);
}

class RaceThread : public QThread
{
    Q_OBJECT
    QMutex mutex;
    QList<QPointer<QObject> > objects;

public slots:
    void newObject(QObject *object)
    {
        QMutexLocker locker(&mutex);
        objects.append(object);
        start();
    }

public:
    void run()
    {
        forever {
            QObject *object = 0;
            {
                QMutexLocker locker(&mutex);
                if (!objects.isEmpty())
                    object = objects.takeFirst();
            }
            // this is the race... after dereferencing the QPointer
            // the object could be destroyed...
            QTime t;
            t.start();
            while (t.elapsed() < 100)
                ;
            if (!object)
                break;
            QApplication::postEvent(object, new QEvent(QEvent::User));
        }
    }
};

class RaceCoordinator : public QObject
{
    Q_OBJECT
    RaceThread *thread;
public slots:
    void nextObject()
    {
        QObject *object = new QObject;
        emit newObject(object);
        QTimer::singleShot(100, object, SLOT(deleteLater()));
    }
signals:
    void newObject(QObject *object);
};

void tst_QApplication::postEventRace()
{
    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);

    RaceCoordinator coordinator;

    enum { ThreadCount = 1 };
    RaceThread *threads[ThreadCount];
    for (int i = 0; i < ThreadCount; ++i) {
        threads[i] = new RaceThread;
        connect(&coordinator, SIGNAL(newObject(QObject *)),
                threads[i], SLOT(newObject(QObject *)), Qt::DirectConnection);
    }
    for (int i = 0; i < ThreadCount; ++i)
        threads[i]->start();

    QTimer zeroTimer;
    connect(&zeroTimer, SIGNAL(timeout()), &coordinator, SLOT(nextObject()));
    zeroTimer.start(0);
    QTimer stopTimer;
    connect(&stopTimer, SIGNAL(timeout()), &app, SLOT(quit()));
    stopTimer.start(60 * 1000); // one minute
    (void) app.exec();

    QVERIFY(threads[0]->wait(10000));
    for (int i = 1; i < ThreadCount; ++i) {
        QVERIFY(threads[i]->wait(10000));
        delete threads[i];
    }
}

void tst_QApplication::thread()
{
    QThread *currentThread = QThread::currentThread();
    // no app, but still have a valid thread
    QVERIFY(currentThread != 0);

    // the thread should be running and not finished
    QVERIFY(currentThread->isRunning());
    QVERIFY(!currentThread->isFinished());

    // this should probably be in the tst_QObject::thread() test, but
    // we put it here since we want to make sure that objects created
    // *before* the QApplication has a thread
    QObject object;
    QObject child(&object);
    QVERIFY(object.thread() == currentThread);
    QVERIFY(child.thread() == currentThread);

    {
        int argc = 0;
        QApplication app(argc, 0, QApplication::GuiServer);

        // current thread still valid
        QVERIFY(QThread::currentThread() != 0);
        // thread should be the same as before
        QCOMPARE(QThread::currentThread(), currentThread);

        // app's thread should be the current thread
        QCOMPARE(app.thread(), currentThread);

        // the thread should still be running and not finished
        QVERIFY(currentThread->isRunning());
        QVERIFY(!currentThread->isFinished());

        QTestEventLoop::instance().enterLoop(1);
    }

    // app dead, current thread still valid
    QVERIFY(QThread::currentThread != 0);
    QCOMPARE(QThread::currentThread(), currentThread);

    // the thread should still be running and not finished
    QVERIFY(currentThread->isRunning());
    QVERIFY(!currentThread->isFinished());

    // should still have a thread
    QVERIFY(object.thread() == currentThread);
    QVERIFY(child.thread() == currentThread);

    // do the test again, making sure that the thread is the same as
    // before
    {
        int argc = 0;
        QApplication app(argc, 0, QApplication::GuiServer);

        // current thread still valid
        QVERIFY(QThread::currentThread() != 0);
        // thread should be the same as before
        QCOMPARE(QThread::currentThread(), currentThread);

        // app's thread should be the current thread
        QCOMPARE(app.thread(), currentThread);

        // the thread should be running and not finished
        QVERIFY(currentThread->isRunning());
        QVERIFY(!currentThread->isFinished());

        // should still have a thread
        QVERIFY(object.thread() == currentThread);
        QVERIFY(child.thread() == currentThread);

        QTestEventLoop::instance().enterLoop(1);
    }

    // app dead, current thread still valid
    QVERIFY(QThread::currentThread() != 0);
    QCOMPARE(QThread::currentThread(), currentThread);

    // the thread should still be running and not finished
    QVERIFY(currentThread->isRunning());
    QVERIFY(!currentThread->isFinished());

    // should still have a thread
    QVERIFY(object.thread() == currentThread);
    QVERIFY(child.thread() == currentThread);
}

class DeleteLaterWidget : public QWidget
{
    Q_OBJECT
public:
    DeleteLaterWidget(QApplication *_app, QWidget *parent = 0)
        : QWidget(parent) { app = _app; child_deleted = false; }

    bool child_deleted;
    QApplication *app;

public slots:
    void runTest();
    void checkDeleteLater();
    void childDeleted() { child_deleted = true; }
};


void DeleteLaterWidget::runTest()
{
    QObject *stillAlive = qFindChild<QObject*>(this, "deleteLater");

    QWidget *w = new QWidget(this);
    connect(w, SIGNAL(destroyed()), this, SLOT(childDeleted()));

    w->deleteLater();
    Q_ASSERT(!child_deleted);

    QDialog dlg;
    QTimer::singleShot(500, &dlg, SLOT(reject()));
    dlg.exec();

    Q_ASSERT(!child_deleted);
    app->processEvents();
    Q_ASSERT(!child_deleted);

    QTimer::singleShot(500, this, SLOT(checkDeleteLater()));

    app->processEvents();

    QVERIFY(!stillAlive); // verify at the end to make test terminate
}

void DeleteLaterWidget::checkDeleteLater()
{
    Q_ASSERT(child_deleted);

    close();
}

void tst_QApplication::testDeleteLater()
{
    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);
    connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    DeleteLaterWidget *wgt = new DeleteLaterWidget(&app);
    QTimer::singleShot(500, wgt, SLOT(runTest()));

    QObject *object = new QObject(wgt);
    object->setObjectName("deleteLater");
    object->deleteLater();

    QObject *stillAlive = qFindChild<QObject*>(wgt, "deleteLater");
    QVERIFY(stillAlive);

    app.exec();

    delete wgt;

}

class EventLoopNester : public QObject
{
    Q_OBJECT
public slots:
    void deleteLaterAndEnterLoop()
    {
        QEventLoop eventLoop;
        QPointer<QObject> p(this);
        deleteLater();
        /*
          DeferredDelete events are compressed, meaning this second
          deleteLater() will *not* delete the object in the nested
          event loop
        */
        QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
        QTimer::singleShot(1000, &eventLoop, SLOT(quit()));
        eventLoop.exec();
        QVERIFY(p);
    }
    void deleteLaterAndExitLoop()
    {
        QEventLoop eventLoop;
        QPointer<QObject> p(this);
        QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
        QMetaObject::invokeMethod(&eventLoop, "quit", Qt::QueuedConnection);
        eventLoop.exec();
        QVERIFY(p); // not dead yet
    }
    void deleteLaterAndProcessEvents()
    {
        QPointer<QObject> p(this);
        deleteLater();
        QApplication::processEvents();
        QVERIFY(p);
        QApplication::processEvents(QEventLoop::DeferredDeletion);
        QVERIFY(!p);
    }
};

void tst_QApplication::testDeleteLaterProcessEvents()
{
    int argc = 0;

    // Calling processEvents() with no event dispatcher does nothing.
    QObject *object = new QObject;
    QPointer<QObject> p(object);
    object->deleteLater();
    QApplication::processEvents();
    QVERIFY(p);
    delete object;

    {
        QApplication app(argc, 0, QApplication::GuiServer);
        // If you call processEvents() with an event dispatcher present, but
        // outside any event loops, deferred deletes are not processed unless
        // QEventLoop::DeferredDeletion is passed.
        object = new QObject;
        p = object;
        object->deleteLater();
        app.processEvents();
        QVERIFY(p);
        app.processEvents(QEventLoop::ProcessEventsFlag(0x10)); // 0x10 == QEventLoop::DeferredDeletion
        QVERIFY(!p);

        // If you call deleteLater() on an object when there is no parent
        // event loop, and then enter an event loop, the object will get
        // deleted.
        object = new QObject;
        p = object;
        object->deleteLater();
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();
        QVERIFY(!p);
    }
    {
        // When an object is in an event loop, then calls deleteLater() and enters
        // an event loop recursively, it should not die until the parent event
        // loop continues.
        QApplication app(argc, 0, QApplication::GuiServer);
        QEventLoop loop;
        EventLoopNester *nester = new EventLoopNester;
        p = nester;
        QTimer::singleShot(3000, &loop, SLOT(quit()));
        QTimer::singleShot(0, nester, SLOT(deleteLaterAndEnterLoop()));

        loop.exec();
        QVERIFY(!p);
    }

    {
        // When the event loop that calls deleteLater() is exited
        // immediately, the object should die when returning to the
        // parent event loop
        QApplication app(argc, 0, QApplication::GuiServer);
        QEventLoop loop;
        EventLoopNester *nester = new EventLoopNester;
        p = nester;
        QTimer::singleShot(3000, &loop, SLOT(quit()));
        QTimer::singleShot(0, nester, SLOT(deleteLaterAndExitLoop()));

        loop.exec();
        QVERIFY(!p);
    }

    {
        // when the event loop that calls deleteLater() also calls
        // processEvents() immediately afterwards, the object should
        // not die until the parent loop continues
        QApplication app(argc, 0, QApplication::GuiServer);
        QEventLoop loop;
        EventLoopNester *nester = new EventLoopNester();
        p = nester;
        QTimer::singleShot(3000, &loop, SLOT(quit()));
        QTimer::singleShot(0, nester, SLOT(deleteLaterAndProcessEvents()));

        loop.exec();
        QVERIFY(!p);
    }
}

/*
    Test for crash whith QApplication::setDesktopSettingsAware(false).
*/
void tst_QApplication::desktopSettingsAware()
{
    QProcess testProcess;
#if defined(Q_OS_WIN) && defined(QT_DEBUG)
    testProcess.start("desktopsettingsaware/debug/desktopsettingsaware");
#elif defined(Q_OS_WIN)
    testProcess.start("desktopsettingsaware/release/desktopsettingsaware");
#else
    testProcess.start("desktopsettingsaware/desktopsettingsaware");
#endif
    QVERIFY(testProcess.waitForFinished(10000));
    QCOMPARE(int(testProcess.state()), int(QProcess::NotRunning));
    QVERIFY(int(testProcess.error()) != int(QProcess::Crashed));
}

void tst_QApplication::setActiveWindow()
{
    int argc = 0;
    QApplication MyApp(argc, 0, QApplication::GuiServer);

    QWidget* w = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(w);

    QLineEdit* pb1 = new QLineEdit("Testbutton1", w);
    QLineEdit* pb2 = new QLineEdit("Test Line Edit", w);

    layout->addWidget(pb1);
    layout->addWidget(pb2);

    pb2->setFocus();
    pb2->setParent(0);
    delete pb2;

    w->show();
    QApplication::setActiveWindow(w); // needs this on twm (focus follows mouse)
    QVERIFY(pb1->hasFocus());
    delete w;
}


/* This might fail on some X11 window managers? */
void tst_QApplication::focusChanged()
{
    int argc = 0;
    QApplication app(argc, 0, QApplication::GuiServer);

    QSignalSpy spy(&app, SIGNAL(focusChanged(QWidget *, QWidget *)));
    QWidget *now = 0;
    QWidget *old = 0;

    QWidget parent1;
    QHBoxLayout hbox1(&parent1);
    QLabel lb1(&parent1);
    QLineEdit le1(&parent1);
    QPushButton pb1(&parent1);
    hbox1.addWidget(&lb1);
    hbox1.addWidget(&le1);
    hbox1.addWidget(&pb1);

    QCOMPARE(spy.count(), 0);

    parent1.show();
    QApplication::setActiveWindow(&parent1); // needs this on twm (focus follows mouse)
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).count(), 2);
    old = qVariantValue<QWidget*>(spy.at(0).at(0));
    now = qVariantValue<QWidget*>(spy.at(0).at(1));
    QVERIFY(now == &le1);
    QVERIFY(now == QApplication::focusWidget());
    QVERIFY(old == 0);
    spy.clear();
    QCOMPARE(spy.count(), 0);

    pb1.setFocus();
    QCOMPARE(spy.count(), 1);
    old = qVariantValue<QWidget*>(spy.at(0).at(0));
    now = qVariantValue<QWidget*>(spy.at(0).at(1));
    QVERIFY(now == &pb1);
    QVERIFY(now == QApplication::focusWidget());
    QVERIFY(old == &le1);
    spy.clear();

    lb1.setFocus();
    QCOMPARE(spy.count(), 1);
    old = qVariantValue<QWidget*>(spy.at(0).at(0));
    now = qVariantValue<QWidget*>(spy.at(0).at(1));
    QVERIFY(now == &lb1);
    QVERIFY(now == QApplication::focusWidget());
    QVERIFY(old == &pb1);
    spy.clear();

    lb1.clearFocus();
    QCOMPARE(spy.count(), 1);
    old = qVariantValue<QWidget*>(spy.at(0).at(0));
    now = qVariantValue<QWidget*>(spy.at(0).at(1));
    QVERIFY(now == 0);
    QVERIFY(now == QApplication::focusWidget());
    QVERIFY(old == &lb1);
    spy.clear();

    QWidget parent2;
    QHBoxLayout hbox2(&parent2);
    QLabel lb2(&parent2);
    QLineEdit le2(&parent2);
    QPushButton pb2(&parent2);
    hbox2.addWidget(&lb2);
    hbox2.addWidget(&le2);
    hbox2.addWidget(&pb2);

    parent2.show();
    QApplication::setActiveWindow(&parent2); // needs this on twm (focus follows mouse)
    QVERIFY(spy.count() > 0); // one for deactivation, one for activation on Windows
    old = qVariantValue<QWidget*>(spy.at(spy.count()-1).at(0));
    now = qVariantValue<QWidget*>(spy.at(spy.count()-1).at(1));
    QVERIFY(now == &le2);
    QVERIFY(now == QApplication::focusWidget());
    QVERIFY(old == 0);
    spy.clear();

    QTestKeyEvent tab(QTest::Press, Qt::Key_Tab, 0, 0);
    QTestKeyEvent backtab(QTest::Press, Qt::Key_Backtab, 0, 0);
    QTestMouseEvent click(QTest::MouseClick, Qt::LeftButton, 0, QPoint(5, 5), 0);

    bool tabAllControls = true;
#ifdef Q_WS_MAC
    // Mac has two modes, one where you tab to everything, one where you can
    // only tab to input controls, here's what we get. Determine which ones we
    // should get.
    QSettings appleSettings(QLatin1String("apple.com"));
    QVariant appleValue = appleSettings.value(QLatin1String("AppleKeyboardUIMode"), 0);
    tabAllControls = (appleValue.toInt() & 0x2);
#endif

    tab.simulate(now);
    if (!tabAllControls) {
        QVERIFY(spy.count() == 0);
        QVERIFY(now == QApplication::focusWidget());
    } else {
        QVERIFY(spy.count() > 0);
        old = qVariantValue<QWidget*>(spy.at(0).at(0));
        now = qVariantValue<QWidget*>(spy.at(0).at(1));
        QVERIFY(now == &pb2);
        QVERIFY(now == QApplication::focusWidget());
        QVERIFY(old == &le2);
        spy.clear();
    }

    if (!tabAllControls) {
        QVERIFY(spy.count() == 0);
        QVERIFY(now == QApplication::focusWidget());
    } else {
        tab.simulate(now);
        QVERIFY(spy.count() > 0);
        old = qVariantValue<QWidget*>(spy.at(0).at(0));
        now = qVariantValue<QWidget*>(spy.at(0).at(1));
        QVERIFY(now == &le2);
        QVERIFY(now == QApplication::focusWidget());
        QVERIFY(old == &pb2);
        spy.clear();
    }

    if (!tabAllControls) {
        QVERIFY(spy.count() == 0);
        QVERIFY(now == QApplication::focusWidget());
    } else {
        backtab.simulate(now);
        QVERIFY(spy.count() > 0);
        old = qVariantValue<QWidget*>(spy.at(0).at(0));
        now = qVariantValue<QWidget*>(spy.at(0).at(1));
        QVERIFY(now == &pb2);
        QVERIFY(now == QApplication::focusWidget());
        QVERIFY(old == &le2);
        spy.clear();
    }


    if (!tabAllControls) {
        QVERIFY(spy.count() == 0);
        QVERIFY(now == QApplication::focusWidget());
        old = &pb2;
    } else {
        backtab.simulate(now);
        QVERIFY(spy.count() > 0);
        old = qVariantValue<QWidget*>(spy.at(0).at(0));
        now = qVariantValue<QWidget*>(spy.at(0).at(1));
        QVERIFY(now == &le2);
        QVERIFY(now == QApplication::focusWidget());
        QVERIFY(old == &pb2);
        spy.clear();
    }

    click.simulate(old);
    if (!(pb2.focusPolicy() & Qt::ClickFocus)) {
        QVERIFY(spy.count() == 0);
        QVERIFY(now == QApplication::focusWidget());
    } else {
        QVERIFY(spy.count() > 0);
        old = qVariantValue<QWidget*>(spy.at(0).at(0));
        now = qVariantValue<QWidget*>(spy.at(0).at(1));
        QVERIFY(now == &pb2);
        QVERIFY(now == QApplication::focusWidget());
        QVERIFY(old == &le2);
        spy.clear();

        click.simulate(old);
        QVERIFY(spy.count() > 0);
        old = qVariantValue<QWidget*>(spy.at(0).at(0));
        now = qVariantValue<QWidget*>(spy.at(0).at(1));
        QVERIFY(now == &le2);
        QVERIFY(now == QApplication::focusWidget());
        QVERIFY(old == &pb2);
        spy.clear();
    }

    parent1.activateWindow();
    QApplication::setActiveWindow(&parent1); // needs this on twm (focus follows mouse)
    QVERIFY(spy.count() == 1 || spy.count() == 2); // one for deactivation, one for activation on Windows

    //on windows, the change of focus is made in 2 steps
    //(the focusChanged SIGNAL is emitted twice)
    if (spy.count()==1)
        old = qVariantValue<QWidget*>(spy.at(spy.count()-1).at(0));
    else
        old = qVariantValue<QWidget*>(spy.at(spy.count()-2).at(0));
    now = qVariantValue<QWidget*>(spy.at(spy.count()-1).at(1));
    QVERIFY(now == &le1);
    QVERIFY(now == QApplication::focusWidget());
    QVERIFY(old == &le2);
    spy.clear();
}

void tst_QApplication::execAfterExit()
{
    int argc = 1;
    QApplication app(argc, &argv0, QApplication::GuiServer);
    QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
    // this should be ignored, as exec() will reset the exitCode
    QApplication::exit(1);
    int exitCode = app.exec();
    QCOMPARE(exitCode, 0);

    // the quitNow flag should have been reset, so we can spin an
    // eventloop after QApplication::exec() returns
    QEventLoop eventLoop;
    QMetaObject::invokeMethod(&eventLoop, "quit", Qt::QueuedConnection);
    exitCode = eventLoop.exec();
    QCOMPARE(exitCode, 0);
}

void tst_QApplication::wheelScrollLines()
{
    int argc = 1;
    QApplication app(argc, &argv0, QApplication::GuiServer);
    // If wheelScrollLines returns 0, the mose wheel will be disabled.
    QVERIFY(app.wheelScrollLines() > 0);
}

void tst_QApplication::style()
{
    int argc = 1;

    {
        QApplication app(argc, &argv0, QApplication::GuiServer);
        QPointer<QStyle> style = app.style();
        app.setStyle(new QWindowsStyle);
        QVERIFY(style.isNull());
    }

    QApplication app(argc, &argv0, QApplication::GuiServer);

    // qApp style can never be 0
    QVERIFY(QApplication::style() != 0);
}

void tst_QApplication::allWidgets()
{
    int argc = 1;
    QApplication app(argc, &argv0, QApplication::GuiServer);
    QWidget *w = new QWidget;
    QVERIFY(app.allWidgets().contains(w)); // uncreate widget test
    WId wid = w->winId();
    QVERIFY(app.allWidgets().contains(w)); // created widget test
    delete w;
    w = 0;
    QVERIFY(!app.allWidgets().contains(w)); // removal test
}


void tst_QApplication::setAttribute()
{
    int argc = 1;
    QApplication app(argc, &argv0, QApplication::GuiServer);
    QVERIFY(!QApplication::testAttribute(Qt::AA_ImmediateWidgetCreation));
    QWidget  *w = new QWidget;
    QVERIFY(!w->testAttribute(Qt::WA_WState_Created));
    delete w;

    QApplication::setAttribute(Qt::AA_ImmediateWidgetCreation);
    QVERIFY(QApplication::testAttribute(Qt::AA_ImmediateWidgetCreation));
    w = new QWidget;
    QVERIFY(w->testAttribute(Qt::WA_WState_Created));
    QWidget *w2 = new QWidget(w);
    w2->setParent(0);
    QVERIFY(w2->testAttribute(Qt::WA_WState_Created));
    delete w;
    delete w2;

    QApplication::setAttribute(Qt::AA_ImmediateWidgetCreation, false);
    QVERIFY(!QApplication::testAttribute(Qt::AA_ImmediateWidgetCreation));
    w = new QWidget;
    QVERIFY(!w->testAttribute(Qt::WA_WState_Created));
    delete w;
}

void tst_QApplication::windowsCommandLine()
{
#if defined(Q_OS_WIN)
    QProcess testProcess;
    QStringList args("Hello \"World\"");
#if defined(QT_DEBUG)
    testProcess.start("wincmdline/debug/wincmdline", args);
#else
    testProcess.start("wincmdline/release/wincmdline", args);
#endif
    QVERIFY(testProcess.waitForFinished(10000));
    QByteArray error = testProcess.readAllStandardError();
    QString procError(error);
    QCOMPARE(procError, QString("Hello \"World\""));
#endif
}

//QTEST_APPLESS_MAIN(tst_QApplication)
int main(int argc, char *argv[])
{
    tst_QApplication tc;
    argv0 = argv[0];
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_qapplication.moc"

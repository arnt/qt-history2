/****************************************************************************
  **
  ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
  **
  ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
  ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
  **
  ****************************************************************************/

#include <QtTest/QtTest>

#include <QMdiSubWindow>
#include <QMdiArea>

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDesktopWidget>

#if defined(Q_WS_X11)
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif

static const Qt::WindowFlags DefaultWindowFlags
    = Qt::SubWindow | Qt::WindowSystemMenuHint
      | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;

Q_DECLARE_METATYPE(QMdiArea::WindowOrder)
Q_DECLARE_METATYPE(QMdiSubWindow *)

class tst_QMdiArea : public QObject
{
    Q_OBJECT
public:
    tst_QMdiArea();
protected slots:
    void activeChanged(QMdiSubWindow *child);

private slots:
    // Tests from QWorkspace
    void subWindowActivated_data();
    void subWindowActivated();
    void subWindowActivatedWithMinimize();
    void showWindows();
    void changeWindowTitle();
    void changeModified();
    void childSize();
    void fixedSize();
    // New tests
    void minimumSizeHint();
    void sizeHint();
    void setActiveSubWindow();
    void addAndRemoveWindows();
    void addAndRemoveWindowsWithReparenting();
    void closeWindows();
    void activateNextAndPreviousWindow();
    void subWindowList_data();
    void subWindowList();
    void setScrollBarsEnabled();
    void setBackground();
    void setViewport();
    void tileSubWindows();
    void cascadeAndTileSubWindows();
    void resizeMaximizedChildWindows_data();
    void resizeMaximizedChildWindows();
    void focusWidgetAfterAddSubWindow();

private:
    QMdiSubWindow *activeWindow;
    bool accelPressed;
};

tst_QMdiArea::tst_QMdiArea()
    : activeWindow(0)
{
    qRegisterMetaType<QMdiSubWindow *>();
}

// Old QWorkspace tests
void tst_QMdiArea::activeChanged(QMdiSubWindow *child)
{
    activeWindow = child;
}

void tst_QMdiArea::subWindowActivated_data()
{
    // define the test elements we're going to use
    QTest::addColumn<int>("count");

    // create a first testdata instance and fill it with data
    QTest::newRow( "data0" ) << 0;
    QTest::newRow( "data1" ) << 1;
    QTest::newRow( "data2" ) << 2;
}

void tst_QMdiArea::subWindowActivated()
{
    QMainWindow mw(0) ;
    mw.menuBar();
    QMdiArea *workspace = new QMdiArea(&mw);
    workspace->setObjectName(QLatin1String("testWidget"));
    mw.setCentralWidget(workspace);
    QSignalSpy spy(workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)));
    connect( workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(activeChanged(QMdiSubWindow *)));
    mw.show();
    qApp->setActiveWindow(&mw);

    QFETCH( int, count );
    int i;

    for ( i = 0; i < count; ++i ) {
        QWidget *widget = new QWidget(workspace, 0);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        workspace->addSubWindow(widget)->show();
        widget->show();
        qApp->processEvents();
        QVERIFY( activeWindow == workspace->activeSubWindow() );
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }

    QList<QMdiSubWindow *> windows = workspace->subWindowList();
    QCOMPARE( (int)windows.count(), count );

    for ( i = 0; i < count; ++i ) {
        QMdiSubWindow *window = windows.at(i);
        window->showMinimized();
        qApp->processEvents();
        QVERIFY( activeWindow == workspace->activeSubWindow() );
        if ( i == 1 )
            QVERIFY( activeWindow == window );
    }

    for ( i = 0; i < count; ++i ) {
        QMdiSubWindow *window = windows.at(i);
        window->showNormal();
        qApp->processEvents();
        QVERIFY( window == activeWindow );
        QVERIFY( activeWindow == workspace->activeSubWindow() );
    }
    spy.clear();

    while (workspace->activeSubWindow() ) {
        workspace->activeSubWindow()->close();
        qApp->processEvents();
        QVERIFY(activeWindow == workspace->activeSubWindow());
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }

    QVERIFY(activeWindow == 0);
    QVERIFY(workspace->activeSubWindow() == 0);
    QCOMPARE(workspace->subWindowList().count(), 0);

    {
        workspace->hide();
        QWidget *widget = new QWidget(workspace);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        QMdiSubWindow *window = workspace->addSubWindow(widget);
        widget->show();
        QCOMPARE(spy.count(), 0);
        workspace->show();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWindow == window );
        window->close();
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWindow == 0 );
    }

    {
        workspace->hide();
        QWidget *widget = new QWidget(workspace);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        QMdiSubWindow *window = workspace->addSubWindow(widget);
        widget->showMaximized();
        qApp->sendPostedEvents();
        QCOMPARE(spy.count(), 0);
        spy.clear();
        workspace->show();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWindow == window );
        window->close();
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWindow == 0 );
    }

    {
        QWidget *widget = new QWidget(workspace);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        QMdiSubWindow *window = workspace->addSubWindow(widget);
        widget->showMinimized();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWindow == window );
        QVERIFY(workspace->activeSubWindow() == window);
        window->close();
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY(workspace->activeSubWindow() == 0);
        QVERIFY( activeWindow == 0 );
    }
}

void tst_QMdiArea::subWindowActivatedWithMinimize()
{
    QMainWindow mw(0) ;
    mw.menuBar();
    QMdiArea *workspace = new QMdiArea(&mw);
    workspace->setObjectName(QLatin1String("testWidget"));
    mw.setCentralWidget(workspace);
    QSignalSpy spy(workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)));
    connect( workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(activeChanged(QMdiSubWindow *)) );
    mw.show();
    qApp->setActiveWindow(&mw);
    QWidget *widget = new QWidget(workspace);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    QMdiSubWindow *window1 = workspace->addSubWindow(widget);
    QWidget *widget2 = new QWidget(workspace);
    widget2->setAttribute(Qt::WA_DeleteOnClose);
    QMdiSubWindow *window2 = workspace->addSubWindow(widget2);

    widget->showMinimized();
    QVERIFY( activeWindow == window1 );
    widget2->showMinimized();
    QVERIFY( activeWindow == window2 );

    window2->close();
    qApp->processEvents();
    QVERIFY( activeWindow == window1 );

    window1->close();
    qApp->processEvents();
    QVERIFY(workspace->activeSubWindow() == 0);
    QVERIFY( activeWindow == 0 );

    QVERIFY( workspace->subWindowList().count() == 0 );
}

void tst_QMdiArea::showWindows()
{
    QMdiArea *ws = new QMdiArea( 0 );

    QWidget *widget = 0;
    ws->show();

    widget = new QWidget(ws);
    widget->show();
    QVERIFY( widget->isVisible() );

    widget = new QWidget(ws);
    widget->showMaximized();
    QVERIFY( widget->isMaximized() );
    widget->showNormal();
    QVERIFY( !widget->isMaximized() );

    widget = new QWidget(ws);
    widget->showMinimized();
    QVERIFY( widget->isMinimized() );
    widget->showNormal();
    QVERIFY( !widget->isMinimized() );

    ws->hide();

    widget = new QWidget(ws);
    ws->show();
    QVERIFY( widget->isVisible() );

    ws->hide();

    widget = new QWidget(ws);
    widget->showMaximized();
    QVERIFY( widget->isMaximized() );
    ws->show();
    QVERIFY( widget->isVisible() );
    QVERIFY( widget->isMaximized() );
    ws->hide();

    widget = new QWidget(ws);
    widget->showMinimized();
    ws->show();
    QVERIFY( widget->isMinimized() );
    ws->hide();

    delete ws;
}


//#define USE_SHOW

void tst_QMdiArea::changeWindowTitle()
{
    const QString mwc = QString::fromLatin1("MainWindow's Caption");
    const QString mwc2 = QString::fromLatin1("MainWindow's New Caption");
    const QString wc = QString::fromLatin1("Widget's Caption");
    const QString wc2 = QString::fromLatin1("Widget's New Caption");

    QMainWindow *mw = new QMainWindow;
    mw->setWindowTitle( mwc );
    QMdiArea *ws = new QMdiArea( mw );
    mw->setCentralWidget( ws );
    mw->menuBar();
    mw->show();

    QWidget *widget = new QWidget( ws );
    widget->setWindowTitle( wc );
    ws->addSubWindow(widget);

    QCOMPARE( mw->windowTitle(), mwc );


#ifdef USE_SHOW
    widget->showMaximized();
#else
    widget->setWindowState(Qt::WindowMaximized);
#endif
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->windowTitle(), QString::fromLatin1("%1 - [%2]").arg(mwc).arg(wc) );
#endif

#ifdef USE_SHOW
    widget->showNormal();
#else
    widget->setWindowState(Qt::WindowNoState);
#endif
    qApp->processEvents();
    QCOMPARE( mw->windowTitle(), mwc );

#ifdef USE_SHOW
    widget->showMaximized();
#else
    widget->setWindowState(Qt::WindowMaximized);
#endif
    qApp->processEvents();
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->windowTitle(), QString::fromLatin1("%1 - [%2]").arg(mwc).arg(wc) );
    widget->setWindowTitle( wc2 );
    QCOMPARE( mw->windowTitle(), QString::fromLatin1("%1 - [%2]").arg(mwc).arg(wc2) );
    mw->setWindowTitle( mwc2 );
    QCOMPARE( mw->windowTitle(), QString::fromLatin1("%1 - [%2]").arg(mwc2).arg(wc2) );
#endif

    mw->show();
    qApp->setActiveWindow(mw);

#ifdef USE_SHOW
    mw->showFullScreen();
#else
    mw->setWindowState(Qt::WindowFullScreen);
#endif

    qApp->processEvents();
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->windowTitle(), QString::fromLatin1("%1 - [%2]").arg(mwc2).arg(wc2) );
#endif
#ifdef USE_SHOW
    widget->showNormal();
#else
    widget->setWindowState(Qt::WindowNoState);
#endif
    qApp->processEvents();
#if defined(Q_WS_MAC)
    QCOMPARE(mw->windowTitle(), mwc);
#else
    QCOMPARE( mw->windowTitle(), mwc2 );
#endif

#ifdef USE_SHOW
    widget->showMaximized();
#else
    widget->setWindowState(Qt::WindowMaximized);
#endif
    qApp->processEvents();
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->windowTitle(), QString::fromLatin1("%1 - [%2]").arg(mwc2).arg(wc2) );
#endif

#ifdef USE_SHOW
    mw->showNormal();
#else
    mw->setWindowState(Qt::WindowNoState);
#endif
    qApp->processEvents();
#ifdef USE_SHOW
    widget->showNormal();
#else
    widget->setWindowState(Qt::WindowNoState);
#endif

    delete mw;
}

void tst_QMdiArea::changeModified()
{
    const QString mwc = QString::fromLatin1("MainWindow's Caption");
    const QString wc = QString::fromLatin1("Widget's Caption[*]");

    QMainWindow *mw = new QMainWindow(0);
    mw->setWindowTitle( mwc );
    QMdiArea *ws = new QMdiArea( mw );
    mw->setCentralWidget( ws );
    mw->menuBar();
    mw->show();

    QWidget *widget = new QWidget( ws );
    widget->setWindowTitle( wc );
    ws->addSubWindow(widget);

    QCOMPARE( mw->isWindowModified(), false);
    QCOMPARE( widget->isWindowModified(), false);
    widget->setWindowState(Qt::WindowMaximized);
    QCOMPARE( mw->isWindowModified(), false);
    QCOMPARE( widget->isWindowModified(), false);

    widget->setWindowState(Qt::WindowNoState);
    QCOMPARE( mw->isWindowModified(), false);
    QCOMPARE( widget->isWindowModified(), false);

    widget->setWindowModified(true);
    QCOMPARE( mw->isWindowModified(), false);
    QCOMPARE( widget->isWindowModified(), true);
    widget->setWindowState(Qt::WindowMaximized);
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->isWindowModified(), true);
#endif
    QCOMPARE( widget->isWindowModified(), true);

    widget->setWindowState(Qt::WindowNoState);
    QCOMPARE( mw->isWindowModified(), false);
    QCOMPARE( widget->isWindowModified(), true);

    widget->setWindowState(Qt::WindowMaximized);
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->isWindowModified(), true);
#endif
    QCOMPARE( widget->isWindowModified(), true);

    widget->setWindowModified(false);
    QCOMPARE( mw->isWindowModified(), false);
    QCOMPARE( widget->isWindowModified(), false);

    widget->setWindowModified(true);
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->isWindowModified(), true);
#endif
    QCOMPARE( widget->isWindowModified(), true);

    widget->setWindowState(Qt::WindowNoState);
    QCOMPARE( mw->isWindowModified(), false);
    QCOMPARE( widget->isWindowModified(), true);

    delete mw;
}

class MyChild : public QWidget
{
public:
    MyChild(QWidget *parent = 0) : QWidget(parent) {}
    QSize sizeHint() const { return QSize(234, 123); }
};

void tst_QMdiArea::childSize()
{
    QMdiArea ws;

    MyChild *child = new MyChild(&ws);
    child->show();
    QCOMPARE(child->size(), child->sizeHint());
    delete child;

    child = new MyChild(&ws);
    child->setFixedSize(200, 200);
    child->show();
    QCOMPARE(child->size(), child->minimumSize());
    delete child;

    child = new MyChild(&ws);
    child->resize(150, 150);
    child->show();
    QCOMPARE(child->size(), QSize(150,150));
    delete child;
}

void tst_QMdiArea::fixedSize()
{
    QMdiArea *ws = new QMdiArea;
    int i;

    ws->resize(500, 500);
//     ws->show();

    QSize fixed(300, 300);
    for (i = 0; i < 4; ++i) {
        QWidget *child = new QWidget(ws);
        child->setFixedSize(fixed);
        child->show();
    }

    QList<QMdiSubWindow *> windows = ws->subWindowList();
    for (i = 0; i < (int)windows.count(); ++i) {
        QMdiSubWindow *child = windows.at(i);
        QCOMPARE(child->size(), fixed);
    }

    ws->cascadeSubWindows();
    ws->resize(800, 800);
    for (i = 0; i < (int)windows.count(); ++i) {
        QMdiSubWindow *child = windows.at(i);
        QCOMPARE(child->size(), fixed);
    }
    ws->resize(500, 500);

    ws->tileSubWindows();
    ws->resize(800, 800);
    for (i = 0; i < (int)windows.count(); ++i) {
        QMdiSubWindow *child = windows.at(i);
        QCOMPARE(child->size(), fixed);
    }
    ws->resize(500, 500);

    for (i = 0; i < (int)windows.count(); ++i) {
        QMdiSubWindow *child = windows.at(i);
        delete child;
    }

    delete ws;
}

// New tests
void tst_QMdiArea::minimumSizeHint()
{
    QMdiArea workspace;
    workspace.show();
    QSize expectedSize(workspace.style()->pixelMetric(QStyle::PM_MDIMinimizedWidth),
                       workspace.style()->pixelMetric(QStyle::PM_TitleBarHeight));
    qApp->processEvents();
    QAbstractScrollArea dummyScrollArea;
    dummyScrollArea.setFrameStyle(QFrame::NoFrame);
    expectedSize = expectedSize.expandedTo(dummyScrollArea.minimumSizeHint());
    QCOMPARE(workspace.minimumSizeHint(), expectedSize.expandedTo(qApp->globalStrut()));

    QWidget *window = workspace.addSubWindow(new QWidget);
    qApp->processEvents();
    window->show();
    QCOMPARE(workspace.minimumSizeHint(), expectedSize.expandedTo(window->minimumSizeHint()));
}

void tst_QMdiArea::sizeHint()
{
    QMdiArea workspace;
    workspace.show();
    QSize desktopSize = QApplication::desktop()->size();
    QSize expectedSize(desktopSize.width() * 2/3, desktopSize.height() * 2/3);
    QCOMPARE(workspace.sizeHint(), expectedSize.expandedTo(qApp->globalStrut()));

    QWidget *window = workspace.addSubWindow(new QWidget);
    qApp->processEvents();
    window->show();
    QCOMPARE(workspace.sizeHint(), expectedSize.expandedTo(window->sizeHint()));

    QMdiSubWindow *nested = workspace.addSubWindow(new QMdiArea);
    expectedSize = QSize(desktopSize.width() * 2/6, desktopSize.height() * 2/6);
    QCOMPARE(nested->widget()->sizeHint(), expectedSize);
}

void tst_QMdiArea::setActiveSubWindow()
{
    QMdiArea workspace;
    workspace.show();

    QSignalSpy spy(&workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)));
    connect(&workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(activeChanged(QMdiSubWindow *)));
    qApp->setActiveWindow(&workspace);

    // Activate hidden windows
    const int windowCount = 10;
    QMdiSubWindow *windows[windowCount];
    for (int i = 0; i < windowCount; ++i) {
        windows[i] = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget));
        qApp->processEvents();
        QVERIFY(windows[i]->isHidden());
        workspace.setActiveSubWindow(windows[i]);
    }
    QCOMPARE(spy.count(), 0);
    QVERIFY(!activeWindow);
    spy.clear();

    // Activate visible windows
    for (int i = 0; i < windowCount; ++i) {
        windows[i]->show();
        QVERIFY(!windows[i]->isHidden());
        workspace.setActiveSubWindow(windows[i]);
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(activeWindow, windows[i]);
        spy.clear();
    }

    // Deactivate active window
    QCOMPARE(workspace.activeSubWindow(), windows[windowCount - 1]);
    workspace.setActiveSubWindow(0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(!activeWindow);
    QVERIFY(!workspace.activeSubWindow());

    // Activate widget which is not child of any window inside workspace
    QMdiSubWindow fakeWindow;
    QTest::ignoreMessage(QtWarningMsg, "QMdiArea::setActiveSubWindow: window is not inside workspace");
    workspace.setActiveSubWindow(&fakeWindow);

}

class LargeWidget : public QWidget
{
public:
    LargeWidget(QWidget *parent = 0) : QWidget(parent) {}
    QSize sizeHint() const { return QSize(1280, 1024); }
};

void tst_QMdiArea::addAndRemoveWindows()
{
    QMdiArea workspace;
    workspace.resize(800, 600);

    { // addSubWindow with large widget
    QCOMPARE(workspace.subWindowList().count(), 0);
    QWidget *window = workspace.addSubWindow(new LargeWidget);
    QVERIFY(window);
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 1);
    QVERIFY(window->windowFlags() == DefaultWindowFlags);
    QCOMPARE(window->size(), workspace.viewport()->size());
    }

    { // addSubWindow, minimumSize set.
    QMdiSubWindow *window = new QMdiSubWindow;
    window->setMinimumSize(900, 900);
    workspace.addSubWindow(window);
    QVERIFY(window);
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 2);
    QVERIFY(window->windowFlags() == DefaultWindowFlags);
    QCOMPARE(window->size(), window->minimumSize());
    }

    { // addSubWindow, resized
    QMdiSubWindow *window = new QMdiSubWindow;
    window->setWidget(new QWidget);
    window->resize(1500, 1500);
    workspace.addSubWindow(window);
    QVERIFY(window);
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 3);
    QVERIFY(window->windowFlags() == DefaultWindowFlags);
    QCOMPARE(window->size(), QSize(1500, 1500));
    }

    { // addSubWindow with 0 pointer
    QTest::ignoreMessage(QtWarningMsg, "QMdiArea::addSubWindow: null pointer to widget");
    QWidget *window = workspace.addSubWindow(0);
    QVERIFY(!window);
    QCOMPARE(workspace.subWindowList().count(), 3);
    }

    { // addChildWindow
    QMdiSubWindow *window = new QMdiSubWindow;
    workspace.addSubWindow(window);
    qApp->processEvents();
    QVERIFY(window->windowFlags() == DefaultWindowFlags);
    window->setWidget(new QWidget);
    QCOMPARE(workspace.subWindowList().count(), 4);
    QTest::ignoreMessage(QtWarningMsg, "QMdiArea::addSubWindow: window is already added");
    workspace.addSubWindow(window);
    }

    { // addChildWindow with 0 pointer
    QTest::ignoreMessage(QtWarningMsg, "QMdiArea::addSubWindow: null pointer to widget");
    workspace.addSubWindow(0);
    QCOMPARE(workspace.subWindowList().count(), 4);
    }

    // removeSubWindow
    foreach (QWidget *window, workspace.subWindowList())
        workspace.removeSubWindow(window);
    QCOMPARE(workspace.subWindowList().count(), 0);

    // removeSubWindow with 0 pointer
    QTest::ignoreMessage(QtWarningMsg, "QMdiArea::removeSubWindow: null pointer to widget");
    workspace.removeSubWindow(0);

    workspace.addSubWindow(new QPushButton(QLatin1String("Dummy to make workspace non-empty")));
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 1);

    // removeSubWindow with window not inside workspace
    QTest::ignoreMessage(QtWarningMsg,"QMdiArea::removeSubWindow: window is not inside workspace");
    QMdiSubWindow *fakeWindow = new QMdiSubWindow;
    workspace.removeSubWindow(fakeWindow);
    delete fakeWindow;
}

void tst_QMdiArea::addAndRemoveWindowsWithReparenting()
{
    QMdiArea workspace;
    QMdiSubWindow window(&workspace);
    QVERIFY(window.windowFlags() == DefaultWindowFlags);

    // 0 because the window list contains widgets and not actual
    // windows. Silly, but that's the behavior.
    QCOMPARE(workspace.subWindowList().count(), 0);
    window.setWidget(new QWidget);
    qApp->processEvents();

    QCOMPARE(workspace.subWindowList().count(), 1);
    window.setParent(0); // Will also reset window flags
    QCOMPARE(workspace.subWindowList().count(), 0);
    window.setParent(&workspace);
    QCOMPARE(workspace.subWindowList().count(), 1);
    QVERIFY(window.windowFlags() == DefaultWindowFlags);

    QTest::ignoreMessage(QtWarningMsg, "QMdiArea::addSubWindow: window is already added");
    workspace.addSubWindow(&window);
    QCOMPARE(workspace.subWindowList().count(), 1);
}

void tst_QMdiArea::closeWindows()
{
    QMdiArea workspace;
    workspace.show();
    qApp->setActiveWindow(&workspace);

    // Close widget
    QWidget *widget = new QWidget;
    QMdiSubWindow *subWindow = workspace.addSubWindow(widget);
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 1);
    subWindow->close();
    QCOMPARE(workspace.subWindowList().count(), 0);

    // Close window
    QWidget *window = workspace.addSubWindow(new QWidget);
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 1);
    window->close();
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 0);

    const int windowCount = 10;

    // Close active window
    for (int i = 0; i < windowCount; ++i)
        workspace.addSubWindow(new QWidget)->show();
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), windowCount);
    int activeSubWindowCount = 0;
    while (workspace.activeSubWindow()) {
        workspace.activeSubWindow()->close();
        qApp->processEvents();
        ++activeSubWindowCount;
    }
    QCOMPARE(activeSubWindowCount, windowCount);
    QCOMPARE(workspace.subWindowList().count(), 0);

    // Close all windows
    for (int i = 0; i < windowCount; ++i)
        workspace.addSubWindow(new QWidget)->show();
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), windowCount);
    QSignalSpy spy(&workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)));
    connect(&workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(activeChanged(QMdiSubWindow *)));
    workspace.closeAllSubWindows();
    qApp->processEvents();
    QCOMPARE(workspace.subWindowList().count(), 0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(!activeWindow);
}

void tst_QMdiArea::activateNextAndPreviousWindow()
{
    QMdiArea workspace;
    workspace.show();
    qApp->setActiveWindow(&workspace);

    const int windowCount = 10;
    QMdiSubWindow *windows[windowCount];
    for (int i = 0; i < windowCount; ++i) {
        windows[i] = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget));
        windows[i]->show();
        qApp->processEvents();
    }

    QSignalSpy spy(&workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)));
    connect(&workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(activeChanged(QMdiSubWindow *)));

    // activateNextSubWindow
    for (int i = 0; i < windowCount; ++i) {
        workspace.activateNextSubWindow();
        qApp->processEvents();
        QCOMPARE(workspace.activeSubWindow(), windows[i]);
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }
    QVERIFY(activeWindow);
    QCOMPARE(workspace.activeSubWindow(), windows[windowCount - 1]);
    QCOMPARE(workspace.activeSubWindow(), activeWindow);

    // activatePreviousSubWindow
    for (int i = windowCount - 2; i >= 0; --i) {
        workspace.activatePreviousSubWindow();
        qApp->processEvents();
        QCOMPARE(workspace.activeSubWindow(), windows[i]);
        QCOMPARE(spy.count(), 1);
        spy.clear();
        if (i % 2 == 0)
            windows[i]->hide(); // 10, 8, 6, 4, 2, 0
    }
    QVERIFY(activeWindow);
    QCOMPARE(workspace.activeSubWindow(), windows[0]);
    QCOMPARE(workspace.activeSubWindow(), activeWindow);

    // activateNextSubWindow with every 2nd window hidden
    for (int i = 0; i < windowCount / 2; ++i) {
        workspace.activateNextSubWindow(); // 1, 3, 5, 7, 9
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }
    QCOMPARE(workspace.activeSubWindow(), windows[windowCount - 1]);

    // activatePreviousSubWindow with every 2nd window hidden
    for (int i = 0; i < windowCount / 2; ++i) {
        workspace.activatePreviousSubWindow(); // 7, 5, 3, 1, 9
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }
    QCOMPARE(workspace.activeSubWindow(), windows[windowCount - 1]);

    workspace.setActiveSubWindow(0);
    QVERIFY(!activeWindow);
}

void tst_QMdiArea::subWindowList_data()
{
    QTest::addColumn<QMdiArea::WindowOrder>("windowOrder");
    QTest::addColumn<int>("windowCount");
    QTest::addColumn<int>("activeSubWindow");
    QTest::addColumn<int>("staysOnTop1");
    QTest::addColumn<int>("staysOnTop2");

    QTest::newRow("CreationOrder") << QMdiArea::CreationOrder << 10 << 4 << 8 << 5;
    QTest::newRow("StackingOrder") << QMdiArea::StackingOrder << 10 << 6 << 3 << 9;
}
void tst_QMdiArea::subWindowList()
{
    QFETCH(QMdiArea::WindowOrder, windowOrder);
    QFETCH(int, windowCount);
    QFETCH(int, activeSubWindow);
    QFETCH(int, staysOnTop1);
    QFETCH(int, staysOnTop2);

    QMdiArea workspace;
    workspace.show();
    qApp->setActiveWindow(&workspace);

    QVector<QMdiSubWindow *> windows;
    for (int i = 0; i < windowCount; ++i) {
        windows.append(qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget)));
        windows[i]->show();
    }

    {
    QList<QMdiSubWindow *> widgets = workspace.subWindowList(windowOrder);
    QCOMPARE(widgets.count(), windowCount);
    for (int i = 0; i < widgets.count(); ++i)
        QCOMPARE(widgets.at(i), windows[i]);
    }

    windows[staysOnTop1]->setWindowFlags(windows[staysOnTop1]->windowFlags() | Qt::WindowStaysOnTopHint);
    workspace.setActiveSubWindow(windows[activeSubWindow]);
    qApp->processEvents();
    QCOMPARE(workspace.activeSubWindow(), windows[activeSubWindow]);

    {
    QList<QMdiSubWindow *> subWindows = workspace.subWindowList(windowOrder);
    if (windowOrder == QMdiArea::CreationOrder) {
        QCOMPARE(subWindows.at(activeSubWindow), windows[activeSubWindow]);
        QCOMPARE(subWindows.at(staysOnTop1), windows[staysOnTop1]);
        return;
    }
    // StackingOrder
    QCOMPARE(subWindows.at(subWindows.count() - 1), windows[staysOnTop1]);
    QCOMPARE(subWindows.at(subWindows.count() - 2), windows[activeSubWindow]);
    QCOMPARE(subWindows.count(), windowCount);
    }

    windows[staysOnTop2]->setWindowFlags(windows[staysOnTop2]->windowFlags() | Qt::WindowStaysOnTopHint);
    workspace.setActiveSubWindow(windows[staysOnTop2]);
    qApp->processEvents();
    QCOMPARE(workspace.activeSubWindow(), windows[staysOnTop2]);
    workspace.setActiveSubWindow(windows[activeSubWindow]);
    qApp->processEvents();
    QCOMPARE(workspace.activeSubWindow(), windows[activeSubWindow]);

    {
    QList<QMdiSubWindow *> widgets = workspace.subWindowList(windowOrder);
    QCOMPARE(widgets.count(), windowCount);
    QCOMPARE(widgets.at(widgets.count() - 1), windows[staysOnTop2]);
    QCOMPARE(widgets.at(widgets.count() - 2), windows[staysOnTop1]);
    QCOMPARE(widgets.at(widgets.count() - 3), windows[activeSubWindow]);
    }
}

void tst_QMdiArea::setScrollBarsEnabled()
{
    QMdiArea workspace;

    QVERIFY(!workspace.scrollBarsEnabled());
    QCOMPARE(workspace.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    QCOMPARE(workspace.horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);

    workspace.setScrollBarsEnabled(true);
    QVERIFY(workspace.scrollBarsEnabled());
    QCOMPARE(workspace.verticalScrollBarPolicy(), Qt::ScrollBarAsNeeded);
    QCOMPARE(workspace.horizontalScrollBarPolicy(), Qt::ScrollBarAsNeeded);

    workspace.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QVERIFY(!workspace.scrollBarsEnabled());
    workspace.setScrollBarsEnabled(true);
    workspace.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QVERIFY(!workspace.scrollBarsEnabled());
}

void tst_QMdiArea::setBackground()
{
    QMdiArea workspace;
    QCOMPARE(workspace.background(), workspace.palette().brush(QPalette::Dark));
    workspace.setBackground(QBrush(Qt::green));
    QCOMPARE(workspace.background(), QBrush(Qt::green));
}

void tst_QMdiArea::setViewport()
{
    QMdiArea workspace;
    workspace.show();

    QWidget *firstViewport = workspace.viewport();
    QVERIFY(firstViewport);

    const int windowCount = 10;
    for (int i = 0; i < windowCount; ++i) {
        QMdiSubWindow *window = workspace.addSubWindow(new QWidget);
        window->show();
        if (i % 2 == 0) {
            window->showMinimized();
            QVERIFY(window->isMinimized());
        } else {
            window->showMaximized();
            QVERIFY(window->isMaximized());
        }
    }

    qApp->processEvents();
    QList<QMdiSubWindow *> windowsBeforeViewportChange = workspace.subWindowList();
    QCOMPARE(windowsBeforeViewportChange.count(), windowCount);

    workspace.setViewport(new QWidget);
    qApp->processEvents();
    QVERIFY(workspace.viewport() != firstViewport);

    QList<QMdiSubWindow *> windowsAfterViewportChange = workspace.subWindowList();
    QCOMPARE(windowsAfterViewportChange.count(), windowCount);
    QCOMPARE(windowsAfterViewportChange, windowsBeforeViewportChange);

    //    for (int i = 0; i < windowCount; ++i) {
    //        QMdiSubWindow *window = windowsAfterViewportChange.at(i);
    //        if (i % 2 == 0)
    //            QVERIFY(!window->isMinimized());
    //else
    //    QVERIFY(!window->isMaximized());
    //    }

    QTest::ignoreMessage(QtWarningMsg, "QMdiArea: Deleting the view port is undefined, "
                                       "use setViewport instead.");
    delete workspace.viewport();
    qApp->processEvents();

    QCOMPARE(workspace.subWindowList().count(), 0);
    QVERIFY(!workspace.activeSubWindow());
}

void tst_QMdiArea::tileSubWindows()
{
    QMdiArea workspace;
    workspace.show();

    const int windowCount = 10;
    for (int i = 0; i < windowCount; ++i)
        workspace.addSubWindow(new QWidget)->show();
    workspace.tileSubWindows();
    QCOMPARE(workspace.viewport()->childrenRect(), workspace.viewport()->rect());
}

void tst_QMdiArea::cascadeAndTileSubWindows()
{
    QMdiArea workspace;
    workspace.resize(400, 400);
    workspace.show();

    const int windowCount = 10;
    QList<QMdiSubWindow *> windows;
    for (int i = 0; i < windowCount; ++i) {
        QMdiSubWindow *window = workspace.addSubWindow(new MyChild);
        if (i % 3 == 0) {
            window->showMinimized();
            QVERIFY(window->isMinimized());
        } else {
            window->showMaximized();
            QVERIFY(window->isMaximized());
        }
        windows.append(window);
    }

    // cascadeSubWindows
    qApp->processEvents();
    workspace.cascadeSubWindows();
    qApp->processEvents();

    // Check dy between two cascaded windows
    QStyleOptionTitleBar options;
    options.initFrom(windows.at(1));
    int titleBarHeight = windows.at(1)->style()->pixelMetric(QStyle::PM_TitleBarHeight, &options);
    // ### Remove this after the mac style has been fixed
    if (windows.at(1)->style()->inherits("QMacStyle"))
        titleBarHeight -= 4;
    const QFontMetrics fontMetrics = QFontMetrics(QApplication::font("QWorkspaceTitleBar"));
    const int dy = qMax(titleBarHeight - (titleBarHeight - fontMetrics.height()) / 2, 1);
    QCOMPARE(windows.at(2)->geometry().top() - windows.at(1)->geometry().top(), dy);

    for (int i = 0; i < windows.count(); ++i) {
        QMdiSubWindow *window = windows.at(i);
        if (i % 3 == 0) {
            QVERIFY(window->isMinimized());
        } else {
            QVERIFY(!window->isMaximized());
            QCOMPARE(window->size(), window->sizeHint());
            window->showMaximized();
            QVERIFY(window->isMaximized());
        }
    }

    // arrangeSubWindows icons
    qApp->processEvents();
    workspace.arrangeMinimizedSubWindows();
    qApp->processEvents();

    for (int i = 0; i < windows.count(); ++i) {
        QMdiSubWindow *window = windows.at(i);
        if (i % 3 == 0)
            QVERIFY(window->isMinimized());
        else
            QVERIFY(window->isMaximized());
    }
}

void tst_QMdiArea::resizeMaximizedChildWindows_data()
{
    QTest::addColumn<int>("startSize");
    QTest::addColumn<int>("increment");
    QTest::addColumn<int>("windowCount");

    QTest::newRow("multiple children") << 400 << 20 << 10;
}

void tst_QMdiArea::resizeMaximizedChildWindows()
{
    QFETCH(int, startSize);
    QFETCH(int, increment);
    QFETCH(int, windowCount);

    QMdiArea workspace;
    workspace.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&workspace);
#endif
    workspace.resize(startSize, startSize);
    QSize workspaceSize = workspace.size();
    QVERIFY(workspaceSize.isValid());
    QCOMPARE(workspaceSize, QSize(startSize, startSize));

    QList<QMdiSubWindow *> windows;
    for (int i = 0; i < windowCount; ++i) {
        QMdiSubWindow *window = workspace.addSubWindow(new QWidget);
        windows.append(window);
        qApp->processEvents();
        window->showMaximized();
        QVERIFY(window->isMaximized());
        QSize windowSize = window->size();
        QVERIFY(windowSize.isValid());
        QCOMPARE(window->rect(), workspace.contentsRect());

        workspace.resize(workspaceSize + QSize(increment, increment));
        qApp->syncX();
        QCOMPARE(workspace.size(), workspaceSize + QSize(increment, increment));
        QCOMPARE(window->size(), windowSize + QSize(increment, increment));
        workspaceSize = workspace.size();
    }

    int newSize = startSize + increment * windowCount;
    QCOMPARE(workspaceSize, QSize(newSize, newSize));
    foreach (QWidget *window, windows)
        QCOMPARE(window->rect(), workspace.contentsRect());
}

// QWidget::setParent clears focusWidget so make sure
// we restore it after QMdiArea::addSubWindow.
void tst_QMdiArea::focusWidgetAfterAddSubWindow()
{
    QWidget *view = new QWidget;
    view->setLayout(new QVBoxLayout);

    QLineEdit *lineEdit1 = new QLineEdit;
    QLineEdit *lineEdit2 = new QLineEdit;
    view->layout()->addWidget(lineEdit1);
    view->layout()->addWidget(lineEdit2);

    lineEdit2->setFocus();
    QCOMPARE(view->focusWidget(), lineEdit2);

    QMdiArea mdiArea;
    mdiArea.addSubWindow(view);
    QCOMPARE(view->focusWidget(), lineEdit2);

    mdiArea.show();
    view->show();
    qApp->setActiveWindow(&mdiArea);
    QCOMPARE(qApp->focusWidget(), lineEdit2);
}

QTEST_MAIN(tst_QMdiArea)
#include "tst_qmdiarea.moc"


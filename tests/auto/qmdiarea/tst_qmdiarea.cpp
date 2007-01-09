/****************************************************************************
  **
  ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
  **
  ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
  ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
  **
  ****************************************************************************/

#include <QtTest/QtTest>

#include "qmdisubwindow.h"
#include "qmdiarea.h"

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QStyle>
#include <q3popupmenu.h>
#include <q3accel.h>

#if defined(Q_WS_X11)
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif

static const Qt::WindowFlags DefaultWindowFlags
    = Qt::SubWindow | Qt::WindowSystemMenuHint
      | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;

Q_DECLARE_METATYPE(QMdiArea::WindowOrder)

class tst_QMdiArea : public QObject
{
    Q_OBJECT
public:
    tst_QMdiArea();
protected slots:
    void activeChanged(QWidget *child);
    void accelActivated();

private slots:
    // Tests from QWorkspace
    void windowActivated_data();
    void windowActivated();
    void accelPropagation();
    void windowActivatedWithMinimize();
    void showWindows();
    void changeWindowTitle();
    void changeModified();
    void childSize();
    void fixedSize();
    // New tests
    void minimumSizeHint();
    void sizeHint();
    void setActiveWindow();
    void addAndRemoveWindows();
    void addAndRemoveWindowsWithReparenting();
    void closeWindows();
    void activateNextAndPreviousWindow();
    void windowList_data();
    void windowList();
    void setScrollBarsEnabled();
    void setBackground();
    void setViewport();
    void tile();
    void cascadeAndArrangeIcons();
    void resizeMaximizedChildWindows_data();
    void resizeMaximizedChildWindows();

private:
    QWidget *activeWidget;
    bool accelPressed;
};

tst_QMdiArea::tst_QMdiArea()
    : activeWidget(0)
{}

// Old QWorkspace tests
void tst_QMdiArea::activeChanged(QWidget *child)
{
    activeWidget = child;
}

void tst_QMdiArea::windowActivated_data()
{
    // define the test elements we're going to use
    QTest::addColumn<int>("count");

    // create a first testdata instance and fill it with data
    QTest::newRow( "data0" ) << 0;
    QTest::newRow( "data1" ) << 1;
    QTest::newRow( "data2" ) << 2;
}

void tst_QMdiArea::windowActivated()
{
    QMainWindow mw(0, Qt::WX11BypassWM) ;
    mw.menuBar();
    QMdiArea *workspace = new QMdiArea(&mw);
    workspace->setObjectName(QLatin1String("testWidget"));
    mw.setCentralWidget(workspace);
    QSignalSpy spy(workspace, SIGNAL(windowActivated(QWidget *)));
    connect( workspace, SIGNAL(windowActivated(QWidget *)), this, SLOT(activeChanged(QWidget *)));
    mw.show();
    qApp->setActiveWindow(&mw);

    QFETCH( int, count );
    int i;

    for ( i = 0; i < count; ++i ) {
        QWidget *widget = new QWidget(workspace, 0);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        workspace->addWindow(widget)->show();
        widget->show();
        qApp->processEvents();
        QVERIFY( activeWidget == workspace->activeWindow() );
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }

    QWidgetList windows = workspace->windowList();
    QCOMPARE( (int)windows.count(), count );

    for ( i = 0; i < count; ++i ) {
        QWidget *window = windows.at(i);
        window->showMinimized();
        qApp->processEvents();
        QVERIFY( activeWidget == workspace->activeWindow() );
        if ( i == 1 )
            QVERIFY( activeWidget == window );
    }

    for ( i = 0; i < count; ++i ) {
        QWidget *window = windows.at(i);
        window->showNormal();
        qApp->processEvents();
        QVERIFY( window == activeWidget );
        QVERIFY( activeWidget == workspace->activeWindow() );
    }
    spy.clear();

    while (workspace->activeWindow() ) {
        workspace->activeWindow()->close();
        qApp->processEvents();
        QVERIFY(activeWidget == workspace->activeWindow());
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }

    QVERIFY(activeWidget == 0);
    QVERIFY(workspace->activeWindow() == 0);
    QCOMPARE(workspace->windowList().count(), 0);

    {
        workspace->hide();
        QWidget *widget = new QWidget(workspace, "normal");
        widget->setAttribute(Qt::WA_DeleteOnClose);
        workspace->addWindow(widget);
        widget->show();
        QCOMPARE(spy.count(), 0);
        workspace->show();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWidget == widget );
        widget->close();
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWidget == 0 );
    }

    {
        workspace->hide();
        QWidget *widget = new QWidget(workspace, "maximized");
        widget->setAttribute(Qt::WA_DeleteOnClose);
        workspace->addWindow(widget);
        widget->showMaximized();
        qApp->sendPostedEvents();
        QCOMPARE(spy.count(), 0);
        spy.clear();
        workspace->show();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWidget == widget );
        widget->close();
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWidget == 0 );
    }

    {
        QWidget *widget = new QWidget(workspace, "minimized");
        widget->setAttribute(Qt::WA_DeleteOnClose);
        workspace->addWindow(widget);
        widget->showMinimized();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY( activeWidget == widget );
        QVERIFY(workspace->activeWindow() == widget);
        widget->close();
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QVERIFY(workspace->activeWindow() == 0);
        QVERIFY( activeWidget == 0 );
    }
}

void tst_QMdiArea::windowActivatedWithMinimize()
{
    QMainWindow mw(0, Qt::WX11BypassWM) ;
    mw.menuBar();
    QMdiArea *workspace = new QMdiArea(&mw);
    workspace->setObjectName(QLatin1String("testWidget"));
    mw.setCentralWidget(workspace);
    QSignalSpy spy(workspace, SIGNAL(windowActivated(QWidget*)));
    connect( workspace, SIGNAL(windowActivated(QWidget*)), this, SLOT(activeChanged(QWidget*)) );
    mw.show();
    qApp->setActiveWindow(&mw);
    QWidget *widget = new QWidget(workspace, "minimized1");
    widget->setAttribute(Qt::WA_DeleteOnClose);
    workspace->addWindow(widget);
    QWidget *widget2 = new QWidget(workspace, "minimized2");
    widget2->setAttribute(Qt::WA_DeleteOnClose);
    workspace->addWindow(widget2);

    widget->showMinimized();
    QVERIFY( activeWidget == widget );
    widget2->showMinimized();
    QVERIFY( activeWidget == widget2 );

    widget2->close();
    qApp->processEvents();
    QVERIFY( activeWidget == widget );

    widget->close();
    qApp->processEvents();
    QVERIFY(workspace->activeWindow() == 0);
    QVERIFY( activeWidget == 0 );

    QVERIFY( workspace->windowList().count() == 0 );
}

void tst_QMdiArea::accelActivated()
{
    accelPressed = TRUE;
}

void tst_QMdiArea::accelPropagation()
{
    QSKIP( "Until QTest::keyPress() sends the events via the OS, this will skip", SkipAll);
    // See #13987 for details of bug report related to this
    // test.  If you have a better idea for a function name then
    // say so :)

    QMainWindow mw(0, Qt::WX11BypassWM) ;
    mw.menuBar();
    QMdiArea *workspace = new QMdiArea(&mw);
    workspace->setObjectName(QLatin1String("testWidget"));
    mw.setCentralWidget(workspace);
    connect( workspace, SIGNAL(windowActivated(QWidget*)), this, SLOT(activeChanged(QWidget*)) );
    mw.show();
    qApp->setActiveWindow(&mw);

    QMainWindow* mainWindow = new QMainWindow( workspace );

    // The popup menu has to have no parent, this is vital in the
    // original case of reproducing the bug

    Q3PopupMenu* popup = new Q3PopupMenu;
    popup->insertItem(QString::fromLatin1("First"));
    mainWindow->menuBar()->insertItem(QString::fromLatin1("Menu"), popup);

    Q3Accel* accel = new Q3Accel(mainWindow);
    accel->connectItem(accel->insertItem(Qt::Key_Escape), this, SLOT(accelActivated()) );

    mainWindow->show();

    QTest::keyPress( mainWindow, Qt::Key_Escape );
    QVERIFY( accelPressed );
    accelPressed = FALSE;

    QTest::mousePress( mainWindow->menuBar(), Qt::LeftButton, 0, QPoint( 5, 5 ) );

    // Check the popup menu did appear to be sure
    QVERIFY( qApp->activePopupWidget() == popup );

    QTest::mouseClick( popup, Qt::LeftButton, 0, QPoint( 5, 25 ) );

    // Check we did actually cause the popup menu to be closed
    QVERIFY( !popup->isVisible() );

    // Now we check that the accelarator still works
    QTest::keyPress( mainWindow, Qt::Key_Escape );
    QVERIFY( accelPressed );
    delete popup;
}

void tst_QMdiArea::showWindows()
{
    QMdiArea *ws = new QMdiArea( 0 );

    QWidget *widget = 0;
    ws->show();

    widget = new QWidget( ws, "plain1" );
    widget->show();
    QVERIFY( widget->isVisible() );

    widget = new QWidget( ws, "maximized1" );
    widget->showMaximized();
    QVERIFY( widget->isMaximized() );
    widget->showNormal();
    QVERIFY( !widget->isMaximized() );

    widget = new QWidget( ws, "minimized1" );
    widget->showMinimized();
    QVERIFY( widget->isMinimized() );
    widget->showNormal();
    QVERIFY( !widget->isMinimized() );

    ws->hide();

    widget = new QWidget( ws, "plain2" );
    ws->show();
    QVERIFY( widget->isVisible() );

    ws->hide();

    widget = new QWidget( ws, "maximized2" );
    widget->showMaximized();
    QVERIFY( widget->isMaximized() );
    ws->show();
    QVERIFY( widget->isVisible() );
    QVERIFY( widget->isMaximized() );
    ws->hide();

    widget = new QWidget( ws, "minimized2" );
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
    ws->addWindow(widget);

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
#if !defined(Q_WS_MAC)
    QCOMPARE( mw->caption(), QString::fromLatin1("%1 - [%2]").arg(mwc2).arg(wc2) );
#endif
#ifdef USE_SHOW
    widget->showNormal();
#else
    widget->setWindowState(Qt::WindowNoState);
#endif
#if defined(Q_WS_MAC)
    QCOMPARE( mw->caption(), mwc);
#else
    QCOMPARE( mw->caption(), mwc2 );
#endif

    delete mw;
}

void tst_QMdiArea::changeModified()
{
    const QString mwc = QString::fromLatin1("MainWindow's Caption");
    const QString wc = QString::fromLatin1("Widget's Caption[*]");

    QMainWindow *mw = new QMainWindow(0, Qt::WX11BypassWM);
    mw->setWindowTitle( mwc );
    QMdiArea *ws = new QMdiArea( mw );
    mw->setCentralWidget( ws );
    mw->menuBar();
    mw->show();

    QWidget *widget = new QWidget( ws );
    widget->setWindowTitle( wc );
    ws->addWindow(widget);

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
    MyChild(QWidget *parent = 0, const char *name= 0, Qt::WFlags f = 0)
        : QWidget(parent, name, f)
    {
    }

    QSize sizeHint() const
    {
        return QSize(234, 123);
    }
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

    QWidgetList windows = ws->windowList();
    for (i = 0; i < (int)windows.count(); ++i) {
        QWidget *child = windows.at(i);
        QCOMPARE(child->size(), fixed);
        QCOMPARE(child->visibleRect().size(), fixed);
    }

    ws->cascade();
    ws->resize(800, 800);
    for (i = 0; i < (int)windows.count(); ++i) {
        QWidget *child = windows.at(i);
        QCOMPARE(child->size(), fixed);
        QCOMPARE(child->visibleRect().size(), fixed);
    }
    ws->resize(500, 500);

    ws->tile();
    ws->resize(800, 800);
    for (i = 0; i < (int)windows.count(); ++i) {
        QWidget *child = windows.at(i);
        QCOMPARE(child->size(), fixed);
        QCOMPARE(child->visibleRect().size(), fixed);
    }
    ws->resize(500, 500);

    for (i = 0; i < (int)windows.count(); ++i) {
        QWidget *child = windows.at(i);
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
    QAbstractScrollArea dummyScrollArea;
    QSize expectedSize = dummyScrollArea.sizeHint() + workspace.baseSize();
    QCOMPARE(workspace.sizeHint(), expectedSize.expandedTo(qApp->globalStrut()));

    QWidget *window = workspace.addSubWindow(new QWidget);
    qApp->processEvents();
    window->show();
    QCOMPARE(workspace.sizeHint(), expectedSize.expandedTo(window->sizeHint()));
}

void tst_QMdiArea::setActiveWindow()
{
    QMdiArea workspace;
    workspace.show();

    QSignalSpy spy(&workspace, SIGNAL(windowActivated(QWidget *)));
    connect(&workspace, SIGNAL(windowActivated(QWidget *)), this, SLOT(activeChanged(QWidget *)));
    qApp->setActiveWindow(&workspace);

    // Activate hidden windows
    const int windowCount = 10;
    QMdiSubWindow *windows[windowCount];
    for (int i = 0; i < windowCount; ++i) {
        windows[i] = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget));
        qApp->processEvents();
        QVERIFY(windows[i]->isHidden());
        workspace.setActiveWindow(windows[i]->widget());
    }
    QCOMPARE(spy.count(), 0);
    QVERIFY(!activeWidget);
    spy.clear();

    // Activate visible windows
    for (int i = 0; i < windowCount; ++i) {
        windows[i]->show();
        QVERIFY(!windows[i]->isHidden());
        workspace.setActiveWindow(windows[i]->widget());
        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(activeWidget, windows[i]->widget());
        spy.clear();
    }

    // Deactivate active window
    QCOMPARE(workspace.activeWindow(), windows[windowCount - 1]->widget());
    workspace.setActiveWindow(0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(!activeWidget);
    QVERIFY(!workspace.activeWindow());

    // Activate widget which is not child of any window inside workspace
    QWidget fakeWidget;
    QTest::ignoreMessage(QtWarningMsg, "QWorkspace::setActiveWindow: widget is not child "
                                       "of any window inside QWorkspace");
    workspace.setActiveWindow(&fakeWidget);

}

void tst_QMdiArea::addAndRemoveWindows()
{
    QMdiArea workspace;

    { // addWindow
    QCOMPARE(workspace.windowList().count(), 0);
    QWidget *window = workspace.addWindow(new QWidget);
    QVERIFY(window);
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), 1);
    QVERIFY(window->windowFlags() == DefaultWindowFlags);
    }

    { // addSubWindow
    QWidget *window = workspace.addSubWindow(new QWidget);
    QVERIFY(window);
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), 2);
    QVERIFY(window->windowFlags() == DefaultWindowFlags);
    }

    { // addSubWindow with 0 pointer
    QTest::ignoreMessage(QtWarningMsg, "QWorkspace::addSubWindow: null pointer to widget");
    QWidget *window = workspace.addSubWindow(0);
    QVERIFY(!window);
    QCOMPARE(workspace.windowList().count(), 2);
    }

    { // addChildWindow
    QMdiSubWindow *window = new QMdiSubWindow;
    workspace.addChildWindow(window);
    qApp->processEvents();
    // 0 because the window list contains widgets and not actual
    // windows. Silly, but that's the behavior.
    QCOMPARE(workspace.windowList().count(), 2);
    QVERIFY(window->windowFlags() == DefaultWindowFlags);
    window->setWidget(new QWidget);
    QCOMPARE(workspace.windowList().count(), 3);
    QTest::ignoreMessage(QtWarningMsg, "QWorkspace::addChildWindow: window is already added");
    workspace.addChildWindow(window);
    }

    { // addChildWindow with 0 pointer
    QTest::ignoreMessage(QtWarningMsg, "QWorkspace::addChildWindow: null pointer to window");
    workspace.addChildWindow(0);
    QCOMPARE(workspace.windowList().count(), 3);
    }

    // removeSubWindow
    foreach (QWidget *window, workspace.windowList())
        workspace.removeSubWindow(window);
    QCOMPARE(workspace.windowList().count(), 0);

    // removeSubWindow with 0 pointer
    QTest::ignoreMessage(QtWarningMsg, "QWorkspace::removeSubWindow: null pointer to widget");
    workspace.removeSubWindow(0);

    // removeSubWindow with widget not inside workspace
    QTest::ignoreMessage(QtWarningMsg,"QWorkspace::removeSubWindow: widget is not child of any window inside QWorkspace");
    QWidget fakeWidget;
    workspace.removeSubWindow(&fakeWidget);
}

void tst_QMdiArea::addAndRemoveWindowsWithReparenting()
{
    QMdiArea workspace;
    QMdiSubWindow window(&workspace);
    QVERIFY(window.windowFlags() == DefaultWindowFlags);

    // 0 because the window list contains widgets and not actual
    // windows. Silly, but that's the behavior.
    QCOMPARE(workspace.windowList().count(), 0);
    window.setWidget(new QWidget);
    qApp->processEvents();

    QCOMPARE(workspace.windowList().count(), 1);
    window.setParent(0); // Will also reset window flags
    QCOMPARE(workspace.windowList().count(), 0);
    window.setParent(&workspace);
    QCOMPARE(workspace.windowList().count(), 1);
    QVERIFY(window.windowFlags() == DefaultWindowFlags);

    QTest::ignoreMessage(QtWarningMsg, "QWorkspace::addChildWindow: window is already added");
    workspace.addChildWindow(&window);
    QCOMPARE(workspace.windowList().count(), 1);
}

void tst_QMdiArea::closeWindows()
{
    QMdiArea workspace;
    workspace.show();
    qApp->setActiveWindow(&workspace);

    // Close widget
    QWidget *widget = new QWidget;
    workspace.addSubWindow(widget);
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), 1);
    widget->close();
    QCOMPARE(workspace.windowList().count(), 0);

    // Close window
    QWidget *window = workspace.addSubWindow(new QWidget);
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), 1);
    window->close();
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), 0);

    const int windowCount = 10;

    // Close active window
    for (int i = 0; i < windowCount; ++i)
        workspace.addSubWindow(new QWidget)->show();
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), windowCount);
    int activeWindowCount = 0;
    while (workspace.activeWindow()) {
        workspace.activeWindow()->close();
        qApp->processEvents();
        ++activeWindowCount;
    }
    QCOMPARE(activeWindowCount, windowCount);
    QCOMPARE(workspace.windowList().count(), 0);

    // Close all windows
    for (int i = 0; i < windowCount; ++i)
        workspace.addSubWindow(new QWidget)->show();
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), windowCount);
    QSignalSpy spy(&workspace, SIGNAL(windowActivated(QWidget *)));
    connect(&workspace, SIGNAL(windowActivated(QWidget *)), this, SLOT(activeChanged(QWidget *)));
    workspace.closeAllWindows();
    qApp->processEvents();
    QCOMPARE(workspace.windowList().count(), 0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(!activeWidget);
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

    QSignalSpy spy(&workspace, SIGNAL(windowActivated(QWidget *)));
    connect(&workspace, SIGNAL(windowActivated(QWidget *)), this, SLOT(activeChanged(QWidget *)));

    // activateNextWindow
    for (int i = 0; i < windowCount; ++i) {
        workspace.activateNextWindow();
        qApp->processEvents();
        QCOMPARE(workspace.activeWindow(), windows[i]->widget());
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }
    QVERIFY(activeWidget);
    QCOMPARE(workspace.activeWindow(), windows[windowCount - 1]->widget());
    QCOMPARE(workspace.activeWindow(), activeWidget);

    // activatePreviousWindow
    for (int i = windowCount - 2; i >= 0; --i) {
        workspace.activatePreviousWindow();
        qApp->processEvents();
        QCOMPARE(workspace.activeWindow(), windows[i]->widget());
        QCOMPARE(spy.count(), 1);
        spy.clear();
        if (i % 2 == 0)
            windows[i]->hide(); // 10, 8, 6, 4, 2, 0
    }
    QVERIFY(activeWidget);
    QCOMPARE(workspace.activeWindow(), windows[0]->widget());
    QCOMPARE(workspace.activeWindow(), activeWidget);

    // activateNextWindow with every 2nd window hidden
    for (int i = 0; i < windowCount / 2; ++i) {
        workspace.activateNextWindow(); // 1, 3, 5, 7, 9
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }
    QCOMPARE(workspace.activeWindow(), windows[windowCount - 1]->widget());

    // activatePreviousWindow with every 2nd window hidden
    for (int i = 0; i < windowCount / 2; ++i) {
        workspace.activatePreviousWindow(); // 7, 5, 3, 1, 9
        QCOMPARE(spy.count(), 1);
        spy.clear();
    }
    QCOMPARE(workspace.activeWindow(), windows[windowCount - 1]->widget());

    workspace.setActiveWindow(0);
    QVERIFY(!activeWidget);
}

void tst_QMdiArea::windowList_data()
{
    QTest::addColumn<QMdiArea::WindowOrder>("windowOrder");
    QTest::addColumn<int>("windowCount");
    QTest::addColumn<int>("activeWindow");
    QTest::addColumn<int>("staysOnTop1");
    QTest::addColumn<int>("staysOnTop2");

    QTest::newRow("CreationOrder") << QMdiArea::CreationOrder << 10 << 4 << 8 << 5;
    QTest::newRow("StackingOrder") << QMdiArea::StackingOrder << 10 << 6 << 3 << 9;
}
void tst_QMdiArea::windowList()
{
    QFETCH(QMdiArea::WindowOrder, windowOrder);
    QFETCH(int, windowCount);
    QFETCH(int, activeWindow);
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
    QWidgetList widgets = workspace.windowList(windowOrder);
    QCOMPARE(widgets.count(), windowCount);
    for (int i = 0; i < widgets.count(); ++i)
        QCOMPARE(widgets.at(i), windows[i]->widget());
    }

    windows[staysOnTop1]->setWindowFlags(windows[staysOnTop1]->windowFlags() | Qt::WindowStaysOnTopHint);
    workspace.setActiveWindow(windows[activeWindow]->widget());
    qApp->processEvents();
    QCOMPARE(workspace.activeWindow(), windows[activeWindow]->widget());

    {
    QWidgetList widgets = workspace.windowList(windowOrder);
    if (windowOrder == QMdiArea::CreationOrder) {
        QCOMPARE(widgets.at(activeWindow), windows[activeWindow]->widget());
        QCOMPARE(widgets.at(staysOnTop1), windows[staysOnTop1]->widget());
        return;
    }
    // StackingOrder
    QCOMPARE(widgets.at(widgets.count() - 1), windows[staysOnTop1]->widget());
    QCOMPARE(widgets.at(widgets.count() - 2), windows[activeWindow]->widget());
    QCOMPARE(widgets.count(), windowCount);
    }

    windows[staysOnTop2]->setWindowFlags(windows[staysOnTop2]->windowFlags() | Qt::WindowStaysOnTopHint);
    workspace.setActiveWindow(windows[staysOnTop2]->widget());
    qApp->processEvents();
    QCOMPARE(workspace.activeWindow(), windows[staysOnTop2]->widget());
    workspace.setActiveWindow(windows[activeWindow]->widget());
    qApp->processEvents();
    QCOMPARE(workspace.activeWindow(), windows[activeWindow]->widget());

    {
    QWidgetList widgets = workspace.windowList(windowOrder);
    QCOMPARE(widgets.count(), windowCount);
    QCOMPARE(widgets.at(widgets.count() - 1), windows[staysOnTop2]->widget());
    QCOMPARE(widgets.at(widgets.count() - 2), windows[staysOnTop1]->widget());
    QCOMPARE(widgets.at(widgets.count() - 3), windows[activeWindow]->widget());
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
    QCOMPARE(workspace.background(), QBrush(Qt::darkGray, Qt::SolidPattern));
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
        QWidget *window = workspace.addSubWindow(new QWidget);
        window->show();
        if (i % 2 == 0)
            window->showMinimized();
        else
            window->showMaximized();
    }

    qApp->processEvents();
    QWidgetList windowsBeforeViewportChange = workspace.windowList();
    QCOMPARE(windowsBeforeViewportChange.count(), windowCount);

    workspace.setViewport(new QWidget);
    qApp->processEvents();
    QVERIFY(workspace.viewport() != firstViewport);

    QWidgetList windowsAfterViewportChange = workspace.windowList();
    QCOMPARE(windowsAfterViewportChange.count(), windowCount);
    QCOMPARE(windowsAfterViewportChange, windowsBeforeViewportChange);

    for (int i = 0; i < windowCount; ++i) {
        QWidget *window = windowsAfterViewportChange.at(i);
        if (i % 2 == 0)
            QVERIFY(window->isMinimized());
        else
            QVERIFY(window->isMaximized());
    }

    QTest::ignoreMessage(QtWarningMsg, "QWorkspace: Deleting the view port is undefined, "
                                       "use setViewport instead.");
    delete workspace.viewport();
    qApp->processEvents();

    QCOMPARE(workspace.windowList().count(), 0);
    QVERIFY(!workspace.activeWindow());
}

void tst_QMdiArea::tile()
{
    QMdiArea workspace;
    workspace.show();

    const int windowCount = 10;
    for (int i = 0; i < windowCount; ++i)
        workspace.addSubWindow(new QWidget)->show();
    workspace.tile();
    QCOMPARE(workspace.viewport()->childrenRect(), workspace.viewport()->rect());
}

void tst_QMdiArea::cascadeAndArrangeIcons()
{
    QMdiArea workspace;
    workspace.show();

    const int windowCount = 10;
    QWidgetList windows;
    for (int i = 0; i < windowCount; ++i) {
        QWidget *window = workspace.addSubWindow(new QWidget);
        if (i % 3 == 0) {
            window->showMinimized();
            QVERIFY(window->isMinimized());
        } else {
            window->showMaximized();
            QVERIFY(window->isMaximized());
        }
        windows.append(window);
    }

    // Cascade
    qApp->processEvents();
    workspace.cascade();
    qApp->processEvents();

    for (int i = 0; i < windows.count(); ++i) {
        QWidget *window = windows.at(i);
        if (i % 3 == 0) {
            QVERIFY(window->isMinimized());
        } else {
            QVERIFY(!window->isMaximized());
            window->showMaximized();
            QVERIFY(window->isMaximized());
        }
    }

    // Arrange icons
    qApp->processEvents();
    workspace.arrangeIcons();
    qApp->processEvents();

    for (int i = 0; i < windows.count(); ++i) {
        QWidget *window = windows.at(i);
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

    QWidgetList windows;
    for (int i = 0; i < windowCount; ++i) {
        QWidget *window = workspace.addSubWindow(new QWidget);
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

QTEST_MAIN(tst_QMdiArea)
#include "tst_qmdiarea.moc"


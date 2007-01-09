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

#include <QLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QByteArray>
#include <QStyle>
#include <QStyleOptionTitleBar>

#if defined(Q_WS_X11)
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif

static inline void triggerSignal(QMdiSubWindow *window, QMdiArea *workspace,
                                 const QByteArray &signal)
{
    if (signal == SIGNAL(windowMaximized())) {
        window->showMaximized();
        qApp->processEvents();
        if (window->parent())
            QVERIFY(window->isMaximized());
    } else if (signal == SIGNAL(windowMinimized())) {
        window->showMinimized();
        qApp->processEvents();
        if (window->parent())
            QVERIFY(window->isMinimized());
    } else if (signal == SIGNAL(windowRestored())) {
        window->showMaximized();
        qApp->processEvents();
        window->showNormal();
        qApp->processEvents();
        QVERIFY(!window->isMinimized());
        QVERIFY(!window->isMaximized());
        QVERIFY(!window->isShaded());
    } else if (signal == SIGNAL(aboutToBecomeActive())) {
        if (window->parent()) {
            workspace->setActiveWindow(window->widget());
            qApp->processEvents();
        }
    } else if (signal == SIGNAL(windowActivated())) {
        if (window->parent()) {
            workspace->setActiveWindow(window->widget());
            qApp->processEvents();
        }
    } else if (signal == SIGNAL(windowDeactivated())) {
        if (!window->parent())
            return;
        workspace->setActiveWindow(window->widget());
        qApp->processEvents();
        workspace->setActiveWindow(0);
        qApp->processEvents();
    }
}

// --- from tst_qgraphicsview.cpp ---
static void sendMousePress(QWidget *widget, const QPoint &point, Qt::MouseButton button = Qt::LeftButton)
{
    QMouseEvent event(QEvent::MouseButtonPress, point, widget->mapToGlobal(point), button, 0, 0);
    QApplication::sendEvent(widget, &event);
}

static void sendMouseMove(QWidget *widget, const QPoint &point, Qt::MouseButton button = Qt::LeftButton)
{
    QMouseEvent event(QEvent::MouseMove, point, widget->mapToGlobal(point), button, button, 0);
    QApplication::sendEvent(widget, &event);
}

static void sendMouseRelease(QWidget *widget, const QPoint &point, Qt::MouseButton button = Qt::LeftButton)
{
    QMouseEvent event(QEvent::MouseButtonRelease, point, widget->mapToGlobal(point), button, 0, 0);
    QApplication::sendEvent(widget, &event);
}
// ---

static void sendMouseDoubleClick(QWidget *widget, const QPoint &point, Qt::MouseButton button = Qt::LeftButton)
{
    sendMousePress(widget, point, button);
    sendMouseRelease(widget, point, button);
    QMouseEvent event(QEvent::MouseButtonDblClick, point, widget->mapToGlobal(point), button, 0, 0);
    QApplication::sendEvent(widget, &event);
}

static const Qt::WindowFlags StandardWindowFlags
    = Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint;
static const Qt::WindowFlags DialogWindowFlags
    = Qt::WindowTitleHint | Qt::WindowSystemMenuHint;

Q_DECLARE_METATYPE(Qt::WindowState);
Q_DECLARE_METATYPE(Qt::WindowStates);
Q_DECLARE_METATYPE(Qt::WindowType);
Q_DECLARE_METATYPE(Qt::WindowFlags);

class tst_QMdiSubWindow : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void sizeHint();
    void minimumSizeHint();
    void setWidget();
    void setWindowState_data();
    void setWindowState();
    void mainWindowSupport();
    void emittingOfSignals_data();
    void emittingOfSignals();
    void showShaded();
    void iconSize();
    void showNormal_data();
    void showNormal();
    void setOpaqueResizeAndMove_data();
    void setOpaqueResizeAndMove();
    void setWindowFlags_data();
    void setWindowFlags();
    void mouseDoubleClick();
};

void tst_QMdiSubWindow::initTestCase()
{
    qRegisterMetaType<Qt::WindowStates>("Qt::WindowStates");
}

void tst_QMdiSubWindow::sizeHint()
{
    QMdiSubWindow *window = new QMdiSubWindow;
    QCOMPARE(window->sizeHint(), window->minimumSizeHint());
    window->show();
    QCOMPARE(window->sizeHint(), window->minimumSizeHint());
    QMdiArea workspace;
    workspace.addChildWindow(window);
    QCOMPARE(window->sizeHint(), window->minimumSizeHint());
}

void tst_QMdiSubWindow::minimumSizeHint()
{
    QMdiSubWindow window;
    window.show();

    QCOMPARE(window.minimumSizeHint(), qApp->globalStrut());

    window.setWidget(new QWidget);
    QCOMPARE(window.minimumSizeHint(), window.layout()->minimumSize()
                                       .expandedTo(qApp->globalStrut()));

    delete window.widget();
    delete window.layout();
    window.setWidget(new QWidget);
    QCOMPARE(window.minimumSizeHint(), qApp->globalStrut());

    window.widget()->show();
    QCOMPARE(window.minimumSizeHint(), window.widget()->minimumSizeHint()
                                       .expandedTo(qApp->globalStrut()));
}

void tst_QMdiSubWindow::setWidget()
{
    QMdiSubWindow window;
    window.show();
    QVERIFY(window.layout());
    QVERIFY(!window.widget());

    // QPointer so we can check if the widget is deleted
    QPointer<QWidget> widget = new QWidget;
    widget->setWindowTitle(QString::fromLatin1("DummyTitle"));
    QCOMPARE(widget->windowTitle(), QString::fromLatin1("DummyTitle"));
    window.setWidget(widget);
    QCOMPARE(window.windowTitle(), window.widget()->windowTitle());
    QCOMPARE(widget->parent(), &window);
    QVERIFY(!widget->isVisible());
    QCOMPARE(window.layout()->count(), 1);

    QTest::ignoreMessage(QtWarningMsg,"QMdiSubWindow::setWidget: widget is already set");
    window.setWidget(widget);
    QCOMPARE(window.widget(), static_cast<QWidget *>(widget));
    QCOMPARE(widget->parent(), &window);

    window.setWidget(0);
    QVERIFY(widget);
    QVERIFY(!widget->parent());
    QVERIFY(!window.widget());
    QCOMPARE(window.layout()->count(), 0);

    window.setWidget(widget);
    delete window.layout();
    QVERIFY(!window.layout());
    QVERIFY(window.widget());
    QCOMPARE(window.widget()->parent(), &window);

    delete window.widget();
    QVERIFY(!widget);
    QVERIFY(!window.widget());
}

void tst_QMdiSubWindow::setWindowState_data()
{
    QTest::addColumn<Qt::WindowState>("windowState");

    QTest::newRow("maximized") << Qt::WindowMaximized;
    QTest::newRow("minimized") << Qt::WindowMinimized;
    QTest::newRow("normalized") << Qt::WindowNoState;
}

void tst_QMdiSubWindow::setWindowState()
{
    QFETCH(Qt::WindowState, windowState);
    QMdiArea workspace;
    QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QLineEdit));
    window->show();
    workspace.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&workspace);
#endif

    QWidget *testWidget = 0;
    for (int iteration = 0; iteration < 2; ++iteration) {
        if (iteration == 0)
            testWidget = window;
        else
            testWidget = window->widget();

        testWidget->setWindowState(windowState);

        Qt::WindowStates windowStateWindow = window->windowState();
        windowStateWindow &= ~Qt::WindowActive;
        Qt::WindowStates windowStateWidget = window->widget()->windowState();
        windowStateWidget &= ~Qt::WindowActive;
        QCOMPARE(windowStateWindow, windowStateWidget);

        switch (windowState) {
        case Qt::WindowNoState:
            QVERIFY(!window->widget()->isMinimized());
            QVERIFY(!window->widget()->isMaximized());
            QVERIFY(!window->isMinimized());
            QVERIFY(!window->isMaximized());
            break;
        case Qt::WindowMinimized:
            QVERIFY(window->widget()->isMinimized());
            QVERIFY(window->isMinimized());
            break;
        case Qt::WindowMaximized:
            QVERIFY(window->widget()->isMaximized());
            QVERIFY(window->isMaximized());
            break;
        default:
            break;
        }
    }
}

void tst_QMdiSubWindow::mainWindowSupport()
{
    QList<QMdiSubWindow *> windows;
    QMdiArea *workspace = new QMdiArea;
    QMainWindow mainWindow;
    QString originalWindowTitle = QString::fromLatin1("MainWindow");
    mainWindow.setWindowTitle(originalWindowTitle);
    mainWindow.setCentralWidget(workspace);
    mainWindow.show();
    mainWindow.menuBar();
    qApp->setActiveWindow(&mainWindow);

    for (int i = 0; i < 5; ++i) {
        mainWindow.menuBar()->setVisible(false);

        QMdiSubWindow *window = new QMdiSubWindow;
        windows.append(window);
        QVERIFY(!window->maximizedButtonsWidget());
        QVERIFY(!window->maximizedSystemMenuIconWidget());

        QMdiArea *nestedWorkspace = new QMdiArea; // :-)
        window->setWidget(nestedWorkspace);
        window->widget()->setWindowTitle(QString::fromLatin1("Window %1").arg(i));

        workspace->addChildWindow(window);
        QVERIFY(!window->maximizedButtonsWidget());
        QVERIFY(!window->maximizedSystemMenuIconWidget());
        window->show();

        // mainWindow.menuBar() is not visible
        window->showMaximized();
        qApp->processEvents();
        QVERIFY(window->isMaximized());
        QVERIFY(!window->maximizedButtonsWidget());
        QVERIFY(!window->maximizedSystemMenuIconWidget());
        window->showNormal();

        // Now it is
        mainWindow.menuBar()->setVisible(true);

        window->showMaximized();
        qApp->processEvents();
        QVERIFY(window->isMaximized());
#if !defined(Q_WS_MAC)
        QVERIFY(window->maximizedButtonsWidget());
        QCOMPARE(window->maximizedButtonsWidget(), mainWindow.menuBar()->cornerWidget(Qt::TopRightCorner));
        QVERIFY(window->maximizedSystemMenuIconWidget());
        QCOMPARE(window->maximizedSystemMenuIconWidget(), qobject_cast<QWidget *>(mainWindow.menuBar()
                                                                    ->cornerWidget(Qt::TopLeftCorner)));
        QCOMPARE(mainWindow.windowTitle(), QString::fromLatin1("%1 - [%2]")
                                           .arg(originalWindowTitle, window->widget()->windowTitle()));
#endif

        // Check that nested child windows don't set window title
        nestedWorkspace->show();
        QMdiSubWindow *nestedWindow = new QMdiSubWindow;
        nestedWindow->setWidget(new QWidget);
        nestedWorkspace->addChildWindow(nestedWindow);
        nestedWindow->widget()->setWindowTitle(QString::fromLatin1("NestedWindow %1").arg(i));
        nestedWindow->showMaximized();
        qApp->processEvents();
        QVERIFY(nestedWindow->isMaximized());
        QVERIFY(!nestedWindow->maximizedButtonsWidget());
        QVERIFY(!nestedWindow->maximizedSystemMenuIconWidget());

#if !defined(Q_WS_MAC)
        QCOMPARE(mainWindow.windowTitle(), QString::fromLatin1("%1 - [%2]")
                                           .arg(originalWindowTitle, window->widget()->windowTitle()));
#endif
    }

#if defined(Q_WS_MAC)
    return;
#endif

    workspace->activateNextWindow();
    qApp->processEvents();
    foreach (QMdiSubWindow *window, windows) {
        QCOMPARE(workspace->activeWindow(), window->widget());
        QVERIFY(window->isMaximized());
        QVERIFY(window->maximizedButtonsWidget());
        QCOMPARE(window->maximizedButtonsWidget(), mainWindow.menuBar()->cornerWidget(Qt::TopRightCorner));
        QVERIFY(window->maximizedSystemMenuIconWidget());
        QCOMPARE(window->maximizedSystemMenuIconWidget(), qobject_cast<QWidget *>(mainWindow.menuBar()
                                                                   ->cornerWidget(Qt::TopLeftCorner)));
        QCOMPARE(mainWindow.windowTitle(), QString::fromLatin1("%1 - [%2]")
                                           .arg(originalWindowTitle, window->widget()->windowTitle()));
        workspace->activateNextWindow();
        qApp->processEvents();
    }
}

// This test was written when QMdiSubWindow emitted separate signals
void tst_QMdiSubWindow::emittingOfSignals_data()
{
    QTest::addColumn<QByteArray>("signal");
    QTest::addColumn<Qt::WindowState>("watchedState");

    QTest::newRow("windowMaximized") << QByteArray(SIGNAL(windowMaximized())) << Qt::WindowMaximized;
    QTest::newRow("windowMinimized") << QByteArray(SIGNAL(windowMinimized())) << Qt::WindowMinimized;
    QTest::newRow("windowRestored") << QByteArray(SIGNAL(windowRestored())) << Qt::WindowNoState;
    QTest::newRow("aboutToBecomeActive") << QByteArray(SIGNAL(aboutToBecomeActive())) << Qt::WindowNoState;
    QTest::newRow("windowActivated") << QByteArray(SIGNAL(windowActivated())) << Qt::WindowActive;
    QTest::newRow("windowDeactivated") << QByteArray(SIGNAL(windowDeactivated())) << Qt::WindowActive;
}

void tst_QMdiSubWindow::emittingOfSignals()
{
    QFETCH(QByteArray, signal);
    QFETCH(Qt::WindowState, watchedState);
    QMdiArea workspace;
    workspace.show();
    qApp->processEvents();
    qApp->setActiveWindow(&workspace);
    QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget));
    qApp->processEvents();
    window->show();
    workspace.setActiveWindow(0);
    qApp->processEvents();

    QSignalSpy spy(window, signal == SIGNAL(aboutToBecomeActive())
                           ? signal.data()
                           : SIGNAL(windowStateChanged(Qt::WindowStates, Qt::WindowStates)));
    QVERIFY(spy.isEmpty());
    triggerSignal(window, &workspace, signal);
    // Unless the signal is windowRestored or windowDeactivated,
    // we're already in correct state and nothing should happen.
    if (signal != SIGNAL(windowRestored()) && signal != SIGNAL(windowDeactivated()))
        triggerSignal(window, &workspace, signal);

    int count = 0;
    if (signal == SIGNAL(aboutToBecomeActive())) {
        count += spy.count();
    } else {
        for (int i = 0; i < spy.count(); ++i) {
            Qt::WindowStates oldState = qvariant_cast<Qt::WindowStates>(spy.at(i).at(0));
            Qt::WindowStates newState = qvariant_cast<Qt::WindowStates>(spy.at(i).at(1));
            if (watchedState != Qt::WindowNoState) {
                if (!(oldState & watchedState) && (newState & watchedState))
                    ++count;
            } else {
                if ((oldState & (Qt::WindowMinimized | Qt::WindowMaximized))
                        && (newState & (watchedState | Qt::WindowActive))) {
                    ++count;
                }
            }
        }
    }
    QCOMPARE(count, 1);

    window->setParent(0);
    window->showNormal();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(window);
#endif
    qApp->processEvents();

    spy.clear();
    triggerSignal(window, &workspace, signal);
    QCOMPARE(spy.count(), 0);
    window->close();
}

void tst_QMdiSubWindow::showShaded()
{
    QMdiArea workspace;
    QMdiSubWindow *window = new QMdiSubWindow;
    workspace.addChildWindow(window);
    qApp->processEvents();
    workspace.show();
    window->show();

    QVERIFY(!window->isShaded());
    QVERIFY(!window->isMaximized());

    window->showShaded();
    QVERIFY(window->isShaded());
    QVERIFY(window->isMinimized());

    window->setParent(0);
    window->show();
    QVERIFY(!window->isShaded());

    delete window;
}

void tst_QMdiSubWindow::iconSize()
{
    QMdiSubWindow *window = new QMdiSubWindow;
    QCOMPARE(window->iconSize(), QSize(-1, -1));

    QMdiArea workspace;
    workspace.addChildWindow(window);
    qApp->processEvents();
    QStyleOptionTitleBar options;
    options.initFrom(window);
    int width = window->style()->pixelMetric(QStyle::PM_MDIMinimizedWidth);
    int height = window->style()->pixelMetric(QStyle::PM_TitleBarHeight, &options);
#if defined(Q_WS_MAC)
    // ### Remove this after Mac style has been fixed
    height -= 4;
#endif
    QCOMPARE(window->iconSize(), QSize(width, height));

    window->setWindowFlags(window->windowFlags() | Qt::FramelessWindowHint);
    QCOMPARE(window->iconSize(), QSize(-1, -1));
}

void tst_QMdiSubWindow::showNormal_data()
{
    QTest::addColumn<QByteArray>("slot");

    QTest::newRow("showMinimized") << QByteArray("showMinimized");
    QTest::newRow("showMaximized") << QByteArray("showMaximized");
    QTest::newRow("showShaded") << QByteArray("showShaded");
}

void tst_QMdiSubWindow::showNormal()
{
    QFETCH(QByteArray, slot);

    QMdiArea workspace;
    QWidget *window = workspace.addSubWindow(new QWidget);
    qApp->processEvents();
    workspace.show();
    window->show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&workspace);
#endif

    QRect originalGeometry = window->geometry();
    QVERIFY(QMetaObject::invokeMethod(window, slot.data()));
    qApp->processEvents();
    window->showNormal();
    qApp->processEvents();
    QCOMPARE(window->geometry(), originalGeometry);
}

class EventSpy : public QObject
{
public:
    EventSpy(QObject *object, QEvent::Type event)
        : eventToSpy(event), _count(0)
    {
        if (object)
            object->installEventFilter(this);
    }

    int count() const { return _count; }
    void clear() { _count = 0; }

protected:
    bool eventFilter(QObject *object, QEvent *event)
    {
        if (event->type() == eventToSpy)
            ++_count;
        return  QObject::eventFilter(object, event);
    }

private:
    QEvent::Type eventToSpy;
    int _count;
};

void tst_QMdiSubWindow::setOpaqueResizeAndMove_data()
{
    QTest::addColumn<bool>("opaqueMode");
    QTest::addColumn<int>("geometryCount");
    QTest::addColumn<int>("expectedGeometryCount");
    QTest::addColumn<QSize>("workspaceSize");
    QTest::addColumn<QSize>("windowSize");

    QTest::newRow("opaque mode") << true<< 20 << 20 << QSize(400, 400) << QSize(200, 200);
    QTest::newRow("normal mode") << false << 20 << 1 << QSize(400, 400) << QSize(200, 200);
}

void tst_QMdiSubWindow::setOpaqueResizeAndMove()
{
    QFETCH(bool, opaqueMode);
    QFETCH(int, geometryCount);
    QFETCH(int, expectedGeometryCount);
    QFETCH(QSize, workspaceSize);
    QFETCH(QSize, windowSize);

    QMdiArea workspace;
    QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget));
    qApp->processEvents();
    workspace.show();
    workspace.resize(workspaceSize);
    window->show();

    // ----------------------------- resize -----------------------------
    {
    // setOpaqueResize
    window->setOption(QMdiSubWindow::TransparentResize, !opaqueMode);
    QCOMPARE(window->testOption(QMdiSubWindow::TransparentResize), !opaqueMode);

    // Check that the event spy actually works
    EventSpy resizeSpy(window, QEvent::Resize);
    QCOMPARE(resizeSpy.count(), 0);
    window->resize(windowSize);
    QCOMPARE(window->size(), windowSize);
    QCOMPARE(resizeSpy.count(), 1);
    resizeSpy.clear();
    QCOMPARE(resizeSpy.count(), 0);

    // Enter resize mode.
    int offset = window->style()->pixelMetric(QStyle::PM_MDIFrameWidth) / 2;
    QPoint mousePosition(window->width() - qMax(offset, 2), window->height() - qMax(offset, 2));
    sendMouseMove(window, mousePosition, Qt::NoButton);
    sendMousePress(window, mousePosition);

    // Trigger resize events
    for (int i = 0; i < geometryCount; ++i) {
        ++mousePosition.rx();
        ++mousePosition.ry();
        sendMouseMove(window, mousePosition);
    }

    // Leave resize mode
    sendMouseRelease(window, mousePosition);
    QCOMPARE(resizeSpy.count(), expectedGeometryCount);
    QCOMPARE(window->size(), windowSize + QSize(geometryCount, geometryCount));
    }

    // ------------------------------ move ------------------------------
    {
    // setOpaqueMove
    window->setOption(QMdiSubWindow::TransparentMove, !opaqueMode);
    QCOMPARE(window->testOption(QMdiSubWindow::TransparentMove), !opaqueMode);

    EventSpy moveSpy(window, QEvent::Move);
    QCOMPARE(moveSpy.count(), 0);
    window->move(30, 30);
    QCOMPARE(moveSpy.count(), 1);
    moveSpy.clear();

    // Enter move mode
    QStyleOptionTitleBar options;
    options.initFrom(window);
    int height = window->style()->pixelMetric(QStyle::PM_TitleBarHeight, &options);
#if defined(Q_WS_MAC)
    // ### Remove this after mac style has been fixed
    height -= 4;
#endif
    QPoint mousePosition(window->width() / 2, height - 1);
    sendMouseMove(window, mousePosition, Qt::NoButton);
    sendMousePress(window, mousePosition);

    // Trigger move events
    for (int i = 0; i < geometryCount; ++i) {
        ++mousePosition.rx();
        ++mousePosition.ry();
        sendMouseMove(window, mousePosition);
    }

    // Leave move mode
    sendMouseRelease(window, mousePosition);
    QCOMPARE(moveSpy.count(), expectedGeometryCount);
    QCOMPARE(window->size(), windowSize + QSize(geometryCount, geometryCount));
    }
}

void tst_QMdiSubWindow::setWindowFlags_data()
{
    QTest::addColumn<Qt::WindowType>("windowType");
    QTest::addColumn<Qt::WindowType>("expectedWindowType");
    QTest::addColumn<Qt::WindowFlags>("customFlags");
    QTest::addColumn<Qt::WindowFlags>("expectedCustomFlags");

    // NB! I'm lazy, so if 'expectedCustomFlags' is set to 'Qt::WindowFlags(0)'
    // and nothing else, it means we're expecting the same as customFlags. :=/

    // Standard window types with no custom flags set.
    QTest::newRow("Qt::Widget") << Qt::Widget << Qt::SubWindow
                                << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::Window") << Qt::Window << Qt::SubWindow
                                << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::Dialog") << Qt::Dialog << Qt::SubWindow
                                << Qt::WindowFlags(0) << DialogWindowFlags;
    QTest::newRow("Qt::Sheet") << Qt::Sheet << Qt::SubWindow
                               << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::Drawer") << Qt::Drawer << Qt::SubWindow
                                << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::Popup") << Qt::Popup << Qt::SubWindow
                               << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::Tool") << Qt::Tool << Qt::SubWindow
                              << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::ToolTip") << Qt::ToolTip << Qt::SubWindow
                                 << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::SplashScreen") << Qt::SplashScreen << Qt::SubWindow
                                      << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::Desktop") << Qt::Desktop << Qt::SubWindow
                                 << Qt::WindowFlags(0) << StandardWindowFlags;
    QTest::newRow("Qt::SubWindow") << Qt::SubWindow << Qt::SubWindow
                                   << Qt::WindowFlags(0) << StandardWindowFlags;

    // Custom flags
    QTest::newRow("Title") << Qt::SubWindow << Qt::SubWindow
                           << (Qt::WindowTitleHint | Qt::WindowFlags(0))
                           << Qt::WindowFlags(0);
    QTest::newRow("TitleAndMin") << Qt::SubWindow << Qt::SubWindow
                                 << (Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint)
                                 << Qt::WindowFlags(0);
    QTest::newRow("TitleAndMax") << Qt::SubWindow << Qt::SubWindow
                                 << (Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint)
                                 << Qt::WindowFlags(0);
    QTest::newRow("TitleAndMinMax") << Qt::SubWindow << Qt::SubWindow
                                    << (Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint)
                                    << Qt::WindowFlags(0);
    QTest::newRow("Standard") << Qt::SubWindow << Qt::SubWindow
                              << StandardWindowFlags
                              << Qt::WindowFlags(0);
    QTest::newRow("StandardAndShade") << Qt::SubWindow << Qt::SubWindow
                                      << (StandardWindowFlags | Qt::WindowShadeButtonHint)
                                      << Qt::WindowFlags(0);
    QTest::newRow("StandardAndContext") << Qt::SubWindow << Qt::SubWindow
                                        << (StandardWindowFlags | Qt::WindowContextHelpButtonHint)
                                        << Qt::WindowFlags(0);
    QTest::newRow("StandardAndStaysOnTop") << Qt::SubWindow << Qt::SubWindow
                                           << (StandardWindowFlags | Qt::WindowStaysOnTopHint)
                                           << Qt::WindowFlags(0);
    QTest::newRow("StandardAndFrameless") << Qt::SubWindow << Qt::SubWindow
                                          << (StandardWindowFlags | Qt::FramelessWindowHint)
                                          << (Qt::FramelessWindowHint | Qt::WindowFlags(0));
    QTest::newRow("StandardAndFramelessAndStaysOnTop") << Qt::SubWindow << Qt::SubWindow
                                                       << (StandardWindowFlags | Qt::FramelessWindowHint
                                                           | Qt::WindowStaysOnTopHint)
                                                       << (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QTest::newRow("Shade") << Qt::SubWindow << Qt::SubWindow
                           << (Qt::WindowShadeButtonHint | Qt::WindowFlags(0))
                           << (StandardWindowFlags | Qt::WindowShadeButtonHint);
    QTest::newRow("ShadeAndCustomize") << Qt::SubWindow << Qt::SubWindow
                           << (Qt::WindowShadeButtonHint | Qt::CustomizeWindowHint)
                           << Qt::WindowFlags(0);
    QTest::newRow("Context") << Qt::SubWindow << Qt::SubWindow
                             << (Qt::WindowContextHelpButtonHint | Qt::WindowFlags(0))
                             << (StandardWindowFlags | Qt::WindowContextHelpButtonHint);
    QTest::newRow("ContextAndCustomize") << Qt::SubWindow << Qt::SubWindow
                             << (Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint)
                             << Qt::WindowFlags(0);
    QTest::newRow("ShadeAndContext") << Qt::SubWindow << Qt::SubWindow
                             << (Qt::WindowShadeButtonHint | Qt::WindowContextHelpButtonHint)
                             << (StandardWindowFlags | Qt::WindowShadeButtonHint | Qt::WindowContextHelpButtonHint);
    QTest::newRow("ShadeAndContextAndCustomize") << Qt::SubWindow << Qt::SubWindow
                             << (Qt::WindowShadeButtonHint | Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint)
                             << Qt::WindowFlags(0);
    QTest::newRow("OnlyCustomize") << Qt::SubWindow << Qt::SubWindow
                                   << (Qt::CustomizeWindowHint | Qt::WindowFlags(0))
                                   << Qt::WindowFlags(0);
}

void tst_QMdiSubWindow::setWindowFlags()
{
    QSKIP("Until we have a QEvent::WindowFlagsChange event, this will skip", SkipAll);
    QFETCH(Qt::WindowType, windowType);
    QFETCH(Qt::WindowType, expectedWindowType);
    QFETCH(Qt::WindowFlags, customFlags);
    QFETCH(Qt::WindowFlags, expectedCustomFlags);

    QMdiArea workspace;
    QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget));
    qApp->processEvents();
    workspace.show();
    window->show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&workspace);
#endif

    window->setWindowFlags(windowType | customFlags);
    QCOMPARE(window->windowType(), expectedWindowType);
    if (!expectedCustomFlags) // We expect the same as 'customFlags'
        QCOMPARE(window->windowFlags() & ~expectedWindowType, customFlags);
    else
        QCOMPARE(window->windowFlags() & ~expectedWindowType, expectedCustomFlags);

}

void tst_QMdiSubWindow::mouseDoubleClick()
{
    QMdiArea workspace;
    QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(workspace.addSubWindow(new QWidget));
    qApp->processEvents();
    workspace.show();
    window->show();

    QVERIFY(!window->isMaximized());
    QVERIFY(!window->isShaded());

    QRect originalGeometry = window->geometry();

    // Calculate mouse position
    QStyleOptionTitleBar options;
    options.initFrom(window);
    int height = window->style()->pixelMetric(QStyle::PM_TitleBarHeight, &options);
#if defined(Q_WS_MAC)
    // ### Remove this after mac style has been fixed
    height -= 4;
#endif
    QPoint mousePosition(window->width() / 2, height - 1);
    sendMouseMove(window, mousePosition, Qt::NoButton);

    // Without Qt::WindowShadeButtonHint flag set
    sendMouseDoubleClick(window, mousePosition);
    qApp->processEvents();
    QVERIFY(window->isMaximized());

    sendMouseDoubleClick(window, mousePosition);
    qApp->processEvents();
    QVERIFY(!window->isMaximized());
    QCOMPARE(window->geometry(), originalGeometry);

    // With Qt::WindowShadeButtonHint flag set
    window->setWindowFlags(window->windowFlags() | Qt::WindowShadeButtonHint);
    QVERIFY(window->windowFlags() & Qt::WindowShadeButtonHint);
    originalGeometry = window->geometry();
    sendMouseDoubleClick(window, mousePosition);
    qApp->processEvents();
    QVERIFY(window->isShaded());

    sendMouseDoubleClick(window, mousePosition);
    qApp->processEvents();
    QVERIFY(!window->isShaded());
    QCOMPARE(window->geometry(), originalGeometry);

    // Add test for minimized window
}

QTEST_MAIN(tst_QMdiSubWindow)
#include "tst_qmdisubwindow.moc"


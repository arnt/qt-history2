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
#include <QMenu>
#include <QGroupBox>
#include <QTextEdit>
#include <QLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QStyle>
#include <QStyleOptionTitleBar>
#include <QPushButton>
#include <QSizeGrip>
#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)
#include <QMacStyle>
#endif

#if defined(Q_WS_X11)
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif
#if !defined(Q_WS_WIN)
extern bool qt_tab_all_widgets;
#endif

static inline bool tabAllWidgets()
{
#if !defined(Q_WS_WIN)
    if (qApp->style()->inherits("QMacStyle"))
        return qt_tab_all_widgets;
#endif
    return true;
}

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
    } else if (signal == SIGNAL(aboutToActivate())) {
        if (window->parent()) {
            workspace->setActiveSubWindow(window);
            qApp->processEvents();
        }
    } else if (signal == SIGNAL(windowActivated())) {
        if (window->parent()) {
            workspace->setActiveSubWindow(window);
            qApp->processEvents();
        }
    } else if (signal == SIGNAL(windowDeactivated())) {
        if (!window->parent())
            return;
        workspace->setActiveSubWindow(window);
        qApp->processEvents();
        workspace->setActiveSubWindow(0);
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

//TESTED_CLASS=QMdiSubWindow
//TESTED_FILES=gui/widgets/qmdisubwindow.cpp gui/widgets/qmdisubwindow_p.h

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
    void showNormal_data();
    void showNormal();
    void setOpaqueResizeAndMove_data();
    void setOpaqueResizeAndMove();
    void setWindowFlags_data();
    void setWindowFlags();
    void mouseDoubleClick();
    void setSystemMenu();
    void restoreFocus();
    void changeFocusWithTab();
    void closeEvent();
    void setWindowTitle();
    void resizeEvents_data();
    void resizeEvents();
#if defined(Q_WS_MAC)
    void defaultSizeGrip();
#endif
    void hideAndShow();
    void keepWindowMaximizedState();
    void explicitlyHiddenWidget();
    void resizeTimer();
    void fixedMinMaxSize();
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
    workspace.addSubWindow(window);
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
    QCOMPARE(widget->parentWidget(), static_cast<QWidget *>(&window));
    QVERIFY(!widget->isVisible());
    QCOMPARE(window.layout()->count(), 1);

    QTest::ignoreMessage(QtWarningMsg,"QMdiSubWindow::setWidget: widget is already set");
    window.setWidget(widget);
    QCOMPARE(window.widget(), static_cast<QWidget *>(widget));
    QCOMPARE(widget->parentWidget(), static_cast<QWidget *>(&window));

    window.setWidget(0);
    QVERIFY(widget);
    QVERIFY(!widget->parent());
    QVERIFY(!window.widget());
    QCOMPARE(window.layout()->count(), 0);

    window.setWidget(widget);
    delete window.layout();
    QVERIFY(!window.layout());
    QVERIFY(window.widget());
    QCOMPARE(window.widget()->parentWidget(), static_cast<QWidget *>(&window));

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
    mainWindow.setCentralWidget(workspace);
    mainWindow.show();
    mainWindow.menuBar()->setVisible(true);
    qApp->setActiveWindow(&mainWindow);

    // QMainWindow's window title is empty
#if !defined(Q_WS_MAC)
    {
    QCOMPARE(mainWindow.windowTitle(), QString());
    QMdiSubWindow *window = workspace->addSubWindow(new QPushButton(QLatin1String("Test")));
    QString expectedTitle = QLatin1String("MainWindow's title is empty");
    window->setWindowTitle(expectedTitle);
    QCOMPARE(window->windowTitle(), expectedTitle);
    window->showMaximized();
    QVERIFY(window->isMaximized());
    QCOMPARE(window->windowTitle(), expectedTitle);
    QCOMPARE(mainWindow.windowTitle(), expectedTitle);
    window->showNormal();
    QCOMPARE(window->windowTitle(), expectedTitle);
    QCOMPARE(mainWindow.windowTitle(), QString());
    window->close();
    }
#endif

    QString originalWindowTitle = QString::fromLatin1("MainWindow");
    mainWindow.setWindowTitle(originalWindowTitle);

    for (int i = 0; i < 5; ++i) {
        mainWindow.menuBar()->setVisible(false);

        QMdiSubWindow *window = new QMdiSubWindow;
        windows.append(window);
        QVERIFY(!window->maximizedButtonsWidget());
        QVERIFY(!window->maximizedSystemMenuIconWidget());

        QMdiArea *nestedWorkspace = new QMdiArea; // :-)
        window->setWidget(nestedWorkspace);
        window->widget()->setWindowTitle(QString::fromLatin1("Window %1").arg(i));

        workspace->addSubWindow(window);
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
        nestedWorkspace->addSubWindow(nestedWindow);
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

    workspace->activateNextSubWindow();
    qApp->processEvents();
    foreach (QMdiSubWindow *window, windows) {
        QCOMPARE(workspace->activeSubWindow(), window);
        QVERIFY(window->isMaximized());
        QVERIFY(window->maximizedButtonsWidget());
        QCOMPARE(window->maximizedButtonsWidget(), mainWindow.menuBar()->cornerWidget(Qt::TopRightCorner));
        QVERIFY(window->maximizedSystemMenuIconWidget());
        QCOMPARE(window->maximizedSystemMenuIconWidget(), qobject_cast<QWidget *>(mainWindow.menuBar()
                                                                   ->cornerWidget(Qt::TopLeftCorner)));
        QCOMPARE(mainWindow.windowTitle(), QString::fromLatin1("%1 - [%2]")
                                           .arg(originalWindowTitle, window->widget()->windowTitle()));
        workspace->activateNextSubWindow();
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
    QTest::newRow("aboutToActivate") << QByteArray(SIGNAL(aboutToActivate())) << Qt::WindowNoState;
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
    if (signal != SIGNAL(windowRestored()))
        workspace.setActiveSubWindow(0);
    qApp->processEvents();

    QSignalSpy spy(window, signal == SIGNAL(aboutToActivate())
                           ? signal.data()
                           : SIGNAL(windowStateChanged(Qt::WindowStates, Qt::WindowStates)));
    QVERIFY(spy.isEmpty());
    triggerSignal(window, &workspace, signal);
    // Unless the signal is windowRestored or windowDeactivated,
    // we're already in correct state and nothing should happen.
    if (signal != SIGNAL(windowRestored()) && signal != SIGNAL(windowDeactivated()))
        triggerSignal(window, &workspace, signal);

    int count = 0;
    if (signal == SIGNAL(aboutToActivate())) {
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

    delete window;
    window = 0;
}

void tst_QMdiSubWindow::showShaded()
{
    QMdiArea workspace;
    QMdiSubWindow *window = workspace.addSubWindow(new QLineEdit);
    window->resize(300, 300);
    qApp->processEvents();
    workspace.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&workspace);
#endif

    QVERIFY(!window->isShaded());
    QVERIFY(!window->isMaximized());

    QCOMPARE(window->size(), QSize(300, 300));
    QRect restoreGeometry = window->geometry();
    window->showShaded();
    QVERIFY(window->isShaded());
    QVERIFY(window->isMinimized());

    window->showNormal();
    QVERIFY(!window->isShaded());
    QVERIFY(!window->isMinimized());
    QCOMPARE(window->geometry(), restoreGeometry);
    window->showShaded();

    window->showNormal();
    QVERIFY(!window->isShaded());
    QVERIFY(!window->isMinimized());
    QCOMPARE(window->geometry(), restoreGeometry);
    window->showMinimized();
    window->showMaximized();
    window->showShaded();
    QCOMPARE(window->width(), workspace.contentsRect().width());
    window->showNormal();
    QCOMPARE(window->geometry(), workspace.contentsRect());

    window->resize(300, 300);
    QCOMPARE(window->size(), QSize(300, 300));
    window->showShaded();
    window->showNormal();
    QTest::qWait(250);

    const QSize minimumSizeHint = window->minimumSizeHint();
    QVERIFY(minimumSizeHint.height() < 300);
    const int maxHeightDiff = 300 - minimumSizeHint.height();

    // Calculate mouse position for bottom right corner and simulate a
    // vertical resize with the mouse.
    int offset = window->style()->pixelMetric(QStyle::PM_MDIFrameWidth) / 2;
    QPoint mousePosition(window->width() - qMax(offset, 2), window->height() - qMax(offset, 2));
    QWidget *mouseReceiver = 0;
#ifdef Q_WS_MAC
    if (qobject_cast<QMacStyle*>(window->style()))
        mouseReceiver = qFindChild<QSizeGrip *>(window);
    else
#endif
        mouseReceiver = window;
    QVERIFY(mouseReceiver);
    sendMouseMove(mouseReceiver, mousePosition, Qt::NoButton);
    sendMousePress(mouseReceiver, mousePosition);

    for (int i = 0; i < maxHeightDiff + 20; ++i) {
        --mousePosition.ry();
        sendMouseMove(mouseReceiver, mousePosition);
    }

    sendMouseRelease(mouseReceiver, mousePosition);
    // Make sure we respect the minimumSizeHint!
    QCOMPARE(window->height(), minimumSizeHint.height());

    window->showShaded();
    window->setParent(0);
    window->show();
    QVERIFY(!window->isShaded());

    delete window;
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

    QTest::newRow("normal mode") << true<< 20 << 20 << QSize(400, 400) << QSize(200, 200);
    QTest::newRow("rubberband mode") << false << 20 << 1 << QSize(400, 400) << QSize(200, 200);
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
    workspace.resize(workspaceSize);
    workspace.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&workspace);
#endif

    QWidget *mouseReceiver = 0;
    if (window->style()->inherits("QMacStyle"))
        mouseReceiver = qFindChild<QSizeGrip *>(window);
    else
        mouseReceiver = window;
    QVERIFY(mouseReceiver);

    // ----------------------------- resize -----------------------------
    {
    // setOpaqueResize
    window->setOption(QMdiSubWindow::RubberBandResize, !opaqueMode);
    QCOMPARE(window->testOption(QMdiSubWindow::RubberBandResize), !opaqueMode);

    // Check that the event spy actually works
    EventSpy resizeSpy(window, QEvent::Resize);
    QCOMPARE(resizeSpy.count(), 0);
    window->resize(windowSize);
    QCOMPARE(window->size(), windowSize);
    QCOMPARE(resizeSpy.count(), 1);
    resizeSpy.clear();
    QCOMPARE(resizeSpy.count(), 0);

    QTest::qWait(250); // delayed update of dirty regions

    // Enter resize mode.
    int offset = window->style()->pixelMetric(QStyle::PM_MDIFrameWidth) / 2;
    QPoint mousePosition(mouseReceiver->width() - qMax(offset, 2), mouseReceiver->height() - qMax(offset, 2));
    sendMouseMove(mouseReceiver, mousePosition, Qt::NoButton);
    sendMousePress(mouseReceiver, mousePosition);

    // The window itself is the grabber in rubberband mode
    if (!opaqueMode) {
        mouseReceiver = window;
        mousePosition = QPoint(window->width() - qMax(offset, 2), window->height() - qMax(offset, 2));
    }

    // Trigger resize events
    for (int i = 0; i < geometryCount; ++i) {
        if (mouseReceiver == window) {
            ++mousePosition.rx();
            ++mousePosition.ry();
            sendMouseMove(mouseReceiver, mousePosition);
        } else {
            sendMouseMove(mouseReceiver, mousePosition + QPoint(1, 1));
        }
    }

    // Leave resize mode
    sendMouseRelease(mouseReceiver, mousePosition);
    QCOMPARE(resizeSpy.count(), expectedGeometryCount);
    QCOMPARE(window->size(), windowSize + QSize(geometryCount, geometryCount));
    }

    // ------------------------------ move ------------------------------
    {
    // setOpaqueMove
    window->setOption(QMdiSubWindow::RubberBandMove, !opaqueMode);
    QCOMPARE(window->testOption(QMdiSubWindow::RubberBandMove), !opaqueMode);

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
    // ### Remove this after mac style has been fixed
    if (window->style()->inherits("QMacStyle"))
        height -= 4;
    // has border
    if (!window->style()->styleHint(QStyle::SH_TitleBar_NoBorder, &options, window))
        height += window->isMinimized() ? 8 : 4;
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
    QSKIP("Until we have a QEvent::WindowFlagsChange event, this will skip", SkipAll);
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

    window->showMinimized();
    QVERIFY(window->isMinimized());
    sendMouseDoubleClick(window, mousePosition);
    QVERIFY(!window->isMinimized());
    QCOMPARE(window->geometry(), originalGeometry);
}

void tst_QMdiSubWindow::setSystemMenu()
{
    QMdiSubWindow *subWindow = new QMdiSubWindow;
    subWindow->resize(200, 50);
    QPointer<QMenu>systemMenu = subWindow->systemMenu();
    QVERIFY(systemMenu);
    QCOMPARE(subWindow->actions(), systemMenu->actions());

    QMainWindow mainWindow;
    QMdiArea *mdiArea = new QMdiArea;
    mdiArea->addSubWindow(subWindow);
    mainWindow.setCentralWidget(mdiArea);
    mainWindow.menuBar();
    mainWindow.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&mainWindow);
#endif

    QVERIFY(subWindow->isVisible());
    QPoint globalPopupPos = subWindow->mapToGlobal(subWindow->contentsRect().topLeft());

    // Show system menu
    QVERIFY(!qApp->activePopupWidget());
    subWindow->showSystemMenu();
    QTest::qWait(250);
    QCOMPARE(qApp->activePopupWidget(), qobject_cast<QWidget *>(systemMenu));
    QCOMPARE(systemMenu->mapToGlobal(QPoint(0, 0)), globalPopupPos);

    systemMenu->hide();
    QVERIFY(!qApp->activePopupWidget());

    QTest::ignoreMessage(QtWarningMsg, "QMdiSubWindow::setSystemMenu: system menu is already set");
    subWindow->setSystemMenu(systemMenu);

    subWindow->setSystemMenu(0);
    QVERIFY(!systemMenu); // systemMenu is QPointer

    systemMenu = new QMenu(subWindow);
    systemMenu->addAction(QIcon(subWindow->style()->standardIcon(QStyle::SP_TitleBarCloseButton)),
                          QObject::tr("&Close"), subWindow, SLOT(close()));
    subWindow->setSystemMenu(systemMenu);
    QCOMPARE(subWindow->systemMenu(), qobject_cast<QMenu *>(systemMenu));
    QCOMPARE(subWindow->systemMenu()->parentWidget(), static_cast<QWidget *>(subWindow));
    QCOMPARE(subWindow->systemMenu()->actions().count(), 1);

    // Show the new system menu
    QVERIFY(!qApp->activePopupWidget());
    subWindow->showSystemMenu();
    QTest::qWait(250);
    QCOMPARE(qApp->activePopupWidget(), qobject_cast<QWidget *>(systemMenu));
    QCOMPARE(systemMenu->mapToGlobal(QPoint(0, 0)), globalPopupPos);

    systemMenu->hide();
    QVERIFY(!qApp->activePopupWidget());

#ifndef Q_WS_MAC
    // System menu in menu bar.
    subWindow->showMaximized();
    QVERIFY(subWindow->isMaximized());
    QWidget *menuLabel = subWindow->maximizedSystemMenuIconWidget();
    QVERIFY(menuLabel);
    subWindow->showSystemMenu();
    QTest::qWait(250);
    QCOMPARE(qApp->activePopupWidget(), qobject_cast<QWidget *>(systemMenu));
    globalPopupPos = menuLabel->mapToGlobal(QPoint(0, menuLabel->y() + menuLabel->height()));
    QCOMPARE(systemMenu->mapToGlobal(QPoint(0, 0)), globalPopupPos);
    systemMenu->hide();
    QVERIFY(!qApp->activePopupWidget());
    subWindow->showNormal();
#endif

    // Reverse
    qApp->setLayoutDirection(Qt::RightToLeft);
    qApp->processEvents();
    mainWindow.updateGeometry();

    subWindow->showSystemMenu();
    QTest::qWait(250);
    QCOMPARE(qApp->activePopupWidget(), qobject_cast<QWidget *>(systemMenu));
    // + QPoint(1, 0) because topRight() == QPoint(left() + width() -1, top())
    globalPopupPos = subWindow->mapToGlobal(subWindow->contentsRect().topRight()) + QPoint(1, 0);
    globalPopupPos -= QPoint(systemMenu->sizeHint().width(), 0);
    QCOMPARE(systemMenu->mapToGlobal(QPoint(0, 0)), globalPopupPos);

    systemMenu->hide();
    QVERIFY(!qApp->activePopupWidget());

#ifndef Q_WS_MAC
    // System menu in menu bar in reverse mode.
    subWindow->showMaximized();
    QVERIFY(subWindow->isMaximized());
    menuLabel = subWindow->maximizedSystemMenuIconWidget();
    QVERIFY(menuLabel);
    subWindow->showSystemMenu();
    QTest::qWait(250);
    QCOMPARE(qApp->activePopupWidget(), qobject_cast<QWidget *>(systemMenu));
    globalPopupPos = menuLabel->mapToGlobal(QPoint(menuLabel->width(), menuLabel->y() + menuLabel->height()));
    globalPopupPos -= QPoint(systemMenu->sizeHint().width(), 0);
    QCOMPARE(systemMenu->mapToGlobal(QPoint(0, 0)), globalPopupPos);
#endif

    delete systemMenu;
    QVERIFY(!qApp->activePopupWidget());
    QVERIFY(!subWindow->systemMenu());

    // Restore layout direction.
    qApp->setLayoutDirection(Qt::LeftToRight);
}

void tst_QMdiSubWindow::restoreFocus()
{
    // Create complex layout.
    QGroupBox *box = new QGroupBox(tr("GroupBox"));
    box->setCheckable(true);

    QGroupBox *box1 = new QGroupBox(tr("&TopLeft"));
    box1->setLayout(new QHBoxLayout);
    box1->layout()->addWidget(new QTextEdit);

    QGroupBox *box2 = new QGroupBox(tr("&TopRight"));
    box2->setLayout(new QHBoxLayout);
    box2->layout()->addWidget(new QTextEdit);

    QGroupBox *box3 = new QGroupBox(tr("&BottomLeft"));
    box3->setLayout(new QHBoxLayout);
    box3->layout()->addWidget(new QTextEdit);

    QGroupBox *box4 = new QGroupBox(tr("&BottomRight"));
    box4->setLayout(new QHBoxLayout);
    QMdiArea *nestedWorkspace = new QMdiArea;
    for (int i = 0; i < 4; ++i)
        nestedWorkspace->addSubWindow(new QTextEdit)->show();
    qApp->processEvents();
    nestedWorkspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    nestedWorkspace->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    box4->layout()->addWidget(nestedWorkspace);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(box1, 0, 0);
    layout->addWidget(box2, 0, 1);
    layout->addWidget(box3, 1, 0);
    layout->addWidget(box4, 1, 1);

    box->setLayout(layout);

    // Add complex widget to workspace.
    QMdiArea topArea;
    QMdiSubWindow *complexWindow = topArea.addSubWindow(box);
    topArea.show();
    box->show();

    qApp->setActiveWindow(&topArea);
    QMdiSubWindow *expectedFocusWindow = nestedWorkspace->subWindowList().last();
    QVERIFY(expectedFocusWindow);
    QVERIFY(expectedFocusWindow->widget());
    QCOMPARE(qApp->focusWidget(), expectedFocusWindow->widget());

    // Normal -> minimized
    expectedFocusWindow->showMinimized();
    qApp->processEvents();
    QVERIFY(expectedFocusWindow->isMinimized());
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(expectedFocusWindow));

    // Minimized -> normal
    expectedFocusWindow->showNormal();
    qApp->processEvents();
    QVERIFY(!expectedFocusWindow->isMinimized());
    QCOMPARE(qApp->focusWidget(), expectedFocusWindow->widget());

    // Normal -> maximized
    expectedFocusWindow->showMaximized();
    qApp->processEvents();
    QVERIFY(expectedFocusWindow->isMaximized());
    QCOMPARE(qApp->focusWidget(), expectedFocusWindow->widget());

    // Maximized -> normal
    expectedFocusWindow->showNormal();
    qApp->processEvents();
    QVERIFY(!expectedFocusWindow->isMaximized());
    QCOMPARE(qApp->focusWidget(), expectedFocusWindow->widget());

    // Minimized -> maximized
    expectedFocusWindow->showMinimized();
    qApp->processEvents();
    QVERIFY(expectedFocusWindow->isMinimized());
    expectedFocusWindow->showMaximized();
    qApp->processEvents();
    QVERIFY(expectedFocusWindow->isMaximized());
    QCOMPARE(qApp->focusWidget(), expectedFocusWindow->widget());

    // Maximized -> minimized
    expectedFocusWindow->showNormal();
    qApp->processEvents();
    QVERIFY(!expectedFocusWindow->isMaximized());
    expectedFocusWindow->showMaximized();
    qApp->processEvents();
    QVERIFY(expectedFocusWindow->isMaximized());
    expectedFocusWindow->showMinimized();
    qApp->processEvents();
    QVERIFY(expectedFocusWindow->isMinimized());
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(expectedFocusWindow));

    complexWindow->showMinimized();
    qApp->processEvents();
    QVERIFY(complexWindow->isMinimized());
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(complexWindow));

    complexWindow->showNormal();
    qApp->processEvents();
    QVERIFY(!complexWindow->isMinimized());
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(expectedFocusWindow));
}

void tst_QMdiSubWindow::changeFocusWithTab()
{
    QWidget *widget = new QWidget;
    widget->setLayout(new QVBoxLayout);

    QLineEdit *firstLineEdit = new QLineEdit;
    widget->layout()->addWidget(firstLineEdit);
    QLineEdit *secondLineEdit = new QLineEdit;
    widget->layout()->addWidget(secondLineEdit);
    QLineEdit *thirdLineEdit = new QLineEdit;
    widget->layout()->addWidget(thirdLineEdit);

    QMdiArea mdiArea;
    mdiArea.addSubWindow(widget);
    mdiArea.show();
    QCOMPARE(mdiArea.subWindowList().count(), 1);

    qApp->setActiveWindow(&mdiArea);
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(firstLineEdit));

    // Next
    QTest::keyPress(widget, Qt::Key_Tab);
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(secondLineEdit));

    // Next
    QTest::keyPress(widget, Qt::Key_Tab);
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(thirdLineEdit));

    // Previous
    QTest::keyPress(widget, Qt::Key_Backtab);
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(secondLineEdit));

    // Previous
    QTest::keyPress(widget, Qt::Key_Backtab);
    QCOMPARE(qApp->focusWidget(), static_cast<QWidget *>(firstLineEdit));

    QMdiSubWindow *window = mdiArea.addSubWindow(new QPushButton);
    window->show();
    QCOMPARE(mdiArea.activeSubWindow(), window);

    // Check that we don't give away focus to another window by
    // just hitting tab if the child widget does not accept
    // focus (which is the case for a QPushButton).
    QTest::keyPress(window, Qt::Key_Tab);
    QCOMPARE(mdiArea.activeSubWindow(), window);
    QCOMPARE(qApp->focusWidget(), tabAllWidgets() ? window->widget() : window);
    QTest::keyPress(window, Qt::Key_Tab);
    QCOMPARE(mdiArea.activeSubWindow(), window);
    QCOMPARE(qApp->focusWidget(), tabAllWidgets() ? window->widget() : window);
}

class MyTextEdit : public QTextEdit
{
public:
    MyTextEdit(QWidget *parent = 0) : QTextEdit(parent), acceptClose(false) {}
    void setAcceptClose(bool enable = true) { acceptClose = enable; }
protected:
    void closeEvent(QCloseEvent *closeEvent)
    {
        if (!acceptClose)
            closeEvent->ignore();
    }

private:
    bool acceptClose;
};

void tst_QMdiSubWindow::closeEvent()
{
    QMdiArea mdiArea;
    mdiArea.show();

    MyTextEdit *textEdit = new MyTextEdit;
    textEdit->setAcceptClose(false);
    QMdiSubWindow *window = mdiArea.addSubWindow(textEdit);
    EventSpy closeSpy(window->widget(), QEvent::Close);
    window->show();

    QCOMPARE(closeSpy.count(), 0);
    QVERIFY(window->isVisible());
    QVERIFY(textEdit->isVisible());

    QVERIFY(!window->close());
    QCOMPARE(closeSpy.count(), 1);
    QVERIFY(window->isVisible());
    QVERIFY(textEdit->isVisible());

    QVERIFY(!textEdit->close());
    QCOMPARE(closeSpy.count(), 2);
    QVERIFY(window->isVisible());
    QVERIFY(textEdit->isVisible());

    textEdit->setAcceptClose(true);

    QVERIFY(window->close());
    QCOMPARE(closeSpy.count(), 3);
    QCOMPARE(mdiArea.subWindowList().count(), 0);
}

// There exists more tests in QMdiArea which covers window title support
// related to QMainWindow. This test is specific for QMdiSubWindow and its
// widget.
void tst_QMdiSubWindow::setWindowTitle()
{
    QString expectedWindowTitle = QLatin1String("This is teh shit[*]");
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setWindowTitle(expectedWindowTitle);
    QCOMPARE(textEdit->windowTitle(), expectedWindowTitle);
    textEdit->setWindowModified(true);
    QCOMPARE(textEdit->isWindowModified(), true);

    QMdiArea mdiArea;
    QMdiSubWindow *window = new QMdiSubWindow;
    mdiArea.addSubWindow(window);
    QCOMPARE(window->windowTitle(), QString());
    QVERIFY(!window->isWindowModified());

    window->setWidget(textEdit);
    QVERIFY(window->isWindowModified());
    QCOMPARE(textEdit->windowTitle(), expectedWindowTitle);
    QCOMPARE(window->windowTitle(), window->widget()->windowTitle());

    textEdit->setWindowModified(false);
    QVERIFY(!textEdit->isWindowModified());
    QVERIFY(!window->isWindowModified());
    // This will return the title including the astrix, but the
    // actual window title does not contain the astrix. This behavior
    // seems a bit odd, but is equal to e.g. QTextEdit (and probably all
    // other widgets which are not real top-level widgets).
    QCOMPARE(window->windowTitle(), expectedWindowTitle);

    textEdit->setWindowModified(true);;
    expectedWindowTitle = QLatin1String("Override child title");
    window->setWindowTitle(expectedWindowTitle);
    QVERIFY(window->isWindowModified());
    QCOMPARE(window->windowTitle(), expectedWindowTitle);

    textEdit->setWindowTitle(QLatin1String("My parent overrides me"));
    QCOMPARE(window->windowTitle(), expectedWindowTitle);

    textEdit->setWindowModified(false);
    QVERIFY(window->isWindowModified());
    QCOMPARE(window->windowTitle(), expectedWindowTitle);

    window->setWindowModified(false);
    QVERIFY(!window->isWindowModified());
    window->setWindowTitle(QString());
    QCOMPARE(window->windowTitle(), QString());

    expectedWindowTitle = QLatin1String("My parent doesn't have any title so now I can set one[*]");
    textEdit->setWindowTitle(expectedWindowTitle);
    QCOMPARE(window->windowTitle(), expectedWindowTitle);
    textEdit->setWindowModified(true);
    QVERIFY(window->isWindowModified());

    window->setWidget(0);
    QCOMPARE(window->windowTitle(), QString());
    QVERIFY(!window->isWindowModified());
    delete textEdit;
}

void tst_QMdiSubWindow::resizeEvents_data()
{
    QTest::addColumn<Qt::WindowState>("windowState");
    QTest::addColumn<int>("expectedWindowResizeEvents");
    QTest::addColumn<int>("expectedWidgetResizeEvents");
    QTest::addColumn<bool>("isShadeMode");

    QTest::newRow("minimized") << Qt::WindowMinimized << 1 << 0 << false;
    QTest::newRow("maximized") << Qt::WindowMaximized << 1 << 1 << false;
    QTest::newRow("shaded") << Qt::WindowMinimized << 1 << 0 << true;
}

void tst_QMdiSubWindow::resizeEvents()
{
    QFETCH(Qt::WindowState, windowState);
    QFETCH(int, expectedWindowResizeEvents);
    QFETCH(int, expectedWidgetResizeEvents);
    QFETCH(bool, isShadeMode);

    QMainWindow mainWindow;
    QMdiArea *mdiArea = new QMdiArea;
    mainWindow.setCentralWidget(mdiArea);
    mainWindow.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&mainWindow);
#endif

    QMdiSubWindow *window = mdiArea->addSubWindow(new QTextEdit);
    window->show();

    EventSpy windowResizeEventSpy(window, QEvent::Resize);
    QCOMPARE(windowResizeEventSpy.count(), 0);
    EventSpy widgetResizeEventSpy(window->widget(), QEvent::Resize);
    QCOMPARE(widgetResizeEventSpy.count(), 0);

    // Set the window state.
    if (!isShadeMode)
        window->setWindowState(windowState);
    else
        window->showShaded();

    // Check that the window state is correct.
    QCOMPARE(window->windowState(), windowState | Qt::WindowActive);
    QCOMPARE(window->widget()->windowState(), windowState);

    // Make sure we got as many resize events as expected.
    QCOMPARE(windowResizeEventSpy.count(), expectedWindowResizeEvents);
    QCOMPARE(widgetResizeEventSpy.count(), expectedWidgetResizeEvents);
    windowResizeEventSpy.clear();
    widgetResizeEventSpy.clear();

    // Normalize.
    window->showNormal();

    // Check that the window state is correct.
    QCOMPARE(window->windowState(), Qt::WindowNoState | Qt::WindowActive);
    QCOMPARE(window->widget()->windowState(), Qt::WindowNoState);

    // Make sure we got as many resize events as expected.
    QCOMPARE(windowResizeEventSpy.count(), expectedWindowResizeEvents);
    QCOMPARE(widgetResizeEventSpy.count(), expectedWidgetResizeEvents);
}

#if defined(Q_WS_MAC)
void tst_QMdiSubWindow::defaultSizeGrip()
{
    if (!qApp->style()->inherits("QMacStyle"))
        return;
    QMdiArea mdiArea;
    mdiArea.show();

    // QSizeGrip on windows with decoration.
    QMdiSubWindow *windowWithDecoration = mdiArea.addSubWindow(new QWidget);
    windowWithDecoration->show();
    QVERIFY(qFindChild<QSizeGrip *>(windowWithDecoration));

    // ...but not on windows without decoration (Qt::FramelessWindowHint).
    QMdiSubWindow *windowWithoutDecoration = mdiArea.addSubWindow(new QWidget, Qt::FramelessWindowHint);
    windowWithoutDecoration->show();
    QVERIFY(!qFindChild<QSizeGrip *>(windowWithoutDecoration));
}
#endif

void tst_QMdiSubWindow::hideAndShow()
{
    // Create a QTabWidget with two tabs; QMdiArea and QTextEdit.
    QTabWidget *tabWidget = new QTabWidget;
    QMdiArea *mdiArea = new QMdiArea;
    tabWidget->addTab(mdiArea, QLatin1String("QMdiArea"));
    tabWidget->addTab(new QTextEdit, QLatin1String("Dummy"));

    // Set the tab widget as the central widget in QMainWindow.
    QMainWindow mainWindow;
    QMenuBar *menuBar = mainWindow.menuBar();
    mainWindow.setCentralWidget(tabWidget);
    mainWindow.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&mainWindow);
#endif

    QVERIFY(!menuBar->cornerWidget(Qt::TopRightCorner));
    QMdiSubWindow *subWindow = mdiArea->addSubWindow(new QTextEdit);
    subWindow->showMaximized();
#ifndef Q_WS_MAC
    QVERIFY(menuBar->cornerWidget(Qt::TopRightCorner));
    QCOMPARE(menuBar->cornerWidget(Qt::TopRightCorner), subWindow->maximizedButtonsWidget());
#endif

    // Hide QMdiArea.
    tabWidget->setCurrentIndex(1);

    QVERIFY(!menuBar->cornerWidget(Qt::TopRightCorner));
    QVERIFY(!subWindow->maximizedButtonsWidget());
    QVERIFY(!subWindow->maximizedSystemMenuIconWidget());

    // Show QMdiArea.
    tabWidget->setCurrentIndex(0);

#ifndef Q_WS_MAC
    QVERIFY(menuBar->cornerWidget(Qt::TopRightCorner));
    QVERIFY(subWindow->maximizedButtonsWidget());
    QVERIFY(subWindow->maximizedSystemMenuIconWidget());
    QCOMPARE(menuBar->cornerWidget(Qt::TopRightCorner), subWindow->maximizedButtonsWidget());
#endif

    // Hide QMdiArea.
    tabWidget->setCurrentIndex(1);

    // Add few more windows.
    for (int i = 0; i < 5; ++i)
        mdiArea->addSubWindow(new QTextEdit);

    // Show QMdiArea.
    tabWidget->setCurrentIndex(0);
    qApp->processEvents();

    subWindow = mdiArea->subWindowList().back();
    QVERIFY(subWindow);
    QCOMPARE(mdiArea->activeSubWindow(), subWindow);

#ifndef Q_WS_MAC
    QVERIFY(menuBar->cornerWidget(Qt::TopRightCorner));
    QVERIFY(subWindow->maximizedButtonsWidget());
    QVERIFY(subWindow->maximizedSystemMenuIconWidget());
    QCOMPARE(menuBar->cornerWidget(Qt::TopRightCorner), subWindow->maximizedButtonsWidget());
#endif

    subWindow->showNormal();
    QVERIFY(!menuBar->cornerWidget(Qt::TopRightCorner));

    // Check that newly added windows got right sizes.
    foreach (QMdiSubWindow *window, mdiArea->subWindowList())
        QCOMPARE(window->size(), window->sizeHint());

    subWindow->showMaximized();
#ifndef Q_WS_MAC
    QCOMPARE(menuBar->cornerWidget(Qt::TopRightCorner), subWindow->maximizedButtonsWidget());
#endif

    subWindow->hide();
    QVERIFY(!subWindow->maximizedButtonsWidget());
    QVERIFY(!subWindow->maximizedSystemMenuIconWidget());
    QVERIFY(!menuBar->cornerWidget(Qt::TopRightCorner));

    subWindow->show();
#ifndef Q_WS_MAC
    QVERIFY(subWindow->maximizedButtonsWidget());
    QVERIFY(subWindow->maximizedSystemMenuIconWidget());
    QCOMPARE(menuBar->cornerWidget(Qt::TopRightCorner), subWindow->maximizedButtonsWidget());
#endif

    // Hide QMainWindow.
    mainWindow.hide();
    QVERIFY(!subWindow->maximizedButtonsWidget());
    QVERIFY(!subWindow->maximizedSystemMenuIconWidget());
    QVERIFY(!menuBar->cornerWidget(Qt::TopRightCorner));

    // Show QMainWindow.
    mainWindow.show();
#ifndef Q_WS_MAC
    QVERIFY(subWindow->maximizedButtonsWidget());
    QVERIFY(subWindow->maximizedSystemMenuIconWidget());
    QCOMPARE(menuBar->cornerWidget(Qt::TopRightCorner), subWindow->maximizedButtonsWidget());
#endif
}

void tst_QMdiSubWindow::keepWindowMaximizedState()
{
    QMdiArea mdiArea;
    QMdiSubWindow *subWindow = mdiArea.addSubWindow(new QTextEdit);
    mdiArea.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&mdiArea);
#endif

    subWindow->showMaximized();
    QVERIFY(subWindow->isMaximized());

    // move
    const QPoint newPosition = subWindow->pos() + QPoint(10, 10);
    subWindow->move(newPosition);
    QCOMPARE(subWindow->pos(), newPosition);
    QVERIFY(subWindow->isMaximized());

    // resize
    const QSize newSize = subWindow->size() - QSize(10, 10);
    subWindow->resize(newSize);
    QCOMPARE(subWindow->size(), newSize);
    QVERIFY(subWindow->isMaximized());

    // setGeometry
    const QRect newGeometry = QRect(newPosition - QPoint(10, 10), newSize + QSize(10, 10));
    subWindow->setGeometry(newGeometry);
    QCOMPARE(subWindow->geometry(), newGeometry);
    QVERIFY(subWindow->isMaximized());

    subWindow->showNormal();

    // Verify that we don't force Qt::WindowMaximized.
    QVERIFY(!subWindow->isMaximized());
    subWindow->setGeometry(QRect(newPosition, newSize));
    QCOMPARE(subWindow->geometry(), QRect(newPosition, newSize));
    QVERIFY(!subWindow->isMaximized());
}

void tst_QMdiSubWindow::explicitlyHiddenWidget()
{
    QMdiArea mdiArea;
    QTextEdit *textEdit = new QTextEdit;
    textEdit->hide();
    QMdiSubWindow *subWindow = mdiArea.addSubWindow(textEdit);
    mdiArea.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&mdiArea);
#endif

    QVERIFY(subWindow->isVisible());
    QVERIFY(!textEdit->isVisible());

    textEdit->show();
    QVERIFY(textEdit->isVisible());

    // normal -> minimized
    subWindow->showMinimized();
    QVERIFY(subWindow->isVisible());
    QVERIFY(!textEdit->isVisible());

    // minimized -> normal
    subWindow->showNormal();
    QVERIFY(subWindow->isVisible());
    QVERIFY(textEdit->isVisible());

    // minimized -> maximized
    subWindow->showMinimized();
    subWindow->showMaximized();
    QVERIFY(subWindow->isVisible());
    QVERIFY(textEdit->isVisible());

    textEdit->hide();

    // maximized -> normal
    subWindow->showNormal();
    QVERIFY(subWindow->isVisible());
    QVERIFY(!textEdit->isVisible());

    textEdit->show();

    subWindow->showMinimized();
    subWindow->setWidget(0);
    delete textEdit;
    textEdit = new QTextEdit;
    textEdit->hide();
    subWindow->setWidget(textEdit);
    subWindow->showNormal();
    QVERIFY(subWindow->isVisible());
    QVERIFY(!textEdit->isVisible());
}

void tst_QMdiSubWindow::resizeTimer()
{
    QMdiArea mdiArea;
    QMdiSubWindow *subWindow = mdiArea.addSubWindow(new QWidget);
    mdiArea.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&mdiArea);
#endif

    EventSpy timerEventSpy(subWindow, QEvent::Timer);
    QCOMPARE(timerEventSpy.count(), 0);

    for (int i = 0; i < 20; ++i) {
        subWindow->resize(subWindow->size() + QSize(2, 2));
        qApp->processEvents();
    }

    QTest::qWait(500); // Wait for timer events to occur.

    QCOMPARE(timerEventSpy.count(), 1);
}

void tst_QMdiSubWindow::fixedMinMaxSize()
{
    QMdiArea mdiArea;
    mdiArea.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&mdiArea);
#endif

    const QSize minimumSize = QSize(250, 150);
    const QSize maximumSize = QSize(300, 200);

    // Add the sub window to QMdiArea and set min/max size.
    QMdiSubWindow *subWindow = new QMdiSubWindow;
    subWindow->setMinimumSize(minimumSize);
    QCOMPARE(subWindow->minimumSize(), minimumSize);
    subWindow->setMaximumSize(maximumSize);
    QCOMPARE(subWindow->maximumSize(), maximumSize);
    mdiArea.addSubWindow(subWindow);
    subWindow->show();
    QCOMPARE(subWindow->size(), minimumSize);

    // Calculate the size of a minimized sub window.
    QStyleOptionTitleBar options;
    options.initFrom(subWindow);
    int minimizedHeight = subWindow->style()->pixelMetric(QStyle::PM_TitleBarHeight, &options);
#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)
    // ### Remove this after mac style has been fixed
    if (qobject_cast<QMacStyle *>(subWindow->style()))
        minimizedHeight -= 4;
#endif
    if (!subWindow->style()->styleHint(QStyle::SH_TitleBar_NoBorder, &options, subWindow))
        minimizedHeight += 8;
    int minimizedWidth = subWindow->style()->pixelMetric(QStyle::PM_MDIMinimizedWidth, &options);
    const QSize minimizedSize = QSize(minimizedWidth, minimizedHeight);

    // Even though the sub window has a minimum size set, it should be possible
    // to minimize the window.
    subWindow->showMinimized();
    QVERIFY(subWindow->isMinimized());
    QCOMPARE(subWindow->size(), minimizedSize);
    QCOMPARE(subWindow->minimumSize(), minimizedSize);

    // Restore minimum size.
    subWindow->showNormal();
    QVERIFY(!subWindow->isMinimized());
    QCOMPARE(subWindow->size(), minimumSize);
    QCOMPARE(subWindow->minimumSize(), minimumSize);

    // Well, the logic here is of course broken (calling showMaximized on a window with
    // maximum size set), but we should handle it :)
    subWindow->showMaximized();
    QVERIFY(subWindow->isMaximized());
    QCOMPARE(subWindow->size(), maximumSize);

    subWindow->showNormal();
    QVERIFY(!subWindow->isMaximized());
    QCOMPARE(subWindow->size(), minimumSize);
}

QTEST_MAIN(tst_QMdiSubWindow)
#include "tst_qmdisubwindow.moc"


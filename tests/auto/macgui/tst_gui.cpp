/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QMessageBox> 
#include <QtTest/QtTest>
#include <QSplashScreen>
#include <QScrollBar>
#include <QProgressDialog>
#include <QSpinBox>

#ifdef Q_OS_MAC

#include <guitest.h>

class tst_gui : public GuiTester
{
Q_OBJECT
private slots:
    void scrollbarPainting();
    
    void dummy();
    void splashScreenModality();
    void dialogModality();
    void nonModalOrder();

    void spinBoxArrowButtons();
};


QPixmap grabWindowContents(QWidget * widget)
{
    const int titleBarHeight = widget->frameGeometry().height() - widget->height();
    return QPixmap::grabWindow(widget->winId(), 0, titleBarHeight, -1, widget->height());
}

/*
    Test that vertical and horizontal mac-style scrollbars paint their
    entire area.
*/
void tst_gui::scrollbarPainting()
{
#if defined (Q_WS_MAC) && defined (__i386__)
    QSKIP("This test fails on scruffy when run by the autotest system (but not when you run it manually).", SkipAll);
#endif

    ColorWidget colorWidget;
    colorWidget.resize(400, 400);

    QSize scrollBarSize;

    QScrollBar verticalScrollbar(&colorWidget);
    verticalScrollbar.move(10, 10);
    scrollBarSize = verticalScrollbar.sizeHint();
    scrollBarSize.setHeight(200);
    verticalScrollbar.resize(scrollBarSize);

    QScrollBar horizontalScrollbar(&colorWidget);
    horizontalScrollbar.move(30, 10);
    horizontalScrollbar.setOrientation(Qt::Horizontal);
    scrollBarSize = horizontalScrollbar.sizeHint();
    scrollBarSize.setWidth(200);
    horizontalScrollbar.resize(scrollBarSize);

    colorWidget.show();
    QTest::qWait(100);

    QPixmap pixmap = grabWindowContents(&colorWidget);

    QVERIFY(isContent(pixmap.toImage(), verticalScrollbar.geometry(), GuiTester::Horizontal));
    QVERIFY(isContent(pixmap.toImage(), horizontalScrollbar.geometry(), GuiTester::Vertical));
}

// When running the auto-tests on scruffy, the first enter-the-event-loop-and-wait-for-a-click
// test that runs always times out, so we have this dummy test.
void tst_gui::dummy()
{
    QPixmap pix(100, 100);
    QSplashScreen splash(pix);
    splash.show();

    QMessageBox *box = new QMessageBox();
    box->setText("accessible?");
    box->show();

    // Find the "OK" button and schedule a press.
    InterfaceChildPair interface = wn.find(QAccessible::Name, "OK", box);
    QVERIFY(interface.iface);
    const int delay = 1000;
    clickLater(interface, Qt::LeftButton, delay);

    // Show dialog and and enter event loop.
    connect(wn.getWidget(interface), SIGNAL(clicked()), SLOT(exitLoopSlot()));
    const int timeout = 4;
    QTestEventLoop::instance().enterLoop(timeout);
}

/*
    Test that a message box pops up in front of a QSplashScreen.
*/
void tst_gui::splashScreenModality()
{
    QPixmap pix(100, 100);
    QSplashScreen splash(pix);
    splash.show();

    QMessageBox *box = new QMessageBox();
    box->setText("accessible?");
    box->show();

    // Find the "OK" button and schedule a press.
    InterfaceChildPair interface = wn.find(QAccessible::Name, "OK", box);
    QVERIFY(interface.iface);
    const int delay = 1000;
    clickLater(interface, Qt::LeftButton, delay);

    // Show dialog and and enter event loop.
    connect(wn.getWidget(interface), SIGNAL(clicked()), SLOT(exitLoopSlot()));
    const int timeout = 4;
    QTestEventLoop::instance().enterLoop(timeout);
    QVERIFY(QTestEventLoop::instance().timeout() == false);
}


/*
    Test that a non-modal dialog created as a child of a modal dialog is
    shown in front.
*/
void tst_gui::dialogModality()
{ 
    QDialog d;
    d.setModal(true);
    d.show();
    
    QProgressDialog progress(&d);
    progress.setValue(2);

    InterfaceChildPair interface = wn.find(QAccessible::Name, "Cancel", &progress);
    QVERIFY(interface.iface);
    const int delay = 2000;
    clickLater(interface, Qt::LeftButton, delay);
    
    connect(&progress, SIGNAL(canceled()), SLOT(exitLoopSlot()));

    const int timeout = 3;
    QTestEventLoop::instance().enterLoop(timeout);
    QVERIFY(QTestEventLoop::instance().timeout() == false);
}

class PrimaryWindowDialog : public QDialog
{
Q_OBJECT
public:
    PrimaryWindowDialog();
    QWidget *secondaryWindow;
    QWidget *frontWidget;
public slots:
    void showSecondaryWindow();
    void test();
};

PrimaryWindowDialog::PrimaryWindowDialog() : QDialog(0)
{
    frontWidget = 0;
    secondaryWindow = new ColorWidget(this);
    secondaryWindow->setWindowFlags(Qt::Window);
    secondaryWindow->resize(400, 400);
    secondaryWindow->move(100, 100);
    QTimer::singleShot(1000, this, SLOT(showSecondaryWindow()));
    QTimer::singleShot(2000, this, SLOT(test()));
    QTimer::singleShot(3000, this, SLOT(close()));
}

void PrimaryWindowDialog::showSecondaryWindow()
{
    secondaryWindow->show();
}

void PrimaryWindowDialog::test()
{
    frontWidget = QApplication::widgetAt(secondaryWindow->mapToGlobal(QPoint(100, 100)));
}

/*
    Test that a non-modal child window of a modal dialog is shown in front
    of the dialog even if the dialog becomes modal after the child window
    is created.
*/
void tst_gui::nonModalOrder()
{
    clearSequence();
    PrimaryWindowDialog primary;
    primary.resize(400, 400);
    primary.move(100, 100);
    primary.exec();
    QCOMPARE(primary.frontWidget, primary.secondaryWindow);
}

/*
    Test that the QSpinBox buttons are correctly positioned with the Mac style.
*/
void tst_gui::spinBoxArrowButtons()
{
    ColorWidget colorWidget;
    colorWidget.resize(200, 200);
    QSpinBox spinBox(&colorWidget);
    QSpinBox spinBox2(&colorWidget);
    spinBox2.move(0, 100);
    colorWidget.show();
    QTest::qWait(100);
    
    // Grab an unfocused spin box.
    const QImage noFocus = grabWindowContents(&colorWidget).toImage();

    // Set focus by clicking the less button.
    InterfaceChildPair lessInterface = wn.find(QAccessible::Name, "Less", &spinBox);
    QVERIFY(lessInterface.iface);
    const int delay = 500;
    clickLater(lessInterface, Qt::LeftButton, delay);
    const int timeout = 1;
    QTestEventLoop::instance().enterLoop(timeout);

    // Grab a focused spin box.
    const QImage focus = grabWindowContents(&colorWidget).toImage();

    // Compare the arrow area of the less button to see if it moved.
    const QRect lessRect = lessInterface.iface->rect(lessInterface.possibleChild);
    const QRect lessLocalRect(colorWidget.mapFromGlobal(lessRect.topLeft()), colorWidget.mapFromGlobal(lessRect.bottomRight()));
    const QRect compareRect = lessLocalRect.adjusted(5, 3, -5, -7);
    QVERIFY(noFocus.copy(compareRect) == focus.copy(compareRect));
}

QTEST_MAIN(tst_gui)

#else

QTEST_NOOP_MAIN

#endif

#include "tst_gui.moc"

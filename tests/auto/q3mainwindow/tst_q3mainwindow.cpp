/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <q3dockwindow.h>
#include <q3mainwindow.h>
#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qtoolbutton.h>

//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3mainwindow.h compat/widgets/q3mainwindow.cpp

class testMainWindow : public Q3MainWindow
{
public:
    testMainWindow(QWidget* parent=0, const char* name=0);
    ~testMainWindow();
    bool keysuccess;
protected:
    void keyPressEvent(QKeyEvent*);
};

class testLineEdit : public QLineEdit
{
public:
    testLineEdit(QWidget* parent=0, const char* name =0);
    ~testLineEdit();
protected:
    void keyPressEvent(QKeyEvent*);
};


class tst_Q3MainWindow : public QObject
{
    Q_OBJECT
public:
    tst_Q3MainWindow();
    ~tst_Q3MainWindow();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
private slots:
    void propagateEscapeKeyTest();
    void testDockWindowMinimized();

    void hideAndShow();

private:
    testMainWindow* testWidget;
    testLineEdit* le;
};

#ifdef Q_WS_X11
QT_BEGIN_NAMESPACE
extern void qt_x11_wait_for_window_manager( QWidget* w );
QT_END_NAMESPACE
#endif

testMainWindow::testMainWindow( QWidget* parent, const char* name )
    : Q3MainWindow( parent, name )
{
    keysuccess = FALSE;
}

testMainWindow::~testMainWindow()
{
}

void testMainWindow::keyPressEvent( QKeyEvent* ke )
{
    if ( ke->key() == Qt::Key_Escape )
	keysuccess = TRUE;
}


testLineEdit::testLineEdit( QWidget* parent, const char* name )
    : QLineEdit( parent, name )
{
}

testLineEdit::~testLineEdit()
{
}

void testLineEdit::keyPressEvent( QKeyEvent* ke )
{
    if ( ke->key() == Qt::Key_Escape )
	ke->ignore();
}

/*
    Nothing to do here.
*/

tst_Q3MainWindow::tst_Q3MainWindow()
{
}

/*
    Nothing to do here.
    The testwidget is deleted automatically.
*/

tst_Q3MainWindow::~tst_Q3MainWindow()
{
}

/*
    This function is called once when a testcase is being executed.
    You can use it to create the instance of a widget class and set it for instance
    as the mainwidget.
*/

void tst_Q3MainWindow::initTestCase()
{
    testWidget = new testMainWindow(0);
    QWidget *w = new QWidget(testWidget);
    testWidget->setCentralWidget(w);
    QVBoxLayout *vbl = new QVBoxLayout(w);
    le = new testLineEdit( w );
    vbl->addWidget(le);
    new Q3ToolBar(testWidget);
    qApp->setMainWidget( testWidget );
    testWidget->show();
}

void tst_Q3MainWindow::cleanupTestCase()
{
    delete testWidget;
}

/*
    Nothing to do here, but you could for instance use this to clean up temporary files
    you have been using in a test.
*/

void tst_Q3MainWindow::cleanup()
{
}

void tst_Q3MainWindow::propagateEscapeKeyTest()
{
    QTest::keyClick( testWidget, Qt::Key_Escape );
    QVERIFY( testWidget->keysuccess );
}

void tst_Q3MainWindow::testDockWindowMinimized()
{
    Q3MainWindow mw;
    Q3DockWindow *dw = new Q3DockWindow(&mw);
    QToolButton *btn = new QToolButton(dw);
    btn->setUsesTextLabel(true);
    btn->setTextLabel("foo");
    dw->setWidget(btn);
    mw.addDockWindow(dw, Qt::DockMinimized);
    mw.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&mw);
#endif
    qApp->processEvents();
    QEXPECT_FAIL(0, "This test started failing sometime during the 3.x lifetime", Continue);
    QVERIFY(dw->x() + dw->width() < 0);
    QEXPECT_FAIL(0, "This test started failing sometime during the 3.x lifetime", Continue);
    QVERIFY(dw->y() + dw->height() < 0);
}

void tst_Q3MainWindow::hideAndShow()
{
    Q3MainWindow mw;
    mw.show();

    Q3DockWindow *dw = new Q3DockWindow(&mw);
    QToolButton *btn = new QToolButton(dw);
    dw->setWidget(btn);
    mw.addDockWindow(dw, Qt::DockTornOff);

    QVERIFY(dw->isVisible());
    mw.hide();
    qApp->processEvents();
    QVERIFY(!dw->isVisible());
    mw.show();
    qApp->processEvents();

    QVERIFY(dw->isVisible());
}

QTEST_MAIN(tst_Q3MainWindow)
#include "tst_q3mainwindow.moc"


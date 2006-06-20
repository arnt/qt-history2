/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <q3dockarea.h>
#include <q3dockwindow.h>
#include <q3mainwindow.h>
#include <qapplication.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3dockwindow.h compat/widgets/q3dockwindow.cpp

class tst_Q3DockWindow : public QObject
{
    Q_OBJECT
public:
    tst_Q3DockWindow();
    virtual ~tst_Q3DockWindow();


public slots:
    void initTestCase();
    void cleanupTestCase();
private slots:
    void parents();
    void showChild();
};

QFrame *makeFrame( const char *text, QWidget *parent )
{
    QFrame* frame = new QFrame(parent);
    QVBoxLayout* layout = new QVBoxLayout(frame);
    layout->setAutoAdd(true);
    new QLabel(text, frame);
    frame->setMinimumSize(200, 200);
    return frame;
}

Q3DockWindow* makeDock( const char* text, QWidget* parent )
{
    Q3DockWindow* dock = new Q3DockWindow(Q3DockWindow::InDock, parent, text);
    dock->setResizeEnabled(true);
    dock->setCloseMode(Q3DockWindow::Always);
    dock->setCaption(text);
    dock->setWidget(makeFrame(text, dock));
    dock->show();

    return dock;
}


tst_Q3DockWindow::tst_Q3DockWindow()

{
}

tst_Q3DockWindow::~tst_Q3DockWindow()
{
}

void tst_Q3DockWindow::initTestCase()
{
    // create a default mainwindow
    // If you run a widget test, this will be replaced in the testcase by the
    // widget under test
    QWidget *w = new QWidget(0,"mainWidget");
    w->setFixedSize( 200, 200 );
    qApp->setMainWidget( w );
    w->show();
}

void tst_Q3DockWindow::cleanupTestCase()
{
    delete qApp->mainWidget();
}

void tst_Q3DockWindow::parents()
{
    // create 5 dock windows, one for each dock area
    // and one for the mainwindow, in the end they should
    // all except the one with the mainwindow as parent should
    // have the same dock() and parent() pointer.
    Q3MainWindow mw;
    QFrame *central = makeFrame( "Central", &mw );
    mw.setCentralWidget( central );

    Q3DockWindow *topDock = makeDock( "Top", mw.topDock() );
    QVERIFY( topDock->area() == topDock->parent() );

    Q3DockWindow *leftDock = makeDock( "Left", mw.leftDock() );
    QVERIFY( leftDock->area() == leftDock->parent() );

    Q3DockWindow *rightDock= makeDock( "Right", mw.rightDock() );
    QVERIFY( rightDock->area() == rightDock->parent() );

    Q3DockWindow *bottomDock = makeDock( "Bottom", mw.bottomDock() );
    QVERIFY( bottomDock->area() == mw.bottomDock() );

    Q3DockWindow *mainDock = makeDock( "MainWindow as parent", &mw );
    QVERIFY( mainDock->parent() == mw.topDock() );
}


void tst_Q3DockWindow::showChild()
{
    // task 26225
    // calling show dose not propergate to child widgets if
    // main window is already showing

    Q3MainWindow mw;
    mw.show();
    Q3DockWindow * dock = new Q3DockWindow(&mw);
    QPushButton  * qpb = new QPushButton("hi", dock);
    dock->setWidget(qpb);
    dock->show();
    QVERIFY( mw.isVisible() );
    QVERIFY( dock->isVisible() );
    QVERIFY( qpb->isVisible() );
}



QTEST_MAIN(tst_Q3DockWindow)
#include "tst_q3dockwindow.moc"


/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qapplication.h>
#include <qlineedit.h>
#include <q3popupmenu.h>
#include <qlabel.h>
#include <qdialog.h>




#include <qevent.h>
#include <qlineedit.h>

QT_DECLARE_CLASS(QWidget)

//TESTED_CLASS=
//TESTED_FILES=gui/src/qevent.h gui/src/qevent.cpp

class FocusLineEdit : public QLineEdit
{
public:
    FocusLineEdit( QWidget* parent = 0, const char* name = 0 ) : QLineEdit( parent, name ) {}
    int focusInEventReason;
    int focusOutEventReason;
    bool focusInEventRecieved;
    bool focusInEventGotFocus;
    bool focusOutEventRecieved;
    bool focusOutEventLostFocus;
protected:
    virtual void keyPressEvent( QKeyEvent *e )
    {
//        qDebug( QString("keyPressEvent: %1").arg(e->key()) );
        QLineEdit::keyPressEvent( e );
    }
    void focusInEvent( QFocusEvent* e )
    {
        QLineEdit::focusInEvent( e );
        focusInEventReason = e->reason();
	    focusInEventGotFocus = e->gotFocus();
        focusInEventRecieved = TRUE;
    }
    void focusOutEvent( QFocusEvent* e )
    {
        QLineEdit::focusOutEvent( e );
        focusOutEventReason = e->reason();
	    focusOutEventLostFocus = !e->gotFocus();
        focusOutEventRecieved = TRUE;
    }
};

class tst_QFocusEvent : public QObject
{
    Q_OBJECT

public:
    tst_QFocusEvent();
    virtual ~tst_QFocusEvent();


    void initWidget();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void checkReason_Tab();
    void checkReason_ShiftTab();
    void checkReason_BackTab();
    void checkReason_Popup();
    void checkReason_Shortcut();
    void checkReason_ActiveWindow();

private:
    QWidget* testFocusWidget;
    FocusLineEdit* childFocusWidgetOne;
    FocusLineEdit* childFocusWidgetTwo;
};

tst_QFocusEvent::tst_QFocusEvent()
{
}

tst_QFocusEvent::~tst_QFocusEvent()
{

}

void tst_QFocusEvent::initTestCase()
{
    testFocusWidget = new QWidget( 0 );
    childFocusWidgetOne = new FocusLineEdit( testFocusWidget );
    childFocusWidgetOne->setGeometry( 10, 10, 180, 20 );
    childFocusWidgetTwo = new FocusLineEdit( testFocusWidget );
    childFocusWidgetTwo->setGeometry( 10, 50, 180, 20 );

    qApp->setMainWidget( testFocusWidget );
    testFocusWidget->resize( 200,100 );
    testFocusWidget->show();
}

void tst_QFocusEvent::cleanupTestCase()
{
    delete testFocusWidget;
}

void tst_QFocusEvent::init()
{
}

void tst_QFocusEvent::cleanup()
{
    childFocusWidgetTwo->setGeometry( 10, 50, 180, 20 );
}

void tst_QFocusEvent::initWidget()
{
    // On X11 we have to ensure the event was processed before doing any checking, on Windows
    // this is processed straight away.

    for (int i = 0; i < 1000; ++i) {
	if (childFocusWidgetOne->isActiveWindow() && childFocusWidgetOne->hasFocus())
	    break;
	childFocusWidgetOne->activateWindow();
	childFocusWidgetOne->setFocus();
	qApp->processEvents();
	QTest::qWait(100);
    }

    // The first lineedit should have focus
    QVERIFY( childFocusWidgetOne->hasFocus() );

    childFocusWidgetOne->focusInEventRecieved = FALSE;
    childFocusWidgetOne->focusInEventGotFocus = FALSE;
    childFocusWidgetOne->focusInEventReason = -1;
    childFocusWidgetOne->focusOutEventRecieved = FALSE;
    childFocusWidgetOne->focusOutEventLostFocus = FALSE;
    childFocusWidgetOne->focusOutEventReason = -1;
    childFocusWidgetTwo->focusInEventRecieved = FALSE;
    childFocusWidgetTwo->focusInEventGotFocus = FALSE;
    childFocusWidgetTwo->focusInEventReason = -1;
    childFocusWidgetTwo->focusOutEventRecieved = FALSE;
    childFocusWidgetTwo->focusOutEventLostFocus = FALSE;
    childFocusWidgetTwo->focusOutEventReason = -1;
}

void tst_QFocusEvent::checkReason_Tab()
{
    initWidget();

    // Now test the tab key
    QTest::keyClick( childFocusWidgetOne, Qt::Key_Tab );

    QVERIFY(childFocusWidgetOne->focusOutEventRecieved);
    QVERIFY(childFocusWidgetTwo->focusInEventRecieved);
    QVERIFY(childFocusWidgetOne->focusOutEventLostFocus);
    QVERIFY(childFocusWidgetTwo->focusInEventGotFocus);

    QVERIFY( childFocusWidgetTwo->hasFocus() );
    QCOMPARE( childFocusWidgetOne->focusOutEventReason, (int)QFocusEvent::Tab );
    QCOMPARE( childFocusWidgetTwo->focusInEventReason, (int)QFocusEvent::Tab );
}

void tst_QFocusEvent::checkReason_ShiftTab()
{
    initWidget();

    // Now test the shift + tab key
    QTest::keyClick( childFocusWidgetOne, Qt::Key_Tab, Qt::ShiftModifier );

    QVERIFY(childFocusWidgetOne->focusOutEventRecieved);
    QVERIFY(childFocusWidgetTwo->focusInEventRecieved);
    QVERIFY(childFocusWidgetOne->focusOutEventLostFocus);
    QVERIFY(childFocusWidgetTwo->focusInEventGotFocus);

    QVERIFY( childFocusWidgetTwo->hasFocus() );
    QCOMPARE( childFocusWidgetOne->focusOutEventReason, (int)QFocusEvent::Backtab );
    QCOMPARE( childFocusWidgetTwo->focusInEventReason, (int)QFocusEvent::Backtab );

}

/*!
    In this test we verify that the Qt::KeyBacktab key is handled in a qfocusevent
*/
void tst_QFocusEvent::checkReason_BackTab()
{
#ifdef Q_OS_WIN32 // key is not supported on Windows
    QSKIP( "Backtab is not supported on Windows", SkipAll );
#else
    initWidget();
    QVERIFY( childFocusWidgetOne->hasFocus() );

    // Now test the backtab key
    QTest::keyClick( childFocusWidgetOne, Qt::Key_BackTab );
    QTest::qWait(2000);

    QVERIFY(childFocusWidgetOne->focusOutEventRecieved);
    QVERIFY(childFocusWidgetTwo->focusInEventRecieved);
    QVERIFY(childFocusWidgetOne->focusOutEventLostFocus);
    QVERIFY(childFocusWidgetTwo->focusInEventGotFocus);

    QVERIFY( childFocusWidgetTwo->hasFocus() );
    QCOMPARE( childFocusWidgetOne->focusOutEventReason, int(QFocusEvent::Backtab) );
    QCOMPARE( childFocusWidgetTwo->focusInEventReason, int(QFocusEvent::Backtab) );
#endif
}

void tst_QFocusEvent::checkReason_Popup()
{
    initWidget();

    // Now test the popup reason
    Q3PopupMenu* popupMenu = new Q3PopupMenu( testFocusWidget );
    popupMenu->insertItem( "Test" );
    popupMenu->popup( QPoint(0,0) );
    QTest::qWait(500);

    QVERIFY(childFocusWidgetOne->focusOutEventLostFocus);

    QVERIFY( childFocusWidgetOne->hasFocus() );
    QVERIFY( !childFocusWidgetOne->focusInEventRecieved );
    QVERIFY( childFocusWidgetOne->focusOutEventRecieved );
    QVERIFY( !childFocusWidgetTwo->focusInEventRecieved );
    QVERIFY( !childFocusWidgetTwo->focusOutEventRecieved );
    QCOMPARE( childFocusWidgetOne->focusOutEventReason, int(QFocusEvent::Popup));

    popupMenu->hide();

    QVERIFY(childFocusWidgetOne->focusInEventRecieved);
    QVERIFY(childFocusWidgetOne->focusInEventGotFocus);

    QVERIFY( childFocusWidgetOne->hasFocus() );
    QVERIFY( childFocusWidgetOne->focusInEventRecieved );
    QVERIFY( childFocusWidgetOne->focusOutEventRecieved );
    QVERIFY( !childFocusWidgetTwo->focusInEventRecieved );
    QVERIFY( !childFocusWidgetTwo->focusOutEventRecieved );
}

#ifdef Q_WS_MAC
QT_BEGIN_NAMESPACE
    extern void qt_set_sequence_auto_mnemonic(bool);
QT_END_NAMESPACE
#endif

void tst_QFocusEvent::checkReason_Shortcut()
{
    initWidget();
#ifdef Q_WS_MAC
    qt_set_sequence_auto_mnemonic(true);
#endif
    QLabel* label = new QLabel( "&Test", testFocusWidget );
    label->setBuddy( childFocusWidgetTwo );
    label->setGeometry( 10, 50, 90, 20 );
    childFocusWidgetTwo->setGeometry( 105, 50, 95, 20 );
    label->show();

    QVERIFY( childFocusWidgetOne->hasFocus() );
    QVERIFY( !childFocusWidgetTwo->hasFocus() );

    QTest::keyClick( label, Qt::Key_T, Qt::AltModifier );

    QVERIFY(childFocusWidgetOne->focusOutEventRecieved);
    QVERIFY(childFocusWidgetTwo->focusInEventRecieved);
    QVERIFY(childFocusWidgetOne->focusOutEventLostFocus);
    QVERIFY(childFocusWidgetTwo->focusInEventGotFocus);

    QVERIFY( childFocusWidgetTwo->hasFocus() );
    QVERIFY( !childFocusWidgetOne->focusInEventRecieved );
    QVERIFY( childFocusWidgetOne->focusOutEventRecieved );
    QCOMPARE( childFocusWidgetOne->focusOutEventReason, (int)QFocusEvent::Shortcut );
    QVERIFY( childFocusWidgetTwo->focusInEventRecieved );
    QCOMPARE( childFocusWidgetTwo->focusInEventReason, (int)QFocusEvent::Shortcut );
    QVERIFY( !childFocusWidgetTwo->focusOutEventRecieved );

    label->hide();
    QVERIFY( childFocusWidgetTwo->hasFocus() );
    QVERIFY( !childFocusWidgetOne->hasFocus() );
#ifdef Q_WS_MAC
    qt_set_sequence_auto_mnemonic(false);
#endif
}

void tst_QFocusEvent::checkReason_ActiveWindow()
{
    initWidget();

    QDialog* d = new QDialog( testFocusWidget );
    QLineEdit *le = new QLineEdit(d);
    d->show();
    d->activateWindow(); // ### CDE
    // wait 1 secs to give some visible feedback
    QTest::qWait(1000);

    QVERIFY(childFocusWidgetOne->focusOutEventRecieved);
    QVERIFY(childFocusWidgetOne->focusOutEventLostFocus);

    QVERIFY( !childFocusWidgetOne->focusInEventRecieved );
    QVERIFY( childFocusWidgetOne->focusOutEventRecieved );
    QCOMPARE( childFocusWidgetOne->focusOutEventReason, (int)Qt::ActiveWindowFocusReason);
    QVERIFY( !childFocusWidgetOne->hasFocus() );

    d->hide();
    QTest::qWait(1000);

    QVERIFY(childFocusWidgetOne->focusInEventRecieved);
    QVERIFY(childFocusWidgetOne->focusInEventGotFocus);

    QVERIFY( childFocusWidgetOne->hasFocus() );
    QVERIFY( childFocusWidgetOne->focusInEventRecieved );
    QCOMPARE( childFocusWidgetOne->focusInEventReason, (int)Qt::ActiveWindowFocusReason);
}


QTEST_MAIN(tst_QFocusEvent)
#include "tst_qfocusevent.moc"

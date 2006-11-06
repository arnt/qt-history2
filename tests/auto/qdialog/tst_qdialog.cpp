/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qdialog.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qpushbutton.h>

Q_DECLARE_METATYPE(QSize)


class QDialog;

//TESTED_CLASS=
//TESTED_FILES=gui/dialogs/qdialog.h gui/dialogs/qdialog.cpp

class tst_QDialog : public QObject
{
    Q_OBJECT
public:
    tst_QDialog();

public slots:
    void initTestCase();
    void cleanupTestCase();
private slots:
    void getSetCheck();
    void showExtension_data();
    void showExtension();
    void defaultButtons();
    void showMaximized();
    void showMinimized();
    void showFullScreen();
    void showAsTool();
    void toolDialogPosition();
    void deleteMainDefault();
    void deleteInExec();
    void showSizeGrip();

private:
    QDialog *testWidget;
};

// Testing get/set functions
void tst_QDialog::getSetCheck()
{
    QDialog obj1;
    // QWidget* QDialog::extension()
    // void QDialog::setExtension(QWidget*)
    QWidget *var1 = new QWidget;
    obj1.setExtension(var1);
    QCOMPARE(var1, obj1.extension());
    obj1.setExtension((QWidget *)0);
    QCOMPARE((QWidget *)0, obj1.extension());
    // No delete var1, since setExtension takes ownership

    // int QDialog::result()
    // void QDialog::setResult(int)
    obj1.setResult(0);
    QCOMPARE(0, obj1.result());
    obj1.setResult(INT_MIN);
    QCOMPARE(INT_MIN, obj1.result());
    obj1.setResult(INT_MAX);
    QCOMPARE(INT_MAX, obj1.result());
}

// work around function being protected
class DummyDialog : public QDialog {
public:
    DummyDialog(): QDialog(0) {}
    void showExtension( bool b ) { QDialog::showExtension( b ); }
};

class ToolDialog : public QDialog
{
public:
    ToolDialog(QWidget *parent = 0) : QDialog(parent, Qt::Tool), mWasActive(false), tId(-1) {
    }
    bool wasActive() const { return mWasActive; }

    int exec() {
        tId = startTimer(300);
        return QDialog::exec();
    }
protected:
    void timerEvent(QTimerEvent *event) {
        if (tId == event->timerId()) {
            killTimer(tId);
            mWasActive = isActiveWindow();
            reject();
        }
    }

private:
    int mWasActive;
    int tId;
};

tst_QDialog::tst_QDialog()

{
}

void tst_QDialog::initTestCase()
{
    // Create the test class
    testWidget = new QDialog(0, Qt::X11BypassWindowManagerHint);
    testWidget->resize(200,200);
    testWidget->show();
    qApp->setActiveWindow(testWidget);
}

void tst_QDialog::cleanupTestCase()
{
    if (testWidget) {
	delete testWidget;
	testWidget = 0;
    }
}

void tst_QDialog::showExtension_data()
{
    QTest::addColumn<QSize>("dlgSize");
    QTest::addColumn<QSize>("extSize");
    QTest::addColumn<bool>("horizontal");
    QTest::addColumn<QSize>("result");

    //next we fill it with data
    QTest::newRow( "data0" )  << QSize(100,100) << QSize(50,50) << (bool)FALSE << QSize(100,150);
    QTest::newRow( "data1" )  << QSize(100,100) << QSize(120,50) << (bool)FALSE << QSize(120,150);
    QTest::newRow( "data2" )  << QSize(100,100) << QSize(50,50) << (bool)TRUE << QSize(150,100);
    QTest::newRow( "data3" )  << QSize(100,100) << QSize(50,120) << (bool)TRUE << QSize(150,120);
}

void tst_QDialog::showExtension()
{
    QFETCH( QSize, dlgSize );
    QFETCH( QSize, extSize );
    QFETCH( bool, horizontal );

    // set geometry of main dialog and extension widget
    testWidget->setFixedSize( dlgSize );
    QWidget *ext = new QWidget( testWidget );
    ext->setFixedSize( extSize );
    testWidget->setExtension( ext );
    testWidget->setOrientation( horizontal ? Qt::Horizontal : Qt::Vertical );

    QCOMPARE( testWidget->size(), dlgSize );

    // show
    ((DummyDialog*)testWidget)->showExtension( TRUE );
//     while ( testWidget->size() == dlgSize )
// 	qApp->processEvents();
    QTEST( testWidget->size(), "result"  );

    // hide extension. back to old size ?
    ((DummyDialog*)testWidget)->showExtension( FALSE );
    QCOMPARE( testWidget->size(), dlgSize );

    testWidget->setExtension( 0 );
}

void tst_QDialog::defaultButtons()
{
    QLineEdit *lineEdit = new QLineEdit(testWidget);
    QPushButton *push = new QPushButton("Button 1", testWidget);
    QPushButton *pushTwo = new QPushButton("Button 2", testWidget);
    QPushButton *pushThree = new QPushButton("Button 3", testWidget);
    pushThree->setAutoDefault(FALSE);

    push->setDefault(TRUE);
    QVERIFY(push->isDefault());

    pushTwo->setFocus();
    QVERIFY(pushTwo->isDefault());
    pushThree->setFocus();
    QVERIFY(push->isDefault());
    lineEdit->setFocus();
    QVERIFY(push->isDefault());

    pushTwo->setDefault(TRUE);
    QVERIFY(pushTwo->isDefault());

    pushTwo->setFocus();
    QVERIFY(pushTwo->isDefault());
    lineEdit->setFocus();
    QVERIFY(pushTwo->isDefault());
}

void tst_QDialog::showMaximized()
{
    QDialog dialog(0);

    dialog.showMaximized();
    QVERIFY(dialog.isMaximized());
    QVERIFY(dialog.isVisible());

    dialog.showNormal();
    QVERIFY(!dialog.isMaximized());
    QVERIFY(dialog.isVisible());

    dialog.showMaximized();
    QVERIFY(dialog.isMaximized());
    QVERIFY(dialog.isVisible());

    dialog.hide();
    QVERIFY(dialog.isMaximized());
    QVERIFY(!dialog.isVisible());

    dialog.show();
    QVERIFY(dialog.isMaximized());
    QVERIFY(dialog.isVisible());

    dialog.hide();
    QVERIFY(dialog.isMaximized());
    QVERIFY(!dialog.isVisible());

    dialog.showMaximized();
    QVERIFY(dialog.isMaximized());
    QVERIFY(dialog.isVisible());
}

void tst_QDialog::showMinimized()
{
    QDialog dialog(0);

    dialog.showMinimized();
    QVERIFY(dialog.isMinimized());
    QVERIFY(dialog.isVisible());

    dialog.showNormal();
    QVERIFY(!dialog.isMinimized());
    QVERIFY(dialog.isVisible());

    dialog.showMinimized();
    QVERIFY(dialog.isMinimized());
    QVERIFY(dialog.isVisible());

    dialog.hide();
    QVERIFY(dialog.isMinimized());
    QVERIFY(!dialog.isVisible());

    dialog.show();
    QVERIFY(dialog.isMinimized());
    QVERIFY(dialog.isVisible());

    dialog.hide();
    QVERIFY(dialog.isMinimized());
    QVERIFY(!dialog.isVisible());

    dialog.showMinimized();
    QVERIFY(dialog.isMinimized());
    QVERIFY(dialog.isVisible());
}

void tst_QDialog::showFullScreen()
{
    QDialog dialog(0, Qt::X11BypassWindowManagerHint);

    qApp->syncX();
    dialog.showFullScreen();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(dialog.isVisible());

    qApp->syncX();
    dialog.showNormal();
    QVERIFY(!dialog.isFullScreen());
    QVERIFY(dialog.isVisible());

    qApp->syncX();
    dialog.showFullScreen();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(dialog.isVisible());

    qApp->syncX();
    dialog.hide();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(!dialog.isVisible());

    qApp->syncX();
    dialog.show();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(dialog.isVisible());

    qApp->syncX();
    dialog.hide();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(!dialog.isVisible());

    qApp->syncX();
    dialog.showFullScreen();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(dialog.isVisible());

    qApp->syncX();
    dialog.hide();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(!dialog.isVisible());
}

void tst_QDialog::showAsTool()
{
#if defined(Q_WS_X11)
    QSKIP("Qt/X11: Skipped since activeWindow() is not respected by all window managers", SkipAll);
#elif defined(Q_WS_QWS)  && QT_VERSION < 0x040300
    QEXPECT_FAIL(0, "Qtopia Core has messed up WStyle_Tool  (task 126435)", Continue);
#endif
    ToolDialog dialog(testWidget);
    testWidget->activateWindow();
    dialog.exec();
    QCOMPARE(dialog.wasActive(), true);
}

// Verify that pos() returns the same before and after show()
// for a dialog with the Tool window type.
void tst_QDialog::toolDialogPosition()
{
	QDialog dialog(0, Qt::Tool);
	dialog.move(QPoint(100,100));
    const QPoint beforeShowPosition = dialog.pos();
	dialog.show();
    const QPoint afterShowPosition = dialog.pos();
    QCOMPARE(afterShowPosition, beforeShowPosition);
}

class Dialog : public QDialog
{
public:
    Dialog(QPushButton *&button)
    {
        button = new QPushButton(this);
    }
};

void tst_QDialog::deleteMainDefault()
{
    QPushButton *button;
    Dialog dialog(button);
    button->setDefault(true);
    delete button;
    dialog.show();
    QTestEventLoop::instance().enterLoop(2);
}

void tst_QDialog::deleteInExec()
{
    QDialog *dialog = new QDialog(0);
    QMetaObject::invokeMethod(dialog, "deleteLater", Qt::QueuedConnection);
    QCOMPARE(dialog->exec(), int(QDialog::Rejected));
}

// From Task 124269
void tst_QDialog::showSizeGrip()
{
#ifndef QT_NO_SIZEGRIP
    QDialog dialog(0);
    dialog.show();
    QWidget *ext = new QWidget(&dialog);
    QVERIFY(!dialog.extension());
    QVERIFY(!dialog.isSizeGripEnabled());

    dialog.setSizeGripEnabled(true);
    QVERIFY(dialog.isSizeGripEnabled());

    dialog.setExtension(ext);
    QVERIFY(dialog.extension() && !dialog.extension()->isVisible());
    QVERIFY(dialog.isSizeGripEnabled());

    // normal show/hide sequence
    dialog.showExtension(true);
    QVERIFY(dialog.extension() && dialog.extension()->isVisible());
    QVERIFY(!dialog.isSizeGripEnabled());

    dialog.showExtension(false);
    QVERIFY(dialog.extension() && !dialog.extension()->isVisible());
    QVERIFY(dialog.isSizeGripEnabled());

    // show/hide sequence with interleaved size grip update
    dialog.showExtension(true);
    QVERIFY(dialog.extension() && dialog.extension()->isVisible());
    QVERIFY(!dialog.isSizeGripEnabled());

    dialog.setSizeGripEnabled(false);
    QVERIFY(!dialog.isSizeGripEnabled());

    dialog.showExtension(false);
    QVERIFY(dialog.extension() && !dialog.extension()->isVisible());
    QVERIFY(!dialog.isSizeGripEnabled());
#endif
}

QTEST_MAIN(tst_QDialog)
#include "tst_qdialog.moc"

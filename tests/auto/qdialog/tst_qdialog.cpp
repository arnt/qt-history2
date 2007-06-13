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
#include <qstyle.h>
#include <QVBoxLayout>
#include <QSizeGrip>

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
    void setVisible();

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
    QPoint oldPosition = testWidget->pos();

    // show
    ((DummyDialog*)testWidget)->showExtension( TRUE );
//     while ( testWidget->size() == dlgSize )
// 	qApp->processEvents();
    QTEST( testWidget->size(), "result"  );
    QCOMPARE(testWidget->pos(), oldPosition);

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

    //we need to show the buttons. Otherwise they won't get the focus
    push->show();
    pushTwo->show();
    pushThree->show();

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
    dialog.setSizeGripEnabled(true);
    QSizeGrip *sizeGrip = qFindChild<QSizeGrip *>(&dialog);
    QVERIFY(sizeGrip);

    dialog.showMaximized();
    QVERIFY(dialog.isMaximized());
    QVERIFY(dialog.isVisible());
#ifndef Q_WS_MAC
    QVERIFY(!sizeGrip->isVisible());
#endif

    dialog.showNormal();
    QVERIFY(!dialog.isMaximized());
    QVERIFY(dialog.isVisible());
    QVERIFY(sizeGrip->isVisible());

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
    dialog.setSizeGripEnabled(true);
    QSizeGrip *sizeGrip = qFindChild<QSizeGrip *>(&dialog);
    QVERIFY(sizeGrip);

    qApp->syncX();
    dialog.showFullScreen();
    QVERIFY(dialog.isFullScreen());
    QVERIFY(dialog.isVisible());
    QVERIFY(!sizeGrip->isVisible());

    qApp->syncX();
    dialog.showNormal();
    QVERIFY(!dialog.isFullScreen());
    QVERIFY(dialog.isVisible());
    QVERIFY(sizeGrip->isVisible());

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
    ToolDialog dialog(testWidget);
    testWidget->activateWindow();
    dialog.exec();
	if (testWidget->style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, testWidget)) {
#if defined(Q_WS_QWS)  && QT_VERSION < 0x040400
		QEXPECT_FAIL(0, "Qtopia Core has messed up WStyle_Tool  (task 126435)", Continue);
#endif
		QCOMPARE(dialog.wasActive(), true);
	} else {
		QCOMPARE(dialog.wasActive(), false);
	}
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
    QPointer<QSizeGrip> sizeGrip = qFindChild<QSizeGrip *>(&dialog);
    QVERIFY(sizeGrip);
    QVERIFY(sizeGrip->isVisible());
    QVERIFY(dialog.isSizeGripEnabled());

    dialog.setExtension(ext);
    QVERIFY(dialog.extension() && !dialog.extension()->isVisible());
    QVERIFY(dialog.isSizeGripEnabled());

    // normal show/hide sequence
    dialog.showExtension(true);
    QVERIFY(dialog.extension() && dialog.extension()->isVisible());
    QVERIFY(!dialog.isSizeGripEnabled());
    QVERIFY(!sizeGrip);

    dialog.showExtension(false);
    QVERIFY(dialog.extension() && !dialog.extension()->isVisible());
    QVERIFY(dialog.isSizeGripEnabled());
    sizeGrip = qFindChild<QSizeGrip *>(&dialog);
    QVERIFY(sizeGrip);
    QVERIFY(sizeGrip->isVisible());

    // show/hide sequence with interleaved size grip update
    dialog.showExtension(true);
    QVERIFY(dialog.extension() && dialog.extension()->isVisible());
    QVERIFY(!dialog.isSizeGripEnabled());
    QVERIFY(!sizeGrip);

    dialog.setSizeGripEnabled(false);
    QVERIFY(!dialog.isSizeGripEnabled());

    dialog.showExtension(false);
    QVERIFY(dialog.extension() && !dialog.extension()->isVisible());
    QVERIFY(!dialog.isSizeGripEnabled());

    dialog.setSizeGripEnabled(true);
    sizeGrip = qFindChild<QSizeGrip *>(&dialog);
    QVERIFY(sizeGrip);
    QVERIFY(sizeGrip->isVisible());
    sizeGrip->hide();
    dialog.hide();
    dialog.show();
    QVERIFY(!sizeGrip->isVisible());
#endif
}

void tst_QDialog::setVisible()
{
    QWidget topLevel;
    topLevel.show();

    QDialog *dialog = new QDialog;
    dialog->setLayout(new QVBoxLayout);
    dialog->layout()->addWidget(new QPushButton("dialog button"));

    QWidget *widget = new QWidget(&topLevel);
    widget->setLayout(new QVBoxLayout);
    widget->layout()->addWidget(dialog);

    QVERIFY(!dialog->isVisible());
    QVERIFY(!dialog->isHidden());

    widget->show();
    QVERIFY(dialog->isVisible());
    QVERIFY(!dialog->isHidden());

    widget->hide();
    dialog->hide();
    widget->show();
    QVERIFY(!dialog->isVisible());
    QVERIFY(dialog->isHidden());
}

QTEST_MAIN(tst_QDialog)
#include "tst_qdialog.moc"

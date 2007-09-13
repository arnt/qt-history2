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
#include <qfontinfo.h>


#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qtimer.h>

#include <qdialog.h>


//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qmenubar.h gui/widgets/qmenubar.cpp

class TstWidget;
class TstDialog;
QT_DECLARE_CLASS(QPushButton)

class tst_qmouseevent_modal : public QObject
{
    Q_OBJECT

public:
    tst_qmouseevent_modal();
    virtual ~tst_qmouseevent_modal();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void mousePressRelease();

private:
    TstWidget *w;
};

class TstWidget : public QWidget
{
    Q_OBJECT
public:
    TstWidget();
public slots:
    void buttonPressed();
public:
    QPushButton *pb;
    TstDialog *d;
};


class TstDialog : public QDialog
{
    Q_OBJECT
public:
    TstDialog( QWidget *mouseWidget, QWidget *parent, const char *name );
    int count() { return c; }
protected:
    void showEvent ( QShowEvent * );
public slots:
    void releaseMouse();
    void closeDialog();
private:
    QWidget *m;
    int c;
};

tst_qmouseevent_modal::tst_qmouseevent_modal()
{
}

tst_qmouseevent_modal::~tst_qmouseevent_modal()
{
}

void tst_qmouseevent_modal::initTestCase()
{
    w = new TstWidget;
    w->show();
}

void tst_qmouseevent_modal::cleanupTestCase()
{
    delete w;
    w = 0;
}

void tst_qmouseevent_modal::init()
{
}

void tst_qmouseevent_modal::cleanup()
{
}

/*
  Test for task 22500
*/
void tst_qmouseevent_modal::mousePressRelease()
{

    QVERIFY( !w->d->isVisible() );
    QVERIFY( w->d->count() == 0 );

    QTest::mousePress( w->pb, Qt::LeftButton );

    QVERIFY( !w->d->isVisible() );
    QVERIFY( w->d->count() == 1 );
    QVERIFY( !w->pb->isDown() );

    QTest::mousePress( w->pb, Qt::LeftButton );

    QVERIFY( !w->d->isVisible() );
    QVERIFY( w->d->count() == 2 );
    QVERIFY( !w->pb->isDown() );

    // With the buggy QWS mouse handling, the 3rd press would fail...

    QTest::mousePress( w->pb, Qt::LeftButton );

    QVERIFY( !w->d->isVisible() );
    QVERIFY( w->d->count() == 3 );
    QVERIFY( !w->pb->isDown() );

    QTest::mousePress( w->pb, Qt::LeftButton );

    QVERIFY( !w->d->isVisible() );
    QVERIFY( w->d->count() == 4 );
    QVERIFY( !w->pb->isDown() );
}


TstWidget::TstWidget()
{
    pb = new QPushButton( "Press me", this );
    pb->setObjectName("testbutton");
    QSize s = pb->sizeHint();
    pb->setGeometry( 5, 5, s.width(), s.height() );

    connect( pb, SIGNAL(pressed()), this, SLOT(buttonPressed()) );

//	QScrollBar *sb = new QScrollBar( Qt::Horizontal,  this );

//	sb->setGeometry( 5, pb->geometry().bottom() + 5, 100, sb->sizeHint().height() );

    d = new TstDialog( pb, this , 0 );
}

void TstWidget::buttonPressed()
{
    d->exec();
}

TstDialog::TstDialog( QWidget *mouseWidget, QWidget *parent, const char *name )
    :QDialog( parent )
{
    setObjectName(name);
    setModal(true);
    m = mouseWidget;
    c = 0;
}

void TstDialog::showEvent ( QShowEvent * )
{
    QTimer::singleShot(1, this, SLOT(releaseMouse()));
    QTimer::singleShot(100, this, SLOT(closeDialog()));
}

void TstDialog::releaseMouse()
{
    QTest::mouseRelease(m, Qt::LeftButton);
}

void TstDialog::closeDialog()
{
    if ( isVisible() ) {
	c++;
	accept();
    }
}

QTEST_MAIN(tst_qmouseevent_modal)
#include "tst_qmouseevent_modal.moc"

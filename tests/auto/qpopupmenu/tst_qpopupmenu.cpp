/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qglobal.h>
#ifdef Q_WS_MAC
#include <private/qt_mac_p.h>
#undef verify
#endif
#include <QtTest/QtTest>

#include <qapplication.h>
#include <qmessagebox.h>
#include <qmenubar.h>


#include <qdebug.h>


#include <q3popupmenu.h>
#include <qmainwindow.h>


//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3popupmenu.h

class tst_Q3PopupMenu : public QObject
{
    Q_OBJECT
public:
    tst_Q3PopupMenu();
    virtual ~tst_Q3PopupMenu();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void testAccels();
    void fontPropagation();
    void accel_data();
    void accel();
    void testItemParameter();

protected slots:
    void onExclItem();
    void onShiftItem();
    void onSpicy();
    void onSubItem();
    // Needed to slience QObject about non existant slot
    void dummySlot() {}
    void itemParameterChanged(int p = 0){itemParameter = p; }

private:
    QMainWindow *testWidget;
    Q3PopupMenu *popup;
    Q3PopupMenu *subPopup;
    bool excl, shft, spicy, subItem;
    int itemParameter;
};

Q_DECLARE_METATYPE(QKeySequence)

tst_Q3PopupMenu::tst_Q3PopupMenu()
{
}

tst_Q3PopupMenu::~tst_Q3PopupMenu()
{
}

void tst_Q3PopupMenu::initTestCase()
{
    // Create the test class

    testWidget = new QMainWindow(0);
    popup = new Q3PopupMenu( testWidget->menuBar(), "popup" );
    testWidget->menuBar()->insertItem( "menu&bar", popup );
    excl = FALSE;
    shft = FALSE;
    spicy = FALSE;
    subItem = FALSE;
    qApp->setMainWidget(testWidget);
    testWidget->resize( 200, 200 );
    testWidget->show();
    popup->insertItem( tr("Men&u"), this, SLOT(onShiftItem()) );
    popup->insertItem( tr("thing&!"), this, SLOT(onExclItem()) );
    popup->insertItem( tr("Hot && Spic&y" ), this, SLOT(onSpicy()) );

    subPopup = new Q3PopupMenu( popup, "subpopup" );
    subPopup->insertItem( "sub menu &item", this, SLOT(onSubItem()) );

    popup->insertItem( "&sub Popup", subPopup );
}

void tst_Q3PopupMenu::cleanupTestCase()
{
    delete testWidget;
}

void tst_Q3PopupMenu::init()
{
    QApplication::setActiveWindow(testWidget);
    QApplication::processEvents();
}

void tst_Q3PopupMenu::cleanup()
{
    QApplication::processEvents();
}
void tst_Q3PopupMenu::onExclItem()
{
    excl = TRUE;
}

void tst_Q3PopupMenu::onShiftItem()
{
    shft = TRUE;
}

void tst_Q3PopupMenu::onSpicy()
{
    spicy = TRUE;
}

void tst_Q3PopupMenu::onSubItem()
{
    subItem = TRUE;
}

void tst_Q3PopupMenu::testAccels()
{
#if !defined(Q_WS_MAC)
    QTest::keyClick( testWidget, Qt::Key_B, Qt::AltModifier );
    while (!popup->isVisible())
        QApplication::processEvents();
    QTest::keyClick( popup, Qt::Key_S );
    while (!subPopup->isVisible())
        QApplication::processEvents();
    QTest::keyClick( subPopup, Qt::Key_I );
    QVERIFY( subItem );
    QTest::keyClick( testWidget, Qt::Key_B, Qt::AltModifier );
    while (!popup->isVisible())
        QApplication::processEvents();
    QTest::keyClick( popup, 'U' );
    QVERIFY( shft );

    QTest::keyClick( testWidget, Qt::Key_B, Qt::AltModifier );
    QTest::keyClick( popup, '!' );
    QVERIFY( excl );

    QTest::keyClick( testWidget, Qt::Key_B, Qt::AltModifier );
    QTest::keyClick( popup, 'Y' );
    QVERIFY( spicy );
#else
    QSKIP("Mac OS X doesn't use mnemonics", SkipAll);
#endif

}

void tst_Q3PopupMenu::fontPropagation()
{
    QFont newfont = QFont( "times", 24 );
    QFont originalFont = popup->font();
    testWidget->setFont( QFont( "times", 24 ) );
    QVERIFY( !popup->ownFont() );
    QVERIFY( !(popup->font() == newfont) );
    QApplication::setFont( newfont, TRUE );
    QVERIFY( !popup->ownFont() );
    QVERIFY( popup->font() == newfont );
}

void tst_Q3PopupMenu::accel_data()
{
    QTest::addColumn<QKeySequence>("accelerator");
    QTest::addColumn<int>("id");
    QTest::addColumn<QString>("accelString");

#ifndef Q_WS_MAC
    QTest::newRow("simple_accel") << QKeySequence("CTRL+C") << 1 << QString("Ctrl+C");
    QTest::newRow("complex_accel") << QKeySequence("CTRL+ALT+SHIFT+T") << 2 << QString("Ctrl+Alt+Shift+T");
#else
    QTest::newRow("simple_accel") << QKeySequence("CTRL+C") << 1
                               << QString(QChar(kCommandUnicode) + 'C');
    QTest::newRow("complex_accel") << QKeySequence("CTRL+ALT+SHIFT+T") << 2
                                << QString(QChar(kOptionUnicode) + QString(QChar(kShiftUnicode))
                                   + QString(QChar(kCommandUnicode)) + 'T');
#endif
}

void tst_Q3PopupMenu::accel()
{
    QFETCH(QKeySequence, accelerator);
    QFETCH(int, id);
    QFETCH(QString, accelString);

    popup->insertItem("Dummy item", this, SLOT(dummySlot()), accelerator, id);
    QCOMPARE(accelString, (QString)popup->accel(id));
}

void tst_Q3PopupMenu::testItemParameter()
{
#if !defined(Q_WS_MAC)
    itemParameter = 0;
    int id = popup->insertItem( tr("&ItemParameter"), this, SLOT(itemParameterChanged(int)));
    popup->setItemParameter(id, 17);
    QTest::keyClick( testWidget, Qt::Key_B, Qt::AltModifier );
    while (!popup->isVisible())
        QApplication::processEvents();
    QTest::keyClick( popup, 'I' );
    QCOMPARE(itemParameter, 17);
#else
    QSKIP("Mac OS X doesn't use mnemonics", SkipAll);
#endif
}


QTEST_MAIN(tst_Q3PopupMenu)
#include "tst_qpopupmenu.moc"

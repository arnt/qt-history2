/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if QT_VERSION < 0x040200
QTEST_NOOP_MAIN
#else

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qsystemtrayicon.h>
#include <qmenu.h>

//TESTED_CLASS=QSystemTrayIcon
//TESTED_FILES=gui/util/qsystemtrayicon.h gui/util/qsystemtrayicon.cpp

class tst_QSystemTrayIcon: public QObject
{
Q_OBJECT

public:
    tst_QSystemTrayIcon();
    virtual ~tst_QSystemTrayIcon();

private slots:
    void getSetCheck();
    void showHide();
    void showMessage();
    void supportsMessages();
    void lastWindowClosed();
};

tst_QSystemTrayIcon::tst_QSystemTrayIcon()
{
}

tst_QSystemTrayIcon::~tst_QSystemTrayIcon()
{
}

// Testing get/set functions
void tst_QSystemTrayIcon::showHide()
{
    QSystemTrayIcon icon;
    icon.setIcon(QIcon("icons/icon.png"));
    icon.show();
    icon.setIcon(QIcon("icons/icon.png"));
    icon.hide();
}

// Testing get/set functions
void tst_QSystemTrayIcon::showMessage()
{
    QSystemTrayIcon icon;
    icon.setIcon(QIcon("icons/icon.png"));

    icon.showMessage("Title", "Messagecontents");
    icon.showMessage("Title", "Messagecontents", QSystemTrayIcon::NoIcon);
    icon.showMessage("Title", "Messagecontents", QSystemTrayIcon::Warning);
    icon.showMessage("Title", "Messagecontents", QSystemTrayIcon::Critical);

    icon.show();
    icon.showMessage("Title", "Messagecontents");
    icon.showMessage("Title", "Messagecontents", QSystemTrayIcon::NoIcon);
    icon.showMessage("Title", "Messagecontents", QSystemTrayIcon::Warning);
    icon.showMessage("Title", "Messagecontents", QSystemTrayIcon::Critical);
}

// Testing get/set functions
void tst_QSystemTrayIcon::getSetCheck()
{
    QSystemTrayIcon icon;
    QCOMPARE(true, icon.toolTip().isEmpty());
    icon.setToolTip("testToolTip");
    QCOMPARE(true, "testToolTip" == icon.toolTip());

    QCOMPARE(true, icon.icon().isNull());
    icon.setIcon(QIcon("icons/icon.png"));
    QCOMPARE(false, icon.icon().isNull());

    QMenu menu;
    QCOMPARE(true, icon.contextMenu() == 0);
    icon.setContextMenu(&menu);
    QCOMPARE(false, icon.contextMenu() == 0);
}

void tst_QSystemTrayIcon::supportsMessages()
{
    // This is rather idiotic, but it does improve coverage
#if !defined(Q_WS_QWS)
    QCOMPARE(QSystemTrayIcon::supportsMessages(), true );
#else
    QCOMPARE(QSystemTrayIcon::supportsMessages(), false );
#endif

}

void tst_QSystemTrayIcon::lastWindowClosed()
{
    QSignalSpy spy(qApp, SIGNAL(lastWindowClosed()));
    QWidget window;
    QSystemTrayIcon icon;
    icon.setIcon(QIcon("whatever.png"));
    icon.show();
    window.show();
    QTimer::singleShot(500, &window, SLOT(close()));
    QTimer::singleShot(1000, qApp, SLOT(quit())); // in case the test fails
    qApp->exec();
    QVERIFY(spy.count() == 1);
}

QTEST_MAIN(tst_QSystemTrayIcon)
#include "tst_qsystemtrayicon.moc"
#endif

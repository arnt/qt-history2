/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <q3textbrowser.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <q3mimefactory.h>

#include <qtextbrowser.h>
#include <qdesktopwidget.h>

#ifdef Q_WS_X11
QT_BEGIN_NAMESPACE
extern void qt_x11_wait_for_window_manager( QWidget* w );
QT_END_NAMESPACE
#endif

//TESTED_CLASS=
//TESTED_FILES=compat/text/qtextbrowser.h compat/text/qtextbrowser.cpp

class tst_Q3TextBrowser : public QObject
{
    Q_OBJECT
public:
    tst_Q3TextBrowser();
    virtual ~tst_Q3TextBrowser();

public slots:
    void initTestCase();
    void cleanupTestCase();
private slots:
    void setFont();
private:
    Q3TextBrowser *testWidget;
};

tst_Q3TextBrowser::tst_Q3TextBrowser()
{
    Q3MimeSourceFactory::defaultFactory()->addFilePath(".");
}

tst_Q3TextBrowser::~tst_Q3TextBrowser()
{
}

void tst_Q3TextBrowser::initTestCase()
{
    testWidget = new Q3TextBrowser;
    testWidget->setParent(0, Qt::WX11BypassWM);
    testWidget->move(QApplication::desktop()->availableGeometry().topLeft()+QPoint(5, 5));

    testWidget->show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(testWidget);
#endif
    qApp->processEvents();
}

void tst_Q3TextBrowser::cleanupTestCase()
{
    delete testWidget;
}

void tst_Q3TextBrowser::setFont()
{
    QFont f("Courier", 6);
    testWidget->setFont(f);
    QVERIFY(testWidget->font() == f);
}

QTEST_MAIN(tst_Q3TextBrowser)
#include "tst_q3textbrowser.moc"


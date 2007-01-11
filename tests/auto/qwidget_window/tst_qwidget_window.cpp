/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QtGui>
#include <qeventloop.h>
#include <qlist.h>

#include <qlistwidget.h>

class tst_QWidget_window : public QWidget
{
    Q_OBJECT

public:
    tst_QWidget_window(){};

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void tst_move_show();
    void tst_show_move();
    void tst_show_move_hide_show();

    void tst_resize_show();
    void tst_show_resize();
    void tst_show_resize_hide_show();

};

void tst_QWidget_window::initTestCase()
{
}

void tst_QWidget_window::cleanupTestCase()
{
}

void tst_QWidget_window::tst_move_show()
{
    QWidget w;
    w.move(100, 100);
    w.show();
    QCOMPARE(w.pos(), QPoint(100, 100));
//    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void tst_QWidget_window::tst_show_move()
{
    QWidget w;
    w.show();
    w.move(100, 100);
    QCOMPARE(w.pos(), QPoint(100, 100));
//    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void tst_QWidget_window::tst_show_move_hide_show()
{
    QWidget w;
    w.show();
    w.move(100, 100);
    w.hide();
    w.show();
    QCOMPARE(w.pos(), QPoint(100, 100));
//    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void tst_QWidget_window::tst_resize_show()
{
    QWidget w;
    w.resize(100, 100);
    w.show();
    QCOMPARE(w.size(), QSize(100, 100));
//    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void tst_QWidget_window::tst_show_resize()
{
    QWidget w;
    w.show();
    w.resize(100, 100);
    QCOMPARE(w.size(), QSize(100, 100));
//    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void tst_QWidget_window::tst_show_resize_hide_show()
{
    QWidget w;
    w.show();
    w.resize(100, 100);
    w.hide();
    w.show();
    QCOMPARE(w.size(), QSize(100, 100));
//    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

QTEST_MAIN(tst_QWidget_window)
#include "tst_qwidget_window.moc"

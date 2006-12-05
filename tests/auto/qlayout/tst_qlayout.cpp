/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qboxlayout.h>
#include <qmenubar.h>
#include <QtGui/QFrame>
#include <QtGui/QWindowsStyle>

//TESTED_CLASS=
//TESTED_FILES=qlayout.h

class tst_QLayout : public QObject
{
Q_OBJECT

public:
    tst_QLayout();
    virtual ~tst_QLayout();

private slots:
    void getSetCheck();
    void geometry();
};

tst_QLayout::tst_QLayout()
{
}

tst_QLayout::~tst_QLayout()
{
}

// Testing get/set functions
void tst_QLayout::getSetCheck()
{
    QBoxLayout obj1(QBoxLayout::LeftToRight);
    // QWidget * QLayout::menuBar()
    // void QLayout::setMenuBar(QWidget *)
    QMenuBar *var1 = new QMenuBar();
    obj1.setMenuBar(var1);
    QCOMPARE(var1, obj1.menuBar());
    obj1.setMenuBar((QWidget *)0);
    QCOMPARE((QWidget *)0, obj1.menuBar());
    delete var1;
}

class SizeHinterFrame : public QFrame
{
public:
    SizeHinterFrame(const QSize &s) 
    : QFrame(0), sh(s) {
        setFrameStyle(QFrame::Box | QFrame::Plain);
    }



    void setSizeHint(const QSize &s) { sh = s; }
    QSize sizeHint() const { return sh; }

private:
    QSize sh;
};


void tst_QLayout::geometry()
{
    // For QWindowsStyle we know that QWidgetItem::geometry() and QWidget::geometry()
    // should be the same.
    QApplication::setStyle(new QWindowsStyle);
    QWidget w;
    QVBoxLayout layout(&w);
    SizeHinterFrame widget(QSize(100,100));
    layout.addWidget(&widget);
    QLayoutItem *item = layout.itemAt(0);
    w.show();
    QApplication::processEvents();
    QCOMPARE(item->geometry().size(), QSize(100,100));

    widget.setMinimumSize(QSize(110,110));
    QCOMPARE(item->geometry().size(), QSize(110,110));

    widget.setMinimumSize(QSize(0,0));
    widget.setMaximumSize(QSize(90,90));
    widget.setSizeHint(QSize(100,100));
    QCOMPARE(item->geometry().size(), QSize(90,90));
}


QTEST_MAIN(tst_QLayout)
#include "tst_qlayout.moc"

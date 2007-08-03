/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui>

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qboxlayout.cpp gui/kernel/qboxlayout.h

class tst_QBoxLayout : public QObject
{
    Q_OBJECT

public:
    tst_QBoxLayout();
    virtual ~tst_QBoxLayout();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void insertSpacerItem();
};


tst_QBoxLayout::tst_QBoxLayout()
{
}

tst_QBoxLayout::~tst_QBoxLayout()
{
}

void tst_QBoxLayout::initTestCase()
{
}

void tst_QBoxLayout::cleanupTestCase()
{
}

void tst_QBoxLayout::init()
{
}

void tst_QBoxLayout::cleanup()
{
}

void tst_QBoxLayout::insertSpacerItem()
{
    QWidget *window = new QWidget;

    QSpacerItem *spacer1 = new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSpacerItem *spacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);

    QBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(new QLineEdit("Foooooooooooooooooooooooooo"));
    layout->addSpacerItem(spacer1);
    layout->addWidget(new QLineEdit("Baaaaaaaaaaaaaaaaaaaaaaaaar"));
    layout->insertSpacerItem(0, spacer2);
    window->setLayout(layout);

    QVERIFY(layout->itemAt(0) == spacer2);
    QVERIFY(layout->itemAt(2) == spacer1);

    window->show();
}

QTEST_MAIN(tst_QBoxLayout)
#include "tst_qboxlayout.moc"

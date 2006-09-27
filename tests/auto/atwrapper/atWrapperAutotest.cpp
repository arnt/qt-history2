/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include "atWrapper.h"
#include <QApplication>

class atWrapperAutotest: public QObject
{

Q_OBJECT

public slots:
    void init();

private slots:
    void runTest();
};

void atWrapperAutotest::init()
{
#ifndef Q_OS_IRIX
    QDir::setCurrent(SRCDIR);
#endif
}

void atWrapperAutotest::runTest()
{

    //QApplication app(argc, argv);

    atWrapper wrapper;
    if (!wrapper.runAutoTests())
	QSKIP("Arthur not tested on this machine", SkipAll);
    QVERIFY(true);
}

QTEST_MAIN(atWrapperAutotest)
#include "atWrapperAutotest.moc"

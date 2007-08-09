/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifdef Q_WS_QWS

//TESTED_CLASS=QWSInputMethod
//TESTED_FILES=gui/embedded/qwindowsystem_qws.h gui/embedded/qwindowsystem_qws.cpp

#include <QWSInputMethod>

class tst_QWSInputMethod : public QObject
{
    Q_OBJECT

public:
    tst_QWSInputMethod() {}
    virtual ~tst_QWSInputMethod() {}

private slots:
    void createSubClass();
};

/*
    Dummy class just to make sure the QWSInputMethod header compiles.
*/
class MyInputMethod : public QWSInputMethod
{
public:
    MyInputMethod() : QWSInputMethod() {}
    ~MyInputMethod() {}
    bool filter(int, int, int, bool, bool) { return true; }
    bool filter(const QPoint &, int, int) { return true; }
    void mouseHandler(int, int) {}
    void queryResponse(int, const QVariant &) {}
    void reset() {}

    void updateHandler(int) {}
};

void tst_QWSInputMethod::createSubClass()
{
    MyInputMethod im;
}

QTEST_MAIN(tst_QWSInputMethod)

#include "tst_qwsinputmethod.moc"

#else // Q_WS_QWS
QTEST_NOOP_MAIN
#endif

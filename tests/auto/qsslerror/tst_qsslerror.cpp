/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsslerror.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>

class tst_QSslError : public QObject
{
    Q_OBJECT

public:
    tst_QSslError();
    virtual ~tst_QSslError();

    static void enterLoop(int secs)
    {
        ++loopLevel;
        QTestEventLoop::instance().enterLoop(secs);
        --loopLevel;
    }
    static void exitLoop()
    {
        // Safe exit - if we aren't in an event loop, don't
        // exit one.
        if (loopLevel > 0)
            QTestEventLoop::instance().exitLoop();
    }
    static bool timeout()
    {
        return QTestEventLoop::instance().timeout();
    }

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL
private slots:
    void constructing();
#endif
    
private:
    static int loopLevel;
};

int tst_QSslError::loopLevel = 0;

tst_QSslError::tst_QSslError()
{
}

tst_QSslError::~tst_QSslError()
{

}

void tst_QSslError::initTestCase_data()
{
}

void tst_QSslError::init()
{
}

void tst_QSslError::cleanup()
{
}

#ifndef QT_NO_OPENSSL

void tst_QSslError::constructing()
{
    QSslError error;
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslError)
#include "tst_qsslerror.moc"

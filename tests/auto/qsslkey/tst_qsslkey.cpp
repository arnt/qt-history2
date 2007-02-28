/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsslkey.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>

class tst_QSslKey : public QObject
{
    Q_OBJECT

public:
    tst_QSslKey();
    virtual ~tst_QSslKey();

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL

private slots:
    void constructing();

#endif
};

tst_QSslKey::tst_QSslKey()
{
}

tst_QSslKey::~tst_QSslKey()
{

}

void tst_QSslKey::initTestCase_data()
{
}

void tst_QSslKey::init()
{
}

void tst_QSslKey::cleanup()
{
}

#ifndef QT_NO_OPENSSL

void tst_QSslKey::constructing()
{
    QSslKey key;
}

#endif

QTEST_MAIN(tst_QSslKey)
#include "tst_qsslkey.moc"

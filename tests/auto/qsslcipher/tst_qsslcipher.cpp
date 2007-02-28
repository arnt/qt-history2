/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsslcipher.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>

class tst_QSslCipher : public QObject
{
    Q_OBJECT

public:
    tst_QSslCipher();
    virtual ~tst_QSslCipher();

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL

private slots:
    void constructing();

#endif
};

tst_QSslCipher::tst_QSslCipher()
{
}

tst_QSslCipher::~tst_QSslCipher()
{

}

void tst_QSslCipher::initTestCase_data()
{
}

void tst_QSslCipher::init()
{
}

void tst_QSslCipher::cleanup()
{
}

#ifndef QT_NO_OPENSSL

void tst_QSslCipher::constructing()
{
    QSslCipher cipher;
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslCipher)
#include "tst_qsslcipher.moc"

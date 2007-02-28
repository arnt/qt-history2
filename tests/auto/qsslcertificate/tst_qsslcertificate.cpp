/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsslcertificate.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>

class tst_QSslCertificate : public QObject
{
    Q_OBJECT

public:
    tst_QSslCertificate();
    virtual ~tst_QSslCertificate();

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL
private slots:
    void constructing();
#endif
};

tst_QSslCertificate::tst_QSslCertificate()
{
}

tst_QSslCertificate::~tst_QSslCertificate()
{
}

void tst_QSslCertificate::initTestCase_data()
{
}

void tst_QSslCertificate::init()
{
}

void tst_QSslCertificate::cleanup()
{
}

#ifndef QT_NO_OPENSSL

void tst_QSslCertificate::constructing()
{
    QSslCertificate certificate;
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslCertificate)
#include "tst_qsslcertificate.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_QCryptographicHash : public QObject
{
    Q_OBJECT
private slots:
    void sha1();
};
#include <QtCore>

void tst_QCryptographicHash::sha1()
{
//  SHA1("abc") =
//      A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
    QCOMPARE(QCryptographicHash::hash("abc", QCryptographicHash::Sha1).toHex().toUpper(),
             QByteArray("A9993E364706816ABA3E25717850C26C9CD0D89D"));
             
//  SHA1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") =
//      84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
    QCOMPARE(QCryptographicHash::hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                                      QCryptographicHash::Sha1).toHex().toUpper(),
             QByteArray("84983E441C3BD26EBAAE4AA1F95129E5E54670F1"));
             
//  SHA1(A million repetitions of "a") =
//      34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
    QByteArray as;
    for (int i = 0; i < 1000000; ++i)
        as += 'a';
    QCOMPARE(QCryptographicHash::hash(as, QCryptographicHash::Sha1).toHex().toUpper(), 
             QByteArray("34AA973CD4C4DAA4F61EEB2BDBAD27316534016F"));
}


QTEST_MAIN(tst_QCryptographicHash)
#include "tst_qcryptographichash.moc"

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qstring.h>

#include <ostream>
#include <sstream>

class tst_QStl: public QObject
{
    Q_OBJECT

private slots:
    void streaming_data();
    void streaming();

    void concatenate();
};


static inline std::ostream &operator<<(std::ostream &out, const QString &string)
{
    out << string.toLocal8Bit().constData();
    return out;
}

void tst_QStl::streaming_data()
{
    QTest::addColumn<QString>("str");

    QTest::newRow("hello") << "hello";
    QTest::newRow("empty") << "";
}

void tst_QStl::streaming()
{
    QFETCH(QString, str);

    std::ostringstream buf;
    buf << str;

    std::string result = buf.str();

    QCOMPARE(QString::fromLatin1(result.data()), str);
}

void tst_QStl::concatenate()
{
    std::ostringstream buf;
    buf << QLatin1String("Hello ") << QLatin1String("World");

    QCOMPARE(QString::fromLatin1(buf.str().data()), QString("Hello World"));
}


QTEST_APPLESS_MAIN(tst_QStl)
#include "tst_qstl.moc"

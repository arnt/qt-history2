/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include "PushBaseliner.h"

#include "qxmlquery.h"
#include "qxmlname.h"

/*!
 \class tst_QXmlQuery
 \internal
 \since 4.4
 \brief Tests class QXmlQuery.
This test is not intended for testing the engine, but the functionality specific
 to the QXmlQuery class.

 In other words, if you have an engine bug; don't add it here because it won't be
 tested properly. Instead add it to the test suite.

 */
class tst_QXmlQuery : public QObject
{
    Q_OBJECT

public:
    inline tst_QXmlQuery() : generatedBaselines(0)
    {
    }

private Q_SLOTS:
    void defaultConstructor() const;
    void isValid() const;
    void hasEvaluationError() const;
    void messageHandler() const;
    void evaluateToPushCallback();
    void evaluateToPushCallback_data() const;
    void createName() const;
    void createName_data() const;
    void bindQObject() const;
    void bindQObject_data();
    void checkGeneratedBaselines() const;

    // TODO bind variables before and after setQuery().
    // TODO 1) setQuery with validquery, 2) setQUery with invalid query; 3) check states
    // TODO Call 1) setQuery 2) call isValid() with valid & invalid query. isValid() must trigger query compilation.
    // TODO Call 1) setQuery & evaluate) 2) copy 3) change variable 4) evaluate
    // TODO call all URI resolving functions where 1) the URI resolver return a null QUrl(); 2) resolves into valid, existing URI, 3) invalid, non-existing URI.

private:
    static QStringList queries();
    int generatedBaselines;
};

QStringList tst_QXmlQuery::queries()
{
    QDir dir;
    dir.cd(QLatin1String("../cli/queries/"));

    return dir.entryList(QStringList(QLatin1String("*.xq")));
}

void tst_QXmlQuery::defaultConstructor() const
{
    /* Allocate instance in different orders. */
    {
        QXmlQuery query;
    }

    {
        QXmlQuery query1;
        QXmlQuery query2;
    }

    {
        QXmlQuery query1;
        QXmlQuery query2;
        QXmlQuery query3;
    }
}

void tst_QXmlQuery::isValid() const
{
    /* Check default value. */
    QXmlQuery query;
    QVERIFY(!query.isValid());
}

void tst_QXmlQuery::hasEvaluationError() const
{
    /* Check default value. */
    QXmlQuery query;
    QVERIFY(query.hasEvaluationError());
}

void tst_QXmlQuery::messageHandler() const
{
    /* Check default value. */
    QXmlQuery query;
    QCOMPARE(query.messageHandler(), QAbstractMessageHandler::Ptr());
}

void tst_QXmlQuery::evaluateToPushCallback()
{
    QFETCH(QString, inputQuery);

    if(inputQuery == QLatin1String("wrongArity.xq"))
        QSKIP("wrongArity.xq crashes, some NamePool problem.", SkipSingle);
        
    QFile queryFile(QLatin1String("../cli/queries/") + inputQuery);
    QVERIFY(queryFile.open(QIODevice::ReadOnly));

    QXmlQuery query;
    query.setQuery(&queryFile);
    
    /* We read all the queries, and some of them are invalid. However, we
     * only wants those that compile. */
    if(!query.isValid())
        return;
    
    QString produced;
    QTextStream stream(&produced, QIODevice::WriteOnly);
    PushBaseliner push(stream, query);
    query.evaluateToPushCallback(&push);

    const QString baselineName(QLatin1String("pushBaselines/") + inputQuery.left(inputQuery.length() - 2) + QString::fromLatin1("ref")); 
    QFile baseline(baselineName);

    if(baseline.exists())
    {
        QVERIFY(baseline.open(QIODevice::ReadOnly));
        QCOMPARE(produced, QString::fromUtf8(baseline.readAll()));
    }
    else
    {
        QVERIFY(baseline.open(QIODevice::WriteOnly));
        /* This is intentionally a warning, don't remove it. */
        qWarning() << "Generated baseline for:" << baselineName;
        ++generatedBaselines;

        baseline.write(produced.toUtf8());
    }
}

void tst_QXmlQuery::evaluateToPushCallback_data() const
{
    QTest::addColumn<QString>("inputQuery");

    const QStringList qs(queries());
    QVERIFY2(qs.size() > 0, "Failed to locate query files, something is wrong with the setup.");

    for(int i = 0; i < qs.size(); ++i)
        QTest::newRow(qs.at(i).toUtf8().constData()) << qs.at(i);
}

Q_DECLARE_METATYPE(QXmlQuery)
void tst_QXmlQuery::createName() const
{
    QFETCH(QString, namespaceURI);
    QFETCH(QString, localName);
    QFETCH(QString, prefix);
    QFETCH(QXmlQuery, query);

    const QXmlName name(query.createName(localName, namespaceURI, prefix));

    QCOMPARE(name.namespaceUri(query), namespaceURI);
    QCOMPARE(name.localName(query), localName);
    QCOMPARE(name.prefix(query), prefix);
}

/*!
  \internal

 Below we use the same QXmlQuery instance. This means the same name pool
 is used.
 */
void tst_QXmlQuery::createName_data() const
{
    QTest::addColumn<QString>("namespaceURI");
    QTest::addColumn<QString>("localName");
    QTest::addColumn<QString>("prefix");
    QTest::addColumn<QXmlQuery>("query");

    QXmlQuery query;
    QTest::newRow("Basic test")
                    << QString::fromLatin1("http://example.com/Namespace1")
                    << QString::fromLatin1("localName1")
                    << QString::fromLatin1("prefix1")
                    << query;

    QTest::newRow("Same namespace & prefix as before, different local name.")
                    << QString::fromLatin1("http://example.com/Namespace1")
                    << QString::fromLatin1("localName2")
                    << QString::fromLatin1("prefix1")
                    << query;

    QTest::newRow("Same namespace & local name as before, different prefix.")
                    << QString::fromLatin1("http://example.com/Namespace1")
                    << QString::fromLatin1("localName2")
                    << QString::fromLatin1("prefix2")
                    << query;

    QTest::newRow("No prefix")
                    << QString::fromLatin1("http://example.com/Namespace2")
                    << QString::fromLatin1("localName3")
                    << QString()
                    << query;
}

class CustomQObject : public QObject
{
    Q_OBJECT
public:
    CustomQObject(QObject *const p) : QObject(p)
    {
    }
};

void tst_QXmlQuery::bindQObject() const
{
    QFETCH(QObject *, inObject);
    QFETCH(QString, expectedOut);

    QXmlQuery query;
    query.bindVariable(QLatin1String("qobject"), qVariantFromValue(inObject));
    query.setQuery(QLatin1String("declare variable $qobject external;"
                                 "$qobject"));

    QByteArray out;
    QBuffer buff(&out);
    QVERIFY(buff.open(QIODevice::WriteOnly));
    query.serialize(&buff);
    buff.close();

    QCOMPARE(QString::fromUtf8(out), expectedOut);
}

void tst_QXmlQuery::bindQObject_data()
{
    QTest::addColumn<QString>("expectedOut");
    QTest::addColumn<QObject *>("inObject");

    {
        QTest::newRow("Simple, unamed QObject")
            << QString::fromLatin1("<QObject objectName=\"\"/>")
            << new QObject(this);
    }
    
    {
        QObject *const qo = new QObject(this);
        qo->setObjectName(QLatin1String("QObjectName"));

        QTest::newRow("Simple, named QObject")
            << QString::fromLatin1("<QObject objectName=\"QObjectName\"/>")
            << qo;
    }

    {
        QObject *const p = new QObject(this);
        QObject *const qo = new QObject(p);
        qo->setObjectName(QLatin1String("QObjectName"));

        QTest::newRow("Simple, named QObject, with parent")
            << QString::fromLatin1("<QObject objectName=\"QObjectName\"/>")
            << qo;
    }

    {
        QObject *const p = new QObject(this);
        QObject *const qo = new QObject(p);

        p->setObjectName(QLatin1String("Parent"));
        qo->setObjectName(QLatin1String("Child"));

        QTest::newRow("Simple, named QObject, with named child")
            << QString::fromLatin1("<QObject objectName=\"Parent\"><QObject objectName=\"Child\"/></QObject>")
            << p;
    }

    {
        QObject *const p = new QObject(this);
        QObject *const qo = new QObject(p);
        QObject *const cqo = new CustomQObject(p);

        p->setObjectName(QLatin1String("Parent"));
        qo->setObjectName(QLatin1String("Child"));
        cqo->setObjectName(QLatin1String("CustomChild"));

        QTest::newRow("Simple, named QObject, with parent")
            << QString::fromLatin1("<QObject objectName=\"Parent\"><QObject objectName=\"Child\"/><CustomQObject objectName=\"CustomChild\"/></QObject>")
            << p;
    }
}

/*!
  If baselines were generated, we flag it as a failure such that it gets attention.
 */
void tst_QXmlQuery::checkGeneratedBaselines() const
{
    QCOMPARE(generatedBaselines, 0);
}

QTEST_MAIN(tst_QXmlQuery)

#include "tst_qxmlquery.moc"

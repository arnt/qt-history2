/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QXmlDefaultHandler>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QtTest>
#include <QtDebug>

//TESTED_CLASS=QXmlInputSource
//TESTED_FILES=xml/qxml.h xml/qxml.cpp

class tst_QXmlInputSource : public QObject
{
    Q_OBJECT

private slots:
    void reset() const;
    void resetSimplified() const;
};

/*!
  \internal
  \since 4.4

  See task 166278.
 */
void tst_QXmlInputSource::reset() const
{
    const QString input(QString::fromLatin1("<element attribute1='value1' attribute2='value2'/>"));
   
    QXmlSimpleReader reader;
    QXmlDefaultHandler handler;
    reader.setContentHandler(&handler);

    QXmlInputSource source;
    source.setData(input);

    QCOMPARE(source.data(), input);

    source.reset();
    QCOMPARE(source.data(), input);

    source.reset();
    QVERIFY(reader.parse(source));
    source.reset();
    QCOMPARE(source.data(), input);
}

/*!
  \internal
  \since 4.4

  See task 166278.
 */
void tst_QXmlInputSource::resetSimplified() const
{
    const QString input(QString::fromLatin1("<element/>"));
   
    QXmlSimpleReader reader;

    QXmlInputSource source;
    source.setData(input);

    QVERIFY(reader.parse(source));
    source.reset();
    QCOMPARE(source.data(), input);
}

QTEST_MAIN(tst_QXmlInputSource)
#include "tst_qxmlinputsource.moc"

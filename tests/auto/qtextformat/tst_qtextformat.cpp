/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qsettings.h>
#include <qtextformat.h>

//TESTED_CLASS=
//TESTED_FILES=qtextformat.h

class tst_QTextFormat : public QObject
{
Q_OBJECT

private slots:
    void getSetCheck();
    void defaultAlignment();
    void testQTextCharFormat() const;
};

/*! \internal
  This (used to) trigger a crash in:
 
    QDataStream &operator>>(QDataStream &stream, QTextFormat &fmt)

  which is most easily produced through QSettings.
 */
void tst_QTextFormat::testQTextCharFormat() const
{
    QSettings settings("test", "test");
    QTextCharFormat test;

    settings.value("test", test);
}

// Testing get/set functions
void tst_QTextFormat::getSetCheck()
{
    QTextFormat obj1;
    // int QTextFormat::objectIndex()
    // void QTextFormat::setObjectIndex(int)
    obj1.setObjectIndex(0);
    QCOMPARE(0, obj1.objectIndex());
    obj1.setObjectIndex(INT_MIN);
    QCOMPARE(INT_MIN, obj1.objectIndex());
    obj1.setObjectIndex(INT_MAX);
    QCOMPARE(INT_MAX, obj1.objectIndex());
}

void tst_QTextFormat::defaultAlignment()
{
    QTextBlockFormat fmt;
    QVERIFY(!fmt.hasProperty(QTextFormat::BlockAlignment));
    QCOMPARE(fmt.intProperty(QTextFormat::BlockAlignment), 0);
    QVERIFY(fmt.alignment() == Qt::AlignLeft);
}

QTEST_MAIN(tst_QTextFormat)
#include "tst_qtextformat.moc"

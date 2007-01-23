/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

class tst_QDebug: public QObject
{
    Q_OBJECT
private slots:
    void assignment() const;
    void warningWithoutDebug() const;
    void criticalWithoutDebug() const;
};

void tst_QDebug::assignment() const
{
    QDebug debug1(QtDebugMsg);
    QDebug debug2(QtWarningMsg);

    QTest::ignoreMessage(QtDebugMsg, "foo ");
    QTest::ignoreMessage(QtWarningMsg, "bar 1 2 ");

    debug1 << "foo";
    debug2 << "bar";
    debug1 = debug2;
    debug1 << "1";
    debug2 << "2";
}

static QtMsgType s_msgType;
static const char *s_msg;

static void myMessageHandler(QtMsgType type, const char *msg)
{
    s_msg = msg;
    s_msgType = type;
}

/*! \internal
  The qWarning() stream should be usable even if QT_NO_DEBUG is defined.
 */
void tst_QDebug::warningWithoutDebug() const
{
    qInstallMsgHandler(myMessageHandler);
    qWarning() << "A qWarning() message";
    QCOMPARE(s_msgType, QtWarningMsg);
    QCOMPARE(QString::fromLatin1(s_msg), QString::fromLatin1("A qWarning() message "));
    qInstallMsgHandler(0);
}

/*! \internal
  The qCritical() stream should be usable even if QT_NO_DEBUG is defined.
 */
void tst_QDebug::criticalWithoutDebug() const
{
    qInstallMsgHandler(myMessageHandler);
    qCritical() << "A qCritical() message";
    QCOMPARE(s_msgType, QtCriticalMsg);
    QCOMPARE(QString::fromLatin1(s_msg), QString::fromLatin1("A qCritical() message "));
    qInstallMsgHandler(0);
}

QTEST_MAIN(tst_QDebug);
#include "tst_qdebug.moc"

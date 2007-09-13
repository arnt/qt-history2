/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

QT_USE_NAMESPACE

#if defined(QT_NO_EXCEPTIONS)
    QTEST_NOOP_MAIN
#else
class tst_ExceptionSafety: public QObject
{
    Q_OBJECT
private slots:
    void exceptionInSlot();
};

class Emitter: public QObject
{
    Q_OBJECT
public:
    inline void emitTestSignal() { emit testSignal(); }
signals:
    void testSignal();
};

class ExceptionThrower: public QObject
{
    Q_OBJECT
public slots:
    void thrower() { throw 5; }
};

// connect a signal to a slot that throws an exception
// run this through valgrind to make sure it doesn't corrupt
void tst_ExceptionSafety::exceptionInSlot()
{
    Emitter emitter;
    ExceptionThrower thrower;

    connect(&emitter, SIGNAL(testSignal()), &thrower, SLOT(thrower()));

    try {
        emitter.emitTestSignal();
    } catch (int i) {
        QCOMPARE(i, 5);
    }
}

QTEST_MAIN(tst_ExceptionSafety)
#include "tst_exceptionsafety.moc"
#endif // QT_NO_EXCEPTIONS

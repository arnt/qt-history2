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

#include <qobject.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/kernel/qobject.h corelib/kernel/qobject.cpp

class tst_QObjectPerformance : public QObject
{
    Q_OBJECT

public:

private slots:
    void emitToManyReceivers();
};

class SimpleSenderObject : public QObject
{
    Q_OBJECT

signals:
    void signal();

public:
    void emitSignal()
    {
        emit signal();
    }
};

class SimpleReceiverObject : public QObject
{
    Q_OBJECT

public slots:
    void slot()
    {
    }
};

void tst_QObjectPerformance::emitToManyReceivers()
{
    // ensure that emission times remain mostly linear as the number of receivers increase

    SimpleSenderObject sender;
    int elapsed = 0;
    const int increase = 3000;
    const int base = 5000;

    for (int i = 0; i < 4; ++i) {
        const int size = base + i * increase;
        const double increaseRatio = double(size) / (double)(size - increase);

        QList<SimpleReceiverObject *> receivers;
        for (int k = 0; k < size; ++k) {
            SimpleReceiverObject *receiver = new SimpleReceiverObject;
            QObject::connect(&sender, SIGNAL(signal()), receiver, SLOT(slot()));
            receivers.append(receiver);
        }

        QTime timer;
        timer.start();
        sender.emitSignal();
        int e = timer.elapsed();

        if (elapsed > 1) {
            qDebug() << size << "receivers, elapsed time" << e << "compared to previous time" << elapsed;
            QVERIFY(double(e) / double(elapsed) <= increaseRatio * 2.0);
        } else {
            qDebug() << size << "receivers, elapsed time" << e << "cannot be compared to previous, unmeasurable time";
        }
        elapsed = e;

        qDeleteAll(receivers);
        receivers.clear();
    }
}


QTEST_MAIN(tst_QObjectPerformance)
#include "tst_qobjectperformance.moc"

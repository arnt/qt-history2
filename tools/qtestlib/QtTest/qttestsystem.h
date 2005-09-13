#ifndef QTTESTSYSTEM_H
#define QTTESTSYSTEM_H

#include <QtTest/qttestcase.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>

namespace QtTest
{
    inline static void wait(int ms)
    {
        Q_ASSERT(QCoreApplication::instance());

        QTime timer;
        timer.start();
        do {
            QCoreApplication::processEvents(QEventLoop::AllEvents, ms);
            QtTest::sleep(10);
        } while (timer.elapsed() < ms);
    }
}

#endif

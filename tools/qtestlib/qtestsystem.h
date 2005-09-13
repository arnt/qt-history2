#ifndef QTESTSYSTEM_H
#define QTESTSYSTEM_H

#include <QTest/qtestcase.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>

namespace QTest
{
    inline static void wait(int ms)
    {
        Q_ASSERT(QCoreApplication::instance());

        QTime timer;
        timer.start();
        do {
            QCoreApplication::processEvents(QEventLoop::AllEvents, ms);
            QTest::sleep(10);
        } while (timer.elapsed() < ms);
    }
}

#endif

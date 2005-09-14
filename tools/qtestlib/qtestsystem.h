/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTESTSYSTEM_H
#define QTESTSYSTEM_H

#include <QtTest/qtestcase.h>
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

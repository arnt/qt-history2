#include "QTest/private/qabstracttestlogger_p.h"
#include "QTest/private/qtestlog_p.h"
#include "QTest/qtestassert.h"

#include <stdio.h>
#include <stdlib.h>

namespace QTest
{
    static FILE *stream = 0;
}

void QAbstractTestLogger::outputString(const char *msg)
{
    QTEST_ASSERT(QTest::stream);

    ::fputs(msg, QTest::stream);
    ::fflush(QTest::stream);
}

void QAbstractTestLogger::startLogging()
{
    QTEST_ASSERT(!QTest::stream);

    const char *out = QTestLog::outputFileName();
    if (!out) {
        QTest::stream = stdout;
        return;
    }

    QTest::stream = ::fopen(out, "wt");
    if (!QTest::stream) {
        printf("Unable to open file for logging: %s", out);
        ::exit(1);
    }
}

void QAbstractTestLogger::stopLogging()
{
    QTEST_ASSERT(QTest::stream);
    if (QTest::stream != stdout)
        fclose(QTest::stream);
    QTest::stream = 0;
}


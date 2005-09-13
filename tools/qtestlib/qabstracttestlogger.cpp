#include "QtTest/private/qabstracttestlogger_p.h"
#include "QtTest/private/qtestlog_p.h"
#include "QtTest/qttestassert.h"

#include <stdio.h>
#include <stdlib.h>

namespace QtTest
{
    static FILE *stream = 0;
}

void QAbstractTestLogger::outputString(const char *msg)
{
    QTEST_ASSERT(QtTest::stream);

    ::fputs(msg, QtTest::stream);
    ::fflush(QtTest::stream);
}

void QAbstractTestLogger::startLogging()
{
    QTEST_ASSERT(!QtTest::stream);

    const char *out = QtTestLog::outputFileName();
    if (!out) {
        QtTest::stream = stdout;
        return;
    }

    QtTest::stream = ::fopen(out, "wt");
    if (!QtTest::stream) {
        printf("Unable to open file for logging: %s", out);
        ::exit(1);
    }
}

void QAbstractTestLogger::stopLogging()
{
    QTEST_ASSERT(QtTest::stream);
    if (QtTest::stream != stdout)
        fclose(QtTest::stream);
    QtTest::stream = 0;
}


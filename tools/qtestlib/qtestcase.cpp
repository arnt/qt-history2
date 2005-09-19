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

#include "QtTest/qtestcase.h"
#include "QtTest/qtestassert.h"

#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>

#include "QtTest/private/qtestlog_p.h"
#include "QtTest/private/qtesttable_p.h"
#include "QtTest/qtestdata.h"
#include "QtTest/private/qtestresult_p.h"
#include "QtTest/private/qsignaldumper_p.h"

#ifndef QTEST_LIGHT
#include "QtTest/private/qtestextended_p.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef Q_OS_WIN32
#include <windows.h> // for Sleep
#endif
#ifdef Q_OS_UNIX
#include <time.h>
#endif

namespace QTest
{
    static bool skipCurrentTest = false;
    static QObject *currentTestObject = 0;

    struct TestFunction {
        TestFunction() { function = 0; data = 0; }
        ~TestFunction() { if(data) free(data); }
        int function;
        char *data;
    } testFuncs[512];
    static int lastTestFuncIdx = -1;

    static int keyDelay = -1;
    static int mouseDelay = -1;
    static int eventDelay = -1;
    static int keyVerbose = -1;

int qt_snprintf(char *str, int size, const char *format, ...)
{
    va_list ap;
    int res = 0;

    va_start(ap, format);
    ::qvsnprintf(str, size, format, ap);
    va_end(ap);
    str[size - 1] = '\0';

    char *idx = str;
    while (*idx) {
        if (((*idx < 0x20 && *idx != '\n' && *idx != '\t') || *idx > 0x7e))
            *idx = '?';
        ++idx;
    }
    return res;
}

bool Q_TESTLIB_EXPORT defaultKeyVerbose()
{
    if (keyVerbose == -1) {
        keyVerbose = ::getenv("QTEST_KEYEVENT_VERBOSE") ? 1 : 0;
    }
    return keyVerbose == 1;
}

int Q_TESTLIB_EXPORT defaultEventDelay()
{
    if (eventDelay == -1) {
        if (::getenv("QTEST_EVENT_DELAY"))
            eventDelay = atoi(::getenv("QTEST_EVENT_DELAY"));
        else
            eventDelay = 0;
    }
    return eventDelay;
}

int Q_TESTLIB_EXPORT defaultMouseDelay()
{
    if (mouseDelay == -1) {
        if (::getenv("QTEST_MOUSEEVENT_DELAY"))
            mouseDelay = atoi((::getenv("QTEST_MOUSEEVENT_DELAY")));
        else
            mouseDelay = defaultEventDelay();
    }
    return mouseDelay;
}

int Q_TESTLIB_EXPORT defaultKeyDelay()
{
    if (keyDelay == -1) {
        if (::getenv("QTEST_KEYEVENT_DELAY"))
            keyDelay = atoi(::getenv("QTEST_KEYEVENT_DELAY"));
        else
            keyDelay = defaultEventDelay();
    }
    return keyDelay;
}

static bool isValidSlot(const QMetaMethod &sl)
{
    if (sl.access() != QMetaMethod::Private || !sl.parameterTypes().isEmpty()
        || qstrlen(sl.typeName()) || sl.methodType() != QMetaMethod::Slot)
        return false;
    const char *sig = sl.signature();
    int len = qstrlen(sig);
    if (len < 2)
        return false;
    if (sig[len - 2] != '(' || sig[len - 1] != ')')
        return false;
    if (len > 7 && strcmp(sig + (len - 7), "_data()") == 0)
        return false;
    if (strcmp(sig, "initTestCase()") == 0 || strcmp(sig, "cleanupTestCase()") == 0
        || strcmp(sig, "cleanup()") == 0 || strcmp(sig, "init()") == 0)
        return false;
    return true;
}

static void qPrintTestSlots()
{
    for (int i = 0; i < QTest::currentTestObject->metaObject()->methodCount(); ++i) {
        QMetaMethod sl = QTest::currentTestObject->metaObject()->method(i);
        if (isValidSlot(sl))
            printf("%s\n", sl.signature());
    }
}

static void qParseArgs(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-help") == 0) {
            const char *eHelp = "";
#ifndef QTEST_LIGHT
            eHelp = QTest::helpText();
#endif
            printf(" Usage: %s [options] [testfunctions[:testdata]]...\n"
                   "    By default, all testfunction will be run.\n\n"
                   " options:\n"
                   " -functions : Returns a list of current testfunctions\n"
                   " -xml       : Outputs results as XML document\n"
                   " -lightxml  : Outputs results as stream of XML tags\n"
                   " -o filename: Writes all output into a file\n"
                   " -v1        : Print enter messages for each testfunction\n"
                   " -v2        : Also print out each VERIFY/COMPARE/TEST\n"
                   " -vs        : Print every signal emitted\n"
                   " -eventdelay ms    : Set default delay for mouse and keyboard simulation to ms milliseconds\n"
                   " -keydelay ms      : Set default delay for keyboard simulation to ms milliseconds\n"
                   " -mousedelay ms    : Set default delay for mouse simulation to ms milliseconds\n"
                   " -keyevent-verbose : Turn on verbose messages for keyboard simulation\n"
                   "%s"
                   " -help      : This help\n", argv[0], eHelp);
            exit(0);
        } else if (strcmp(argv[i], "-functions") == 0) {
            qPrintTestSlots();
            exit(0);
        } else if (strcmp(argv[i], "-xml") == 0) {
            QTestLog::setLogMode(QTestLog::XML);
        } else if (strcmp(argv[i], "-lightxml") == 0) {
            QTestLog::setLogMode(QTestLog::LightXML);
        } else if (strcmp(argv[i], "-v1") == 0) {
            QTestLog::setVerboseLevel(1);
        } else if (strcmp(argv[i], "-v2") == 0) {
            QTestLog::setVerboseLevel(2);
        } else if (strcmp(argv[i], "-vs") == 0) {
            QSignalDumper::startDump();
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                printf("-o needs an extra parameter specifying the filename\n");
                exit(1);
            } else {
                QTestLog::redirectOutput(argv[++i]);
            }
        } else if (strcmp(argv[i], "-eventdelay") == 0) {
            if (i + 1 >= argc) {
                printf("-eventdelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QTest::eventDelay = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-keydelay") == 0) {
            if (i + 1 >= argc) {
                printf("-keydelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QTest::keyDelay = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-mousedelay") == 0) {
            if (i + 1 >= argc) {
                printf("-mousedelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QTest::mouseDelay = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-keyevent-verbose") == 0) {
            QTest::keyVerbose = 1;
        } else if (strcmp(argv[i], "-qws") == 0) {
            // do nothing
        } else if (argv[i][0] == '-') {
#ifndef QTEST_LIGHT
            if (!QTest::parseArg_hook(i, argc, argv)) {
                printf("Unknown option: '%s', try -help\n", argv[i]);
                exit(1);
            }
#else
            printf("Unknown option: '%s'\n", argv[i]);
            exit(1);
#endif
        } else {
            int colon = -1;
            char buf[512], *data=0;
            for(int off = 0; *(argv[i]+off); ++off) {
                if (*(argv[i]+off) == ':') {
                    colon = off;
                    break;
                }
            }
            if(colon != -1) {
                *(argv[i]+colon) = '\0';
                data = qstrdup(argv[i]+colon+1);
            }
            QTest::qt_snprintf(buf, 512, "%s()", argv[i]);
            int idx = QTest::currentTestObject->metaObject()->indexOfMethod(buf);
            if (idx < 0 || !isValidSlot(QTest::currentTestObject->metaObject()->method(idx))) {
                printf("Unknown testfunction: '%s'\n", buf);
                printf("Available testfunctions:\n");
                qPrintTestSlots();
                exit(1);
            }
            ++QTest::lastTestFuncIdx;
            QTest::testFuncs[QTest::lastTestFuncIdx].function = idx;
            QTest::testFuncs[QTest::lastTestFuncIdx].data = data;
            QTEST_ASSERT(QTest::lastTestFuncIdx < 512);
        }
    }
}


static bool qInvokeTestMethod(const char *slotName, const char *data=0)
{
    QTEST_ASSERT(slotName);

    char cur[512];
    QTestTable table;

    char *sl = qstrdup(slotName);
    sl[strlen(sl) - 2] = '\0';
    QTestResult::setCurrentTestFunction(sl);

    const QTestTable *gTable = QTestTable::globalTestTable();
    const int globalDataCount = gTable->dataCount();
    int curGlobalDataIndex = 0;
    do {
        if (!gTable->isEmpty())
            QTestResult::setCurrentGlobalTestData(gTable->testData(curGlobalDataIndex));

        QTestResult::setCurrentTestLocation(QTestResult::DataFunc);
        QTest::qt_snprintf(cur, 512, "%s_data", sl);
        QMetaObject::invokeMethod(QTest::currentTestObject, cur, Qt::DirectConnection);

        bool foundFunction = false;
        if (!QTest::skipCurrentTest) {
            int curDataIndex = 0;
            const int dataCount = table.dataCount();
            do {
                if (!data || !qstrcmp(data, table.testData(curDataIndex)->dataTag())) {
                    foundFunction = true;
                    if (!table.isEmpty())
                        QTestResult::setCurrentTestData(table.testData(curDataIndex));
                    QTestResult::setCurrentTestLocation(QTestResult::InitFunc);
                    QMetaObject::invokeMethod(QTest::currentTestObject, "init");
                    if (QTest::skipCurrentTest)
                        break;

                    QTestResult::setCurrentTestLocation(QTestResult::Func);
                    if (!QMetaObject::invokeMethod(QTest::currentTestObject, sl,
                                                  Qt::DirectConnection)) {
                        QTestResult::addFailure("Unable to execute slot", __FILE__, __LINE__);
                        break;
                    }

                    QTestResult::setCurrentTestLocation(QTestResult::CleanupFunc);
                    QMetaObject::invokeMethod(QTest::currentTestObject, "cleanup");
                    QTestResult::setCurrentTestLocation(QTestResult::NoWhere);
                    QTestResult::setCurrentTestData(0);

                    if (QTest::skipCurrentTest)
                        // check whether SkipAll was requested
                        break;
                    if (data)
                        break;
                }
                ++curDataIndex;
            } while (curDataIndex < dataCount);
        }
        QTest::skipCurrentTest = false;

        if (data && !foundFunction) {
            printf("Unknown testdata for function %s: '%s'\n", slotName, data);
            printf("Available testdata:\n");
            for(int i = 0; i < table.dataCount(); ++i)
                printf("%s\n", table.testData(i)->dataTag());
            return false;
        }

        QTestResult::setCurrentGlobalTestData(0);
        ++curGlobalDataIndex;
    } while (curGlobalDataIndex < globalDataCount);

    QTestResult::finishedCurrentTestFunction();
    delete[] sl;

    return true;
}

void *fetchData(QTestData *data, const char *tagName, int typeId)
{
    QTEST_ASSERT(typeId);
    QTEST_ASSERT_X(data, "QTest::fetchData()", "Test data requested, but no testdata available .");
    QTEST_ASSERT(data->parent());

    int idx = data->parent()->indexOf(tagName);

    if (typeId != data->parent()->elementTypeId(idx)) {
        qFatal("Requested type '%s' does not match available type '%s'.",
               QMetaType::typeName(typeId),
               QMetaType::typeName(data->parent()->elementTypeId(idx)));
    }

    return data->data(idx);
}

} // namespace

int QTest::exec(QObject *testObject, int argc, char **argv)
{
#ifndef QT_NO_EXCEPTION
    try {
#endif

#if defined(Q_CC_MSVC)
    SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);
#endif

    QTEST_ASSERT(testObject);
    QTEST_ASSERT(!currentTestObject);
    currentTestObject = testObject;

#ifndef QTEST_LIGHT
    int hookVal = exec_hook(argc, argv);
    if (hookVal)
        return hookVal;
#endif

    const QMetaObject *mo = testObject->metaObject();
    QTEST_ASSERT(mo);

    QTestResult::setCurrentTestObject(mo->className());
    qParseArgs(argc, argv);

    QTestLog::startLogging();

    QTestResult::setCurrentTestFunction("initTestCase");
    QTestResult::setCurrentTestLocation(QTestResult::DataFunc);
    QTestTable *gTable = QTestTable::globalTestTable();
    QMetaObject::invokeMethod(testObject, "initTestCase_data", Qt::DirectConnection);

    QTestResult::setCurrentTestLocation(QTestResult::Func);
    QMetaObject::invokeMethod(testObject, "initTestCase");
    QTestResult::finishedCurrentTestFunction();

    if (lastTestFuncIdx >= 0) {
        for (int i = 0; i <= lastTestFuncIdx; ++i) {
            qInvokeTestMethod(mo->method(testFuncs[i].function).signature(), testFuncs[i].data);
        }
    } else {
        int sc = mo->methodCount();
        for (int i = 0; i < sc; ++i) {
            QMetaMethod sl = mo->method(i);
            if (!isValidSlot(sl))
                continue;
            qInvokeTestMethod(sl.signature());
        }
    }

    QTestResult::setCurrentTestFunction("cleanupTestCase");
    QMetaObject::invokeMethod(testObject, "cleanupTestCase");
    QTestResult::finishedCurrentTestFunction();
    QTestResult::setCurrentTestFunction(0);
    delete gTable; gTable = 0;

#ifndef QT_NO_EXCEPTION
    } catch (...) {
        QTestResult::addFailure("Caught unhandled exception", __FILE__, __LINE__);
        if (QTestResult::currentTestFunction()) {
            QTestResult::finishedCurrentTestFunction();
            QTestResult::setCurrentTestFunction(0);
        }

        QTestLog::stopLogging();
        return -1;
    }
#endif

    QTestLog::stopLogging();
    currentTestObject = 0;
#ifdef QTEST_NOEXITCODE
    return 0;
#else
    return QTestResult::failCount();
#endif
}

void QTest::fail(const char *statementStr, const char *file, int line)
{
    QTestResult::addFailure(statementStr, file, line);
}

bool QTest::verify(bool statement, const char *statementStr, const char *description,
                   const char *file, int line)
{
    return QTestResult::verify(statement, statementStr, description, file, line);
}

void QTest::skip(const char *message, QTest::SkipMode mode,
                 const char *file, int line)
{
    QTestResult::addSkip(message, mode, file, line);
    if (mode == QTest::SkipAll)
        skipCurrentTest = true;
}

bool QTest::expectFail(const char *dataIndex, const char *comment,
                       QTest::TestFailMode mode, const char *file, int line)
{
    return QTestResult::expectFail(dataIndex, comment, mode, file, line);
}

void QTest::warn(const char *message)
{
    QTestLog::warn(message);
}

void QTest::ignoreMessage(QtMsgType type, const char *message)
{
    QTestResult::ignoreMessage(type, message);
}

void *QTest::data(const char *tagName, int typeId)
{
    return fetchData(QTestResult::currentTestData(), tagName, typeId);
}

void *QTest::globalData(const char *tagName, int typeId)
{
    return fetchData(QTestResult::currentGlobalTestData(), tagName, typeId);
}

void *QTest::elementData(const char *tagName, int metaTypeId)
{
    QTEST_ASSERT(tagName);
    QTestData *data = QTestResult::currentTestData();
    QTEST_ASSERT(data);
    QTEST_ASSERT(data->parent());

    int idx = data->parent()->indexOf(tagName);
    QTEST_ASSERT(idx != -1);
    QTEST_ASSERT(data->parent()->elementTypeId(idx) == metaTypeId);

    return data->data(data->parent()->indexOf(tagName));
}

void QTest::addColumnInternal(int id, const char *name)
{
    QTestTable *tbl = QTestTable::currentTestTable();
    QTEST_ASSERT_X(tbl, "QTest::addColumn()", "Cannot add testdata outside of a _data slot.");

    tbl->addColumn(id, name);
}

QTestData &QTest::newRow(const char *dataTag)
{
    QTestTable *tbl = QTestTable::currentTestTable();
    QTEST_ASSERT_X(tbl, "QTest::addColumn()", "Cannot add testdata outside of a _data slot.");

    return *tbl->newData(dataTag);
}

const char *QTest::currentTestFunction()
{
    return QTestResult::currentTestFunction();
}

const char *QTest::currentDataTag()
{
    return QTestResult::currentDataTag();
}

bool QTest::currentTestFailed()
{
    return QTestResult::currentTestFailed();
}

void QTest::sleep(int ms)
{
#ifdef Q_OS_WIN32
    Sleep(uint(ms));
#else
    struct timespec ts = { 0, 0 };
    // a nanosecond is 1/1000 of a microsecond, a microsecond is 1/1000 of a millisecond
    ts.tv_nsec = ms * 1000 * 1000;
    nanosleep(&ts, NULL);
#endif
}

QObject *QTest::testObject()
{
    return currentTestObject;
}

bool QTest::compare_helper(bool success, const char *msg, const char *file, int line)
{
    return QTestResult::compare(success, msg, file, line);
}

bool QTest::compare_helper(bool success, const char *msg, char *val1, char *val2,
                    const char *actual, const char *expected, const char *file, int line)
{
    return QTestResult::compare(success, msg, val1, val2, actual, expected, file, line);
}

template<>
bool QTest::compare(float const &t1, float const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    return (qAbs(t1 - t2) > 0.00001f)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared floats are not the same (fuzzy compare)",
                             toString(t1), toString(t2), actual, expected, file, line);
}

template<>
bool QTest::compare(double const &t1, double const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    return (qAbs(t1 - t2) > 0.000000000001)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared doubles are not the same (fuzzy compare)",
                             toString(t1), toString(t2), actual, expected, file, line);
}

#define COMPARE_IMPL2(TYPE, FORMAT) \
template <> char *QTest::toString(const TYPE &t) \
{ \
    char *msg = new char[128]; \
    qt_snprintf(msg, 128, #FORMAT, t); \
    return msg; \
}

COMPARE_IMPL2(short, %hd)
COMPARE_IMPL2(ushort, %hu)
COMPARE_IMPL2(int, %d)
COMPARE_IMPL2(uint, %u)
COMPARE_IMPL2(long, %ld)
COMPARE_IMPL2(ulong, %lu)
COMPARE_IMPL2(qint64, %lld)
COMPARE_IMPL2(quint64, %llu)
COMPARE_IMPL2(bool, %d)
COMPARE_IMPL2(char, %c)
COMPARE_IMPL2(float, %f);
COMPARE_IMPL2(double, %lf);

char *QTest::toString(const char *str)
{
    if (!str)
        return 0;
    char *msg = new char[strlen(str) + 1];
    return strcpy(msg, str);
}

char *QTest::toString(const void *p)
{
    char *msg = new char[128];
    qt_snprintf(msg, 128, "%p", p);
    return msg;
}


#include "QtTest/qtestassert.h"

#include "QtTest/private/qtestlog_p.h"
#include "QtTest/private/qtestresult_p.h"
#include "QtTest/private/qabstracttestlogger_p.h"
#include "QtTest/private/qplaintestlogger_p.h"
#include "QtTest/private/qxmltestlogger_p.h"

#include "qatomic.h"

#include <stdlib.h>
#include <string.h>

namespace QTest {

    struct IgnoreResultList
    {
        inline IgnoreResultList(QtMsgType tp, const char *message)
            : type(tp), next(0)
        { msg = strdup(message); }
        inline ~IgnoreResultList()
        { free(msg); }

        static inline void clearList(IgnoreResultList *&list)
        {
            while (list) {
                IgnoreResultList *current = list;
                list = list->next;
                delete current;
            }
        }

        QtMsgType type;
        char *msg;
        IgnoreResultList *next;
    };

    static IgnoreResultList *ignoreResultList = 0;

    static QTestLog::LogMode logMode = QTestLog::Plain;
    static int verbosity = 0;

    static QAbstractTestLogger *testLogger = 0;
    static const char *outFile = 0;

    static QtMsgHandler oldMessageHandler;

    static bool handleIgnoredMessage(QtMsgType type, const char *msg)
    {
        IgnoreResultList *last = 0;
        IgnoreResultList *list = ignoreResultList;
        while (list) {
            if (list->type == type && strcmp(msg, list->msg) == 0) {
                // remove the item from the list
                if (last)
                    last->next = list->next;
                else if (list->next)
                    ignoreResultList = list->next;
                else
                    ignoreResultList = 0;

                delete list;
                return true;
            }

            last = list;
            list = list->next;
        }
        return false;
    }

    static void messageHandler(QtMsgType type, const char *msg)
    {
        static QBasicAtomic counter = Q_ATOMIC_INIT(2002);

        if (!msg || !QTest::testLogger) {
            // if this goes wrong, something is seriously broken.
            qInstallMsgHandler(oldMessageHandler);
            QTEST_ASSERT(msg);
            QTEST_ASSERT(QTest::testLogger);
        }

        if (handleIgnoredMessage(type, msg))
            // the message is expected, so just swallow it.
            return;

        if (type != QtFatalMsg) {
            if (counter <= 0)
                return;

            if (!counter.deref()) {
                QTest::testLogger->addMessage(QAbstractTestLogger::QSystem,
                        "Maximum amount of warnings exceeded.");
                return;
            }
        }

        switch (type) {
        case QtDebugMsg:
            QTest::testLogger->addMessage(QAbstractTestLogger::QDebug, msg);
            break;
        case QtCriticalMsg:
            QTest::testLogger->addMessage(QAbstractTestLogger::QSystem, msg);
            break;
        case QtWarningMsg:
            QTest::testLogger->addMessage(QAbstractTestLogger::QWarning, msg);
            break;
        case QtFatalMsg:
            QTest::testLogger->addMessage(QAbstractTestLogger::QFatal, msg);
            break;
        }
    }


}

QTestLog::QTestLog()
{
}

QTestLog::~QTestLog()
{
}

void QTestLog::enterTestFunction(const char* function)
{
    QTEST_ASSERT(QTest::testLogger);
    QTEST_ASSERT(function);

    QTest::testLogger->enterTestFunction(function);
}

int QTestLog::unhandledIgnoreMessages()
{
    int i = 0;
    QTest::IgnoreResultList *list = QTest::ignoreResultList;
    while (list) {
        ++i;
        list = list->next;
    }
    return i;
}

void QTestLog::leaveTestFunction()
{
    QTEST_ASSERT(QTest::testLogger);

    QTest::IgnoreResultList::clearList(QTest::ignoreResultList);
    QTest::testLogger->leaveTestFunction();
}

void QTestLog::printUnhandledIgnoreMessages()
{
    QTEST_ASSERT(QTest::testLogger);

    char msg[1024];
    QTest::IgnoreResultList *list = QTest::ignoreResultList;
    while (list) {
        QTest::qt_snprintf(msg, 1024, "Did not receive message: \"%s\"", list->msg);
        QTest::testLogger->addMessage(QAbstractTestLogger::Info, msg);

        list = list->next;
    }
}

void QTestLog::addPass(const char *msg)
{
    QTEST_ASSERT(QTest::testLogger);
    QTEST_ASSERT(msg);

    QTest::testLogger->addIncident(QAbstractTestLogger::Pass, msg);
}

void QTestLog::addFail(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(QTest::testLogger);

    QTest::testLogger->addIncident(QAbstractTestLogger::Fail, msg, file, line);
}

void QTestLog::addXFail(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(QTest::testLogger);
    QTEST_ASSERT(msg);
    QTEST_ASSERT(file);

    QTest::testLogger->addIncident(QAbstractTestLogger::XFail, msg, file, line);
}

void QTestLog::addXPass(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(QTest::testLogger);
    QTEST_ASSERT(msg);
    QTEST_ASSERT(file);

    QTest::testLogger->addIncident(QAbstractTestLogger::XPass, msg, file, line);
}

void QTestLog::addSkip(const char *msg, QTest::SkipMode /*mode*/,
                       const char *file, int line)
{
    QTEST_ASSERT(QTest::testLogger);
    QTEST_ASSERT(msg);
    QTEST_ASSERT(file);

    QTest::testLogger->addMessage(QAbstractTestLogger::Skip, msg, file, line);
}

void QTestLog::startLogging()
{
    QTEST_ASSERT(!QTest::testLogger);

    switch (QTest::logMode) {
    case QTestLog::Plain:
        QTest::testLogger = new QPlainTestLogger();
        break;
    case QTestLog::XML:
        QTest::testLogger = new QXmlTestLogger(QXmlTestLogger::Complete);
        break;
    case QTestLog::LightXML:
        QTest::testLogger = new QXmlTestLogger(QXmlTestLogger::Light);
    }

    QTest::testLogger->startLogging();

    QTest::oldMessageHandler = qInstallMsgHandler(QTest::messageHandler);
}

void QTestLog::stopLogging()
{
    qInstallMsgHandler(QTest::oldMessageHandler);

    QTEST_ASSERT(QTest::testLogger);
    QTest::testLogger->stopLogging();
    delete QTest::testLogger;
    QTest::testLogger = 0;
}

void QTestLog::warn(const char *msg)
{
    QTEST_ASSERT(msg);

    QTest::testLogger->addMessage(QAbstractTestLogger::Warn, msg);
}

void QTestLog::info(const char *msg, const char *file, int line)
{
    QTEST_ASSERT(msg);

    if (QTest::testLogger)
    QTest::testLogger->addMessage(QAbstractTestLogger::Info, msg, file, line);
}

void QTestLog::setLogMode(LogMode mode)
{
    QTest::logMode = mode;
}

QTestLog::LogMode QTestLog::logMode()
{
    return QTest::logMode;
}

void QTestLog::setVerboseLevel(int level)
{
    QTest::verbosity = level;
}

int QTestLog::verboseLevel()
{
    return QTest::verbosity;
}

void QTestLog::addIgnoreMessage(QtMsgType type, const char *msg)
{
    QTest::IgnoreResultList *item = new QTest::IgnoreResultList(type, msg);

    QTest::IgnoreResultList *list = QTest::ignoreResultList;
    if (!list) {
        QTest::ignoreResultList = item;
        return;
    }
    while (list->next)
        list = list->next;
    list->next = item;
}

void QTestLog::redirectOutput(const char *fileName)
{
    QTEST_ASSERT(fileName);

    QTest::outFile = fileName;
}

const char *QTestLog::outputFileName()
{
    return QTest::outFile;
}


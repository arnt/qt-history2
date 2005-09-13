#ifndef QTESTLOG_H
#define QTESTLOG_H

#include "QTest/qtest_global.h"

class QTestLog
{
public:
    enum LogMode { Plain = 0, XML, LightXML };

    static void enterTestFunction(const char* function);
    static void leaveTestFunction();

    static void addPass(const char *msg);
    static void addFail(const char *msg, const char *file, int line);
    static void addXFail(const char *msg, const char *file, int line);
    static void addXPass(const char *msg, const char *file, int line);
    static void addSkip(const char *msg, QTest::SkipMode mode,
                        const char *file, int line);
    static void addIgnoreMessage(QtMsgType type, const char *msg);
    static int unhandledIgnoreMessages();
    static void printUnhandledIgnoreMessages();

    static void warn(const char *msg);
    static void info(const char *msg, const char *file, int line);

    static void startLogging();
    static void stopLogging();

    static void setLogMode(LogMode mode);
    static LogMode logMode();

    static void setVerboseLevel(int level);
    static int verboseLevel();

    static void redirectOutput(const char *fileName);
    static const char *outputFileName();

private:
    QTestLog();
    ~QTestLog();
};

#endif


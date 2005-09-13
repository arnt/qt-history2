#ifndef QSIGNALDUMPER_H
#define QSIGNALDUMPER_H

class QByteArray;

class QSignalDumper
{
public:
    static void startDump();
    static void endDump();

    static void ignoreClass(const QByteArray &klass);
    static void clearIgnoredClasses();
};

#endif


/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "logger.h"
#include <QFile>
#include <QDateTime>

LogEntry::LogEntry(QString type, QString location)
:type(type), location(location)
{}

PlainLogEntry::PlainLogEntry(QString type, QString location, QString text)
:LogEntry(type, location), text(text)
{}

SourcePointLogEntry::SourcePointLogEntry(QString type, QString location, QString file, int line, int column, QString text)
:LogEntry(type, location), file(file), line(line), column(column), text(text)
{}

QString SourcePointLogEntry::description() const
{
    return QLatin1String("In file ")  + file +
           QLatin1String(" at line ") + QString(QLatin1String("%1")).arg(line + 1) + //line count is zero based, adjust here.
           QLatin1String(" column ")  + QString(QLatin1String("%1")).arg(column) +
           QLatin1String(": ") + text ;
}

void SourcePointLogEntry::updateLinePos(int threshold,  int delta)
{
    if (line >= threshold)
        line += delta;
}

/////////////////////////////////////////////////////


Logger::~Logger()
{
   qDeleteAll(logEntries);
}

Logger *Logger::theInstance  = 0;
Logger *Logger::instance()
{
    if(!theInstance)
        theInstance = new Logger();
        return theInstance;
}

void Logger::deleteInstance()
{
    if(theInstance)
        delete theInstance;
}

void Logger::addEntry(LogEntry *entry)
{
   Q_ASSERT(entry);
   pendingLogEntries.append(entry);
}

void Logger::beginSection()
{
    commitSection();
}

void Logger::commitSection()
{
    logEntries += pendingLogEntries;
    pendingLogEntries.clear();
}

void Logger::revertSection()
{
    qDeleteAll(pendingLogEntries);
    pendingLogEntries.clear();
}

int Logger::numEntries()
{
    commitSection();
    return logEntries.size();
}

QStringList Logger::fullReport()
{
    commitSection();
    QStringList report;
    report << QLatin1String("Log for qt3to4 on ") + QDateTime::currentDateTime().toString() +
              QLatin1String(". Number of log entries: ") + QString(QLatin1String("%1")).arg(logEntries.size());
    foreach(LogEntry *logEntry, logEntries) {
        report << logEntry->description();
    }
    return report;
}

/*
    Update the line for all SourcePointLogEntrys in the list of pending log
    entries located on or after insertLine.
*/
void Logger::updateLineNumbers(int insertLine, int numLines)
{
    foreach(LogEntry *logEntry, pendingLogEntries) {
        logEntry->updateLinePos(insertLine, numLines);
    }
}

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

#include "logger.h"
#include <QFile>
#include <stdio.h>


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


Logger::~Logger()
{

}

void Logger::setFileState(QString file)
{
    fileState=file;
}


void Logger::addEntry(QString group, QString text, QString file, int line, int column)
{
    LogEntry entry;
    entry.group = group;
    entry.text = text;
    if(file.isEmpty())
        entry.file = fileState;
    else
        entry.file = file;
    entry.line = line;
    entry.column = column;
#if 0
    puts(QString("In file ") + entry.file + QString(" at line ") + QString("%1").arg(entry.line +1) +" column "  +
                                        QString("%1").arg(entry.column) +": " + entry.text );
#endif
    logEntries.append(entry);
}

void Logger::addMetaEntry(QString group, QString description)
{
    MetaLogEntry mentry;
    mentry.group = group;
    mentry.description = description;
    metaLogEntries.append(mentry);
}

QStringList Logger::cronologicalReport()
{
    QStringList report;
    report<<"Porting log. Number of Entries:" + QString("%1").arg(logEntries.size());

    foreach(LogEntry logEntry, logEntries) {

        report<< QString("In file ") + logEntry.file + QString(" at line ") + QString("%1").arg(logEntry.line +1) +" column "  +
                                        QString("%1").arg(logEntry.column) +": " + logEntry.text ;
    }
    return report;
}

void Logger::print(QStringList report)
{
    puts(report.join("\n").toLatin1());
}

void Logger::writeToFile(QString fileName, QStringList report)
{
    QFile f(fileName);
    f.open(QFile::WriteOnly);
    QString contents = report.join("\n");
    f.write(contents.toLatin1());
}

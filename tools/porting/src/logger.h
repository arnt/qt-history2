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

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
/*
    Base class for logger entries;
    description() should return a text for this entry,
*/
class LogEntry
{
public:
    LogEntry(QString type, QString location);
    virtual QString description() const =0;
    virtual ~LogEntry(){};
protected:
    QString type; // Error, Warning, Info, etc
    QString location;// preprocessor, c++parser, porting, etc
};

class PlainLogEntry: public LogEntry
{
public:
     PlainLogEntry(QString type, QString lcation, QString text);
     QString description() const {return text;};
protected:
     QString text;
};

/*
    A log entry that stores a source point: file, line and column.
*/
class SourcePointLogEntry : public LogEntry
{
public:
    SourcePointLogEntry(QString type, QString location, QString file, int line, int column, QString text);
    QString description() const;
protected:
    QString file;
    int line;
    int column;
    QString text;
};


class Logger
{
public:
    Logger(){};
    virtual ~Logger() {};
    static Logger *instance();
    static void deleteInstance();

    void addEntry(LogEntry *entry);
    QStringList fullReport();
    int numEntries();
    /*
        glabalState can be used for storage of application state
        together with the logger. This can be useful in some cases,
        for example the current filename is stored here when processing
        files.
    */
    QMap<QString, QString> globalState;
private:
    static Logger *theInstance;
    QList<LogEntry*> logEntries;
};


#endif

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QStringList>
#include <QList>
struct LogEntry
{
    QString group;
    QString text;
    QString file;
    int line;
    int column;
};

struct MetaLogEntry
{
    QString group;
    QString description;
};

class Logger
{
public:
    static Logger *instance();
    static void deleteInstance();
    ~Logger();
    /*
        State setting
    */
    void setFileState(QString file);

    /*
        Data adding funcitons
    */
    void addEntry(QString group, QString text, QString File=QString(), int line=0, int column=0);
    void addMetaEntry(QString group, QString description);

    /*
        Report generating functions
    */
    QStringList cronologicalReport();
    /*
    QString byFileReport();
    QString byGroupReport();
    */

    /*
        Report output:
    */
    void print(QStringList report);
    void writeToFile(QString fileName, QStringList report);
private:
    Logger(){};
    static Logger *theInstance;
    QString fileState;
    QList<LogEntry> logEntries;
    QList<MetaLogEntry> metaLogEntries;

};


#endif

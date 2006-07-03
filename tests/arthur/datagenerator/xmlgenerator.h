#ifndef XMLGENERATOR_H
#define XMLGENERATOR_H

#include "xmldata.h"

#include <QDateTime>
#include <QMap>
#include <QList>
#include <QString>


class XMLGenerator
{
public:
    XMLGenerator(const QString &baseDir);

    void startSuite(const QString &name);
    void startTestcase(const QString &testcase);
    void addImage(const QString &engine, const QString &image,
                  const XMLData &data, GeneratorFlags flags);
    void endTestcase();
    void endSuite();

    void checkDirs(const QDateTime &dt, const QString &baseDir);
    void generateOutput(const QString &baseDir);
    QString generateData() const;
private:
    QMap<QString, XMLEngine*> engines;
    QString currentSuite;
    QString currentTestcase;
};

#endif

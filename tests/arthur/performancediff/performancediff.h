#ifndef PERFORMANCEDIFF_H
#define PERFORMANCEDIFF_H

#include "xmldata.h"

#include <QMap>
#include <QString>

QT_DECLARE_CLASS(QStringList)
QT_DECLARE_CLASS(QSettings)

class PerformanceDiff
{
public:
    PerformanceDiff();

    //void generateOutput();

    void run(int argc, char **argv);

private:
    void processArguments(int argc, char **argv);
    void generateDiff();
private:
    QMap<QString, XMLEngine*> inputEngines;
    QMap<QString, XMLEngine*> diffEngines;

    QSettings *settings;
    QString inputDirName;
    QString diffDirName;
};

#endif

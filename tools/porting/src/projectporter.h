#ifndef PROJECTPORTER_H
#define PROJECTPORTER_H

#include <QString>
#include <QMap>
#include "filewriter.h"

class ProjectPorter
{
public:
    ProjectPorter(QString rulesFileName);
    void portProject(QString inPath, QString proFileName, QString outPath);
private:
    QString portProFile(QString contents, QMap<QString, QString> tagMap);
    QString rulesFileName;
    QMap<QString, int> processedFilesSet;
};

#endif

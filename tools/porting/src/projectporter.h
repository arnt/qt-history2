#ifndef PROJECTPORTER_H
#define PROJECTPORTER_H

#include <QString>
#include <QMap>
#include "fileporter.h"
#include "filewriter.h"

class ProjectPorter
{
public:
    ProjectPorter(QString rulesFileName);
    void portProject(QString inPath, QString proFileName);
private:
    QString portProFile(QString contents, QMap<QString, QString> tagMap);
    void portFiles(QString basePath, QStringList fileNames, FilePorter::FileType fileType);
    
    QString rulesFileName;
    QMap<QString, int> processedFilesSet;
    FilePorter filePorter;

};

#endif

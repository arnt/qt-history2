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

#ifndef PROJECTPORTER_H
#define PROJECTPORTER_H

#include <QString>
#include <QMap>
#include "fileporter.h"
#include "filewriter.h"
#include "preprocessorcontrol.h"
#include "translationunit.h"
#include "codemodelattributes.h"

class ProjectPorter : public QObject
{
Q_OBJECT
public:
    ProjectPorter(QString basePath, QStringList includeDirectories);
    void portProject(QString filePath);
    void portFile(QString filePath);
private slots:
    void error(QString type, QString text);
private:
    void portProject(QString inPath, QString proFileName);
    QString portProFile(QString contents, QMap<QString, QString> tagMap);
    void portFiles(QString basePath, QStringList fileNames);

    QMap<QString, int> processedFilesSet;
    QString basePath;
    bool analyze;
    IncludeFiles *includeFiles;
    PreprocessorController *preprocessorController;
    PreprocessorCache preprocessorCache;
    TranslationUnitAnalyzer translationUnitAnalyzer;
    CodeModelAttributes codeModelAttributes;
    FilePorter filePorter;
};

#endif

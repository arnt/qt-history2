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
    PreprocessorCache preprocessorCache;
    FilePorter filePorter;
};

#endif

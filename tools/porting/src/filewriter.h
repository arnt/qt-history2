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

#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QMap>
#include <QString>
#include <QByteArray>

class FileWriter
{
public:
    enum OverWriteFiles{DontOverWrite, AlwaysOverWrite, AskOnOverWrite};
    enum WriteResult { WriteFailed = 0, WriteSucceeded = 1, WriteSkipped };
    static FileWriter *instance();
    static void deleteInstance();

    FileWriter(OverWriteFiles overWRite = AskOnOverWrite,
                QString overwriteMessage = QString());
    WriteResult writeFile(QString filePath, QByteArray contents);
    WriteResult writeFileVerbously(QString filePath, QByteArray contents);
    void setOverwriteFiles(OverWriteFiles writeMode);
private:
    QMap<QString, int> processedFilesSet;
    OverWriteFiles overWriteFiles;
    QString overwriteMessage;
    static FileWriter *theInstance;
};

#endif

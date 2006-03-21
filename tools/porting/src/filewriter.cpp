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

#include "filewriter.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <ctype.h>

FileWriter *FileWriter::theInstance  = 0;

FileWriter *FileWriter::instance()
{
     if(!theInstance)
        theInstance = new FileWriter();

        return theInstance;
}

void FileWriter::deleteInstance()
{
    if(theInstance) {
        delete theInstance;
        theInstance=0;
    }
}

FileWriter::FileWriter(OverWriteFiles overWrite, QString overwriteMsg)
:overWriteFiles(overWrite)
,overwriteMessage(overwriteMsg)
{
    if(overwriteMessage.isEmpty())
       overwriteMessage = "Convert file ";
}

FileWriter::WriteResult FileWriter::writeFileVerbously(QString filePath, QByteArray contents)
{
    const WriteResult result = writeFile(filePath, contents);
    if (result == WriteSucceeded) {
        QString cleanPath = QDir::cleanPath(filePath);
        printf("Wrote to file: %s \n", QDir::convertSeparators(cleanPath).toLocal8Bit().constData());
    }
    return result;
}

FileWriter::WriteResult FileWriter::writeFile(QString filePath, QByteArray contents)
{
    if(filePath.isEmpty())
        return WriteFailed;
    QString path = QFileInfo(filePath).path();
    if (!QDir().mkpath(path)){
         printf("Error creating path %s \n", QDir::convertSeparators(path).toLocal8Bit().constData());
    }

    QString cleanPath = QDir::cleanPath(filePath);
    QFile f(cleanPath);
    if (f.exists()) {
        if (overWriteFiles == DontOverWrite) {
            printf("Error writing file %s: It already exists \n",
                QDir::convertSeparators(cleanPath).toLatin1().constData());
            return WriteFailed;
        } else if(overWriteFiles == AskOnOverWrite) {
            printf("%s%s? (Y)es, (N)o, (A)ll ", overwriteMessage.toLatin1().constData(),
                QDir::convertSeparators(cleanPath).toLatin1().constData());
            
            char answer = 0;
            while (answer != 'y' && answer != 'n' && answer != 'a') {
                scanf("%c", &answer);
                answer = tolower(answer);
            }

            if(answer == 'n')
                return WriteSkipped;
            else if(answer == 'a')
                overWriteFiles=AlwaysOverWrite;
        }
    }

    f.open(QFile::WriteOnly);
    if (f.isOpen() && f.write(contents) == contents.size())
        return WriteSucceeded;

    printf("Could not write to to file: %s. Is it write protected?\n",
        QDir::convertSeparators(filePath).toLatin1().constData());

    return WriteFailed;
}

/*
    Sets the write mode for the file writer. writeMode is one of
    DontOverWrite, AlwaysOverWrite, AskOnOverWrite.
*/
void FileWriter::setOverwriteFiles(OverWriteFiles writeMode)
{
    overWriteFiles = writeMode;
}

QByteArray detectLineEndings(const QByteArray &array)
{
    if (array.contains("\r\n")) {
        return "\r\n";
    } else {
        return "\n";
    }
}

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
#include <iostream>

using std::cout;
using std::cin;
using std::endl;


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

bool FileWriter::writeFileVerbously(QString filePath, QByteArray contents)
{
    if( writeFile(filePath, contents)) {
        QString cleanPath = QDir::cleanPath(filePath);
        cout << "Wrote to file: ";
        cout << QDir::convertSeparators(cleanPath).toLocal8Bit().constData() << endl;
        return true;
    }
    return false;
}

bool FileWriter::writeFile(QString filePath, QByteArray contents)
{
    if(filePath.isEmpty())
        return false;
    QString path = QFileInfo(filePath).path();
    if (!QDir().mkdir(path, QDir::Recursive)){
         cout << "Error creating path " <<
         cout << QDir::convertSeparators(path).toLocal8Bit().constData() << endl;
    }

    QString cleanPath = QDir::cleanPath(filePath);
    QFile f(cleanPath);
    if (f.exists()) {
        if (overWriteFiles == DontOverWrite) {
            cout << "Error writing file ";
            cout << QDir::convertSeparators(cleanPath).toLatin1().constData();
            cout << " It already exists" <<endl;
            return false;
        } else if(overWriteFiles == AskOnOverWrite) {
            cout << overwriteMessage.toLatin1().constData();
            cout << QDir::convertSeparators(cleanPath).toLatin1().constData();
            cout << "? (Y)es, (N)o, (A)ll ";

            char answer = 0;
            while (answer != 'y' && answer != 'n' && answer != 'a') {
                cin >> answer;
                answer = tolower(answer);
            }

            if(answer == 'n')
                return false;
            else if(answer == 'a')
                overWriteFiles=AlwaysOverWrite;
        }
    }

    f.open(QFile::WriteOnly);
    if (f.isOpen() && f.write(contents) == contents.size())
        return true;

    cout << "Could not write to to file: ";
    cout << QDir::convertSeparators(filePath).toLatin1().constData();
    cout << "Is it write protected?" << endl;
    return false;
}

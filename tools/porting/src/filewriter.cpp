#include "filewriter.h"
#include <iostream>
#include <QFile>
#include <QFileInfo>
#include <QDir>

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
        cout << QDir::convertSeparators(cleanPath).latin1() << endl;
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
         cout << QDir::convertSeparators(path).latin1() << endl;
    }
            
    QString cleanPath = QDir::cleanPath(filePath);
    QFile f(cleanPath);
    if (f.exists()) {
        if (overWriteFiles == DontOverWrite) {
            cout << "Error writing file ";
            cout << QDir::convertSeparators(cleanPath).latin1();
            cout << " It already exists" <<endl;
            return false;
        } else if(overWriteFiles == AskOnOverWrite) {
            cout << overwriteMessage.latin1();
            cout << QDir::convertSeparators(cleanPath).latin1();
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
    cout << QDir::convertSeparators(filePath).latin1();
    cout << "Is it write protected?" << endl;
    return false;
}

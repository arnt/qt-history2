#include "filewriter.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

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

FileWriter::FileWriter(OverWriteFiles overWrite)
:overWriteFiles(overWrite)
{
    
}

bool FileWriter::writeFileVerbously(QString filePath, QByteArray contents)
{
    
    if( writeFile(filePath, contents)) {
        QString cleanPath = QDir::cleanPath(filePath);
        printf("Wrote to file: %s\n", cleanPath.latin1());
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
        printf("Error creating path %s\n", (path).latin1());
    }
            
    QString cleanPath = QDir::cleanPath(filePath);
    QFile f(cleanPath);
    if (f.exists()) {
        if (overWriteFiles == DontOverWrite) {
            printf("Error writing file %s: It already exists\n", cleanPath.latin1());
            return false;
        } else if(overWriteFiles == AskOnOverWrite) {
            printf("Warning: file %s already exists.", cleanPath.latin1());
            char answer = 0;
            int ret = 0;
            printf("\nOverwrite? (Y)es, (N)o, (A)lways ");
            while (ret == 0 || (answer != 'y' && answer != 'n' && answer != 'a')) {
                ret = scanf("%c", &answer);            
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
    
    printf("Could not write to to file: %s\n", filePath.latin1());
    return false;
}

#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QMap>
#include <QString>
#include <QByteArray>

class FileWriter
{
public:
    enum OverWriteFiles{DontOverWrite, AlwaysOverWrite, AskOnOverWrite};
    static FileWriter *instance();
    static void deleteInstance();
    
    FileWriter(OverWriteFiles overWRite = AskOnOverWrite, 
                QString overwriteMessage = QString());
    bool writeFile(QString filePath, QByteArray contents);
    bool writeFileVerbously(QString filePath, QByteArray contents);
private:    
    QMap<QString, int> processedFilesSet;
    OverWriteFiles overWriteFiles;
    QString overwriteMessage;
    static FileWriter *theInstance;
};

#endif

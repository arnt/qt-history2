#include "projectporter.h"
#include <iostream>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include "qtsimplexml.h"
#include "proparser.h"
#include "textreplacement.h"
#include "fileporter.h"
#include "logger.h"

using std::cout;
using std::endl;

ProjectPorter::ProjectPorter(QString rulesFileName)
:rulesFileName(rulesFileName)
,filePorter(rulesFileName)
{

}

void ProjectPorter::portProject(QString basePath, QString proFileName)
{
    QString fullInFileName = basePath + "/" + proFileName;
    QFileInfo infileInfo(fullInFileName);
    if(!infileInfo.exists()) {
        cout<<"Could not open file: " << QDir::convertSeparators(fullInFileName).latin1() << endl;
        return;
    }     
   
    QString proFileContents = loadFile(fullInFileName);
    QMap<QString, QString> proFileMap = proFileTagMap(proFileContents, QDir(basePath).absolutePath());
#if 0
    puts(proFileContents.latin1());
    puts(proFileMap["SOURCES"].latin1());
    puts(proFileMap["HEADERS"].latin1());
    puts(proFileMap["INCLUDEPATH"].latin1());
    puts(proFileMap["DEPENDPATH"].latin1());
#endif
    
    /*
        Check if this is a TEMPLATE = subdirs .pro file, in that case we
        process each subdir (recursively)
    */
    QString templateTag = proFileMap["TEMPLATE"];
    if(templateTag == "subdirs") {
        QStringList subdirs = proFileMap["SUBDIRS"].split(" ", QString::SkipEmptyParts);
        foreach(QString subdir, subdirs) {
            QString newBasePath  = basePath + "/" + subdir;
            QStringList dirsInSubdir = subdir.split(QRegExp("/|\\\\"), QString::SkipEmptyParts);
            QString newProFileName = dirsInSubdir.last() + ".pro";
            portProject(newBasePath, newProFileName);
        }
     }
    /*
        Get headers and sources file names from .pro file.
    */
    QStringList sources = proFileMap["SOURCES"].split(" ", QString::SkipEmptyParts);
    QStringList headers = proFileMap["HEADERS"].split(" ", QString::SkipEmptyParts);
    
    portFiles(basePath, sources, FilePorter::Source);
    portFiles(basePath, headers, FilePorter::Header);
    
    Logger::instance()->setFileState(fullInFileName); 
    QString portedProFile = portProFile(proFileContents, proFileMap);
    FileWriter::instance()->writeFileVerbously(fullInFileName , portedProFile.latin1());
}

void ProjectPorter::portFiles(QString basePath, QStringList fileNames, FilePorter::FileType fileType)
{
    foreach(QString fileName, fileNames) {
        QString fullFilePath; 
        QFileInfo fileInfo(fileName);
        if (fileInfo.isAbsolute()) {
            fullFilePath = QDir::cleanPath(fileName);
        } else {
            fullFilePath = QDir::cleanPath(basePath + "/" + fileName);
        }
        
        QFileInfo fullFilePathInfo(fullFilePath);
        if(!fullFilePathInfo.exists()) {
            cout << "Could not find file:" << 
                QDir::convertSeparators(fullFilePath).latin1() <<endl;        
            continue;
        }
       
        if(!processedFilesSet.contains(fullFilePath)){
            filePorter.port(QString(), fullFilePath, QString() , fullFilePath, fileType);
            processedFilesSet.insert(fullFilePath, 0);
        }
    }
}

QString ProjectPorter::portProFile(QString contents, QMap<QString, QString> tagMap)
{
   
    Logger *logger = Logger::instance();
    
    //add compat to the Qt tag
    QStringList QTTagAdd;
    QStringList config = tagMap["QT"].split(" ", QString::SkipEmptyParts);
    if (!config.contains("compat"))
        QTTagAdd.append("compat");

    if (!QTTagAdd.isEmpty()) {
        contents += "\n#The following line was inserted by the Qt porting tool\n";
        QString insertText = "QT += " + QTTagAdd.join(" ");
        contents += insertText;
        QString logText = "Added entry to .pro file: " + insertText;
        logger->addEntry("profile", logText,  QString() , -2, -1); //TODO get line/column here
    }
 
    //comment out any REQUIRES tag
    //TODO: make this more intelligent by checking if REQUIRES really is a tag and 
    //not the name of a source file for example
    if(!tagMap["REQUIRES"].isEmpty()) {
        int j=0;
        while ((j = contents.indexOf("REQUIRES", j)) != -1) {
            QString insertText("#The following line was commented out by the Qt Porting tool\n#");
            logger->addEntry("profile", "Commented out REQUIRES in .pro file",  QString(), -2, -1); //TODO get line/column here
            contents.insert(j, insertText);
            j+=insertText.size() + 1;
        }
    }
         
    return contents;
}


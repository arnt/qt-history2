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

using std::cout;
using std::endl;

ProjectPorter::ProjectPorter(QString rulesFileName)
:rulesFileName(rulesFileName)
{

}

void ProjectPorter::portProject(QString inPath, QString proFileName, QString outPath)
{
    QString fullInFileName = inPath + proFileName;
    QFileInfo infileInfo(fullInFileName);
    if(!infileInfo.exists()) {
        cout<<"Could not open file: " << fullInFileName.latin1() <<endl;
        return;
    }     
   
    QString proFileContents = loadFile(fullInFileName);
    QMap<QString, QString> proFileMap = proFileTagMap(proFileContents);
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
            QString newInPath  = inPath  + subdir + "/";
            QString newOutPath = outPath + subdir + "/";
            
            QStringList dirsInSubdir = subdir.split(QRegExp("/|\\\\"), QString::SkipEmptyParts);
            QString newProFileName = dirsInSubdir.last() + ".pro";
            portProject(newInPath, newProFileName, newOutPath);
        }
     }
       
    /*
        Get headers and sources file names from .pro file.
    */
    QStringList sources = proFileMap["SOURCES"].split(" ", QString::SkipEmptyParts);
    QStringList headers = proFileMap["HEADERS"].split(" ", QString::SkipEmptyParts);

    #if 0
    qDebug("Source files:");
    foreach(QString sourceFileName, sources)
        qDebug("|"+sourceFileName+"|");

    qDebug("Header files:");
    foreach(QString headerFileName, headers)
        qDebug("|"+headerFileName+"|");
    #endif

    
    if(!outPath.isEmpty()&& !outPath.endsWith("/")) {
        outPath +="/";
    }
    
    //process headers
    FilePorter filePorter(rulesFileName);
    foreach(QString headerFileName, headers) {
        QString fullFilePath = QDir(inPath + headerFileName).absolutePath();
        if(!processedFilesSet.contains(fullFilePath)){
            filePorter.port(inPath , headerFileName, outPath , headerFileName, FilePorter::Header);
            processedFilesSet.insert(fullFilePath, 0);
        }
    }
    //process source files
    foreach(QString sourceFileName, sources) {
        QString fullFilePath = QDir(inPath + sourceFileName).absolutePath();
        if(!processedFilesSet.contains(fullFilePath)) {
            filePorter.port(inPath , sourceFileName, outPath , sourceFileName, FilePorter::Source);
            processedFilesSet.insert(fullFilePath, 0);
         }
    }

    QString portedProFile = portProFile(proFileContents, proFileMap);
    if(!outPath.isEmpty())
        FileWriter::instance()->writeFileVerbously(outPath+proFileName, portedProFile.latin1());
       
}

QString ProjectPorter::portProFile(QString contents, QMap<QString, QString> tagMap)
{
   
    //add compat to the Qt tag
    QStringList QTTagAdd;
    QStringList config = tagMap["QT"].split(" ", QString::SkipEmptyParts);
    if (!config.contains("compat"))
        QTTagAdd.append("compat");

    if (!QTTagAdd.isEmpty()) {
        contents += "\n#The following line was inserted by the Qt porting tool\n";
        contents += "QT += " + QTTagAdd.join(" ");
    }
 
    //comment out any REQUIRES tag
    //TODO: make this more intelligent by checking if REQUIRES really is a tag and 
    //not the name of a source file for example
    if(!tagMap["REQUIRES"].isEmpty()) {
        int j=0;
        while ((j = contents.indexOf("REQUIRES", j)) != -1) {
            QString insertText("#The following line was commented out by the Qt Porting tool\n#");
            contents.insert(j, insertText);
            j+=insertText.size() + 1;
        }
    }
    
         
    return contents;
}


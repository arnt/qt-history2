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

#include "projectporter.h"
#include "qtsimplexml.h"
#include "proparser.h"
#include "textreplacement.h"
#include "fileporter.h"
#include "logger.h"
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <iostream>

using std::cout;
using std::endl;


ProjectPorter::ProjectPorter(QString rulesFileName)
:rulesFileName(rulesFileName)
,filePorter(rulesFileName, preprocessorCache)
{}

void ProjectPorter::portProject(QString basePath, QString proFileName)
{
    QString fullInFileName = basePath + "/" + proFileName;
    QFileInfo infileInfo(fullInFileName);
    if(!infileInfo.exists()) {
        cout<<"Could not open file: " << QDir::convertSeparators(fullInFileName).toLocal8Bit().constData() << endl;
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
    QStringList forms = proFileMap["FORMS"].split(" ", QString::SkipEmptyParts);
    QStringList uidoth;
    for (int i = 0; i < forms.size(); ++i) {
        QString ui_h = forms.at(i) + ".h";
        if (QFile::exists(basePath + "/" + ui_h))
            uidoth += ui_h;
    }

    portFiles(basePath, sources);
    portFiles(basePath, headers);
    if (!uidoth.isEmpty())
        portFiles(basePath, uidoth);

    Logger::instance()->globalState["currentFileName"] = proFileName;
    QString portedProFile = portProFile(proFileContents, proFileMap);
    FileWriter::instance()->writeFileVerbously(fullInFileName , portedProFile.toLocal8Bit().constData());
}

/*
    Port each file given in the fileNames list. If a file name is relative
    it is assumed to be relative to basePath.
*/
void ProjectPorter::portFiles(QString basePath, QStringList fileNames)
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
                QDir::convertSeparators(fullFilePath).toLocal8Bit().constData() <<endl;
            continue;
        }

        if(!processedFilesSet.contains(fullFilePath)){
            Logger::instance()->globalState["currentFileName"] = fileName;
            filePorter.port(fullFilePath);
            processedFilesSet.insert(fullFilePath, 0);
        }
    }
}

QString ProjectPorter::portProFile(QString contents, QMap<QString, QString> tagMap)
{

    Logger *logger = Logger::instance();

    //add qt3support to the Qt tag
    QStringList QTTagAdd;
    QStringList qt = tagMap["QT"].split(" ", QString::SkipEmptyParts);
    if (!qt.contains("qt3support"))
        QTTagAdd.append("qt3support");
    QStringList config = tagMap["CONFIG"].split(" ", QString::SkipEmptyParts);
    if (config.contains("opengl"))
        QTTagAdd.append("opengl");
    if (config.contains("xml"))
        QTTagAdd.append("xml");
    if (config.contains("sql"))
        QTTagAdd.append("sql");
    if (config.contains("network"))
        QTTagAdd.append("network");

    if (!QTTagAdd.isEmpty()) {
        contents += "\n#The following line was inserted by the Qt porting tool\n";
        QString insertText = "QT += " + QTTagAdd.join(" ");
        contents += insertText;
         QString logText = "In file "
                        + logger->globalState.value("currentFileName")
                        + ": Added entry "
                        + insertText;
        logger->addEntry(new PlainLogEntry("Info", "Porting", logText));
    }
    if (!tagMap["FORMS"].isEmpty() || !tagMap["INTERFACES"].isEmpty()) {
        contents += "\n#The following line was inserted by the Qt porting tool\n";
        QString insertText = "CONFIG += uic3\n";
        contents += insertText;
        QString logText = "In file "
                        + logger->globalState.value("currentFileName")
                        + ": Added entry "
                        + insertText;
        logger->addEntry(new PlainLogEntry("Info", "Porting", logText));
    }

    //comment out any REQUIRES tag
    //TODO: make this more intelligent by checking if REQUIRES really is a tag and
    //not the name of a source file for example
    if(!tagMap["REQUIRES"].isEmpty()) {
        int j=0;
        while ((j = contents.indexOf("REQUIRES", j)) != -1) {
            QString insertText("#The following line was commented out by the Qt Porting tool\n#");
            QString logText = "In file "
                            + logger->globalState.value("currentFileName")
                            + ": Commented out REQUIRES section";
            logger->addEntry(new PlainLogEntry("Info", "Porting", logText));
            contents.insert(j, insertText);
            j+=insertText.size() + 1;
        }
    }

    return contents;
}

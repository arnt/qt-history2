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

#include <iostream>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QByteArray>
#include <QBuffer>
#include <QTextStream>
#include <QCoreApplication>
#include <QLibraryInfo>

#include "projectporter.h"
#include "fileporter.h"
#include "logger.h"
#include "preprocessorcontrol.h"
using std::cout;
using std::endl;

QString rulesFilePath;
QString applicationDirPath;


QString findRulesFile(const QString &fileName)
{
    //check QLibraryInfo::DataPath/filename
    QString filePath;
    filePath = QDir::cleanPath(QLibraryInfo::location(QLibraryInfo::DataPath) + "/" + fileName)  ;
    //cout << "checking" << filePath.toLatin1().constData() << endl;
    if (QFile::exists(filePath))
        return QFileInfo(filePath).canonicalFilePath();

    //check QLibraryInfo::PrefixPath/tools/porting/src/filename
    filePath = QDir::cleanPath(QLibraryInfo::location(QLibraryInfo::PrefixPath) + "/tools/porting/src/" + fileName);
    //cout << "checking" << filePath.toLatin1().constData() << endl;
    if (QFile::exists(filePath))
        return QFileInfo(filePath).canonicalFilePath();

    //no luck
    return QString();
}
/*
int fileMode(QString inFile, QStringList includeDirectories)
{
    if(!QFile::exists(inFile)) {
        cout << "Could not find file " << inFile.toLocal8Bit().constData() << endl;
        return 1;
    }

    QFileInfo inFileInfo = QFileInfo(inFile).canonicalPath();
    cout << inFileInfo.path().toLatin1().constData() << endl;
    cout << inFileInfo.fileName().toLatin1().constData() << endl;

    PreprocessorCache cache;

    if(!includeDirectories.isEmpty()) {
        cout << "analyzing source" << endl;
        IncludeFiles includeFiles(inFileInfo.path(), includeDirectories);
        PreprocessorController preprocController(includeFiles, cache, defaultMacros());
//        TokenEngine::TokenSectionSequence translationUnit = 
//            preprocController.evaluate(absInFile); 
    }


    FilePorter filePorter(rulesFilePath, cache);
    filePorter.port(inFileInfo.filePath());
    return 0;
}

int projectMode(QString inFile, QStringList includeDirectories)
{
    QFileInfo inFileInfo(inFile);
    if(!inFileInfo.exists()) {
        cout<<"Could not find file " << inFile.toLocal8Bit().constData() << endl;
        return 1;
    }

    ProjectPorter porter(rulesFilePath);
    porter.portProject(inFileInfo.path(), inFileInfo.fileName());
    return 0;
}
*/
void usage(char **argv)
{
    using namespace std;
    cout << "Tool for porting Qt 3 applications to Qt 4, using the compatibility library" << endl;
    cout << "and compatibility functions in the core library." << endl;
    cout << "Usage: " << argv[0] << " [options] <Infile>" << endl;
    cout << endl;
    cout << "Infile can be a source file or a project file." << endl;
    cout << "If you specify a project file, ending with .pro or .pri," << endl;
    cout << "qt3to4 will port all files specified in that project" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "-h            Display this help" << endl;
    cout << "-f file       Specify the location for the rules file." << endl;
    cout << endl;
    cout << "The porting documentation contains more information on how " << endl;
    cout << "to use qt3to4 as well as general porting information." << endl;
}

/*
    Syntax for override file:
    Same as main, with the addition that you can specify
    <item Type = "..." disable ="true">
    ...
    </item>
    To include the main file, use the following syntax:
    <include name="foo/bar.xml">
*/

int main(int argc, char**argv)
{
    QCoreApplication app(argc, argv);
    applicationDirPath = app.applicationDirPath();
    QString defualtRulesFileName = "q3porting.xml";
    QString inFileName;
    QStringList includeSearchDirectories;
    int currentArg = 1;

  //  cout << QLibraryInfo::location(QLibraryInfo::DataPath).toLatin1().constData() << endl;

    // read arguments
    while(currentArg < argc) {
        QString argText = argv[currentArg];
        if(argText.isEmpty())
            continue;
        else if (argText == "--help" || argText == "/h" || argText == "-help"
            || argText == "-h"  || argText == "-?" || argText == "/?") {
            usage(argv);
            return 0;
        } else if(argText == "-f") {
            ++currentArg;
            if (currentArg >= argc) {
                cout << "You must specify a file name along with -f" << endl;
                return 1;
            }
            rulesFilePath = argv[currentArg];

            if(!QFile::exists(rulesFilePath)) {
                cout << "File not found: " ;
                cout << rulesFilePath.toLocal8Bit().constData() << endl;
                return 1;
            }
        } else if(argText[0]  == '-') {
            cout << "Unknown option " << argText.toLocal8Bit().constData() << endl;
            return 1;
        } else {
            inFileName = argText;
        }
        ++currentArg;
    }

    if(rulesFilePath.isEmpty())
        rulesFilePath = findRulesFile(defualtRulesFileName);

    //Check if we have a rules file
    if (!QFile::exists(rulesFilePath)) {
        cout << "Error: Could not find the " << defualtRulesFileName.toLocal8Bit().constData() << "rules file: ";
        cout << "Please try specifying the file with the -f option" << endl;
        return 1;
    }
    cout << "Using rules file: ";
    cout << QDir::convertSeparators(rulesFilePath).toLocal8Bit().constData() <<endl;
    PortingRules::createInstance(rulesFilePath);

    //check if we have an infile
    if(inFileName.isEmpty()) {
        cout << "You must specify a file name" << endl; 
        return 1;
    }
    if (!QFile::exists(inFileName)) {
        cout << "Could not find infile: ";
        cout << QDir::convertSeparators(inFileName).toLocal8Bit().constData() <<endl;
        return 1;
    }
    inFileName = QFileInfo(inFileName).canonicalPath();

    //determine mode and do the port
    ProjectPorter porter(QDir::currentPath(), includeSearchDirectories);
    if(inFileName.endsWith(".pro") || inFileName.endsWith(".pri"))
        porter.portProject(inFileName);
    else
        porter.portFile(inFileName);

    //write log
    if(Logger::instance()->numEntries() > 0) {
        QStringList report = Logger::instance()->fullReport();
        QString logFileName =  "portinglog.txt";
        cout << "Writing log to " << logFileName.toLocal8Bit().constData() << endl;
        QByteArray logContents;
        QBuffer logBuffer(&logContents);
        logBuffer.open(QIODevice::Text | QIODevice::WriteOnly);
        QTextStream logStream(&logBuffer);
        foreach(QString logLine, report) {
            logStream << logLine << endl;
        }
        FileWriter fileWriter(FileWriter::AskOnOverWrite, "Overwrite file ");
        fileWriter.writeFile(logFileName, logContents);
    }
    Logger::deleteInstance();
    PortingRules::deleteInstance();
    return 0;
}

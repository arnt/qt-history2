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

/*
    Rules for findng q3porting.xml
    1. current path
    2. program path
    3. qInstallPathLibs()/qt3to4/
    4. $QTDIR/tools/porting/src/
    5. applicationDirPath()../lib/qt3to4/src/
    6. applicationDirPath()../tools/porting/src/
*/
QString findRulesFile(QString fileName, QString programPath)
{
    QString filePath;

    QFile f(fileName);
    if (f.exists()) {
        filePath=fileName;
    }

    if(filePath.isEmpty()) {
        filePath = QFileInfo(programPath).path() + "/" + fileName;
        QFile f(filePath);
        if (!f.exists())
            filePath=QString();
    }

    if(filePath.isEmpty()) {
        filePath = QDir::cleanPath(QLibraryInfo::location(QLibraryInfo::DataPath) + "/qt3to4/" + fileName);
        QFile f(filePath);
        if (!f.exists())
            filePath=QString();
    }

    if(filePath.isEmpty()) {
        filePath = QDir::cleanPath(QFile::decodeName((qgetenv("QTDIR"))) + "/tools/porting/src/" + fileName);
        QFile f(filePath);
        if (!f.exists())
            filePath=QString();
    }

    if(filePath.isEmpty()) {
        filePath = QDir::cleanPath(applicationDirPath + "/../lib/qt3to4/" + fileName);
        QFile f(filePath);
        if (!f.exists())
            filePath=QString();
    }

    if(filePath.isEmpty()) {
        filePath = QDir::cleanPath(applicationDirPath + "/../tools/porting/src/" + fileName);
        QFile f(filePath);
        if (!f.exists())
            filePath=QString();
    }

    if (!filePath.isEmpty())
        filePath = QFileInfo(filePath).absoluteFilePath();

    return filePath;
}

int fileMode(QString inFile)
{
    QFileInfo inFileInfo(inFile);
    if(!inFileInfo.exists()) {
        cout << "Could not find file " << inFile.toLocal8Bit().constData() << endl;
        return 1;
    }
    
    PreprocessorCache cache;
    FilePorter filePorter(rulesFilePath, cache);
    if (QFileInfo(rulesFilePath).suffix() == "h" || (QFileInfo(rulesFilePath).suffix() == "hpp"))
        filePorter.port(QString::null, inFile, QString::null, inFile, FilePorter::Header );
    else
        filePorter.port(QString::null, inFile, QString::null, inFile, FilePorter::Source );
    return 0;
}

int projectMode(QString inFile)
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
    cout << "-h        Display this help" << endl;
    cout << "-f file   Specify the location for the rules file." << endl;
    cout << endl;
    cout << "The porting documentation contains more information on how " << endl;
    cout << "to use qt3to4 as wall as general porting information." << endl;
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

    if(argc > 4) {
        usage(argv);
        return 0;
    }

    QString in = argv[1];
    if (in == "--help" || in == "/h" || in == "-help"
        || in == "-h"  || in == "-?" || in == "/?") {
        usage(argv);
        return 0;
    }

    QString inFileName;
    if(in == "-f") {
        if(argc < 4) {
            usage(argv);
            return 0;
        }
        rulesFilePath = argv[2];
        //Set defualtRulesFileName here so we can reference
        //it when printing the "not found" error.
        defualtRulesFileName = argv[2];
        inFileName = argv[3];
    } else {
        if(argc != 2) {
            usage(argv);
            return 0;
        }
        rulesFilePath = findRulesFile(defualtRulesFileName, argv[0]);
        inFileName = in;
    }

    //Check if we have a rules file
    if (!QFile::exists(rulesFilePath)) {
        cout << "Error: Could not find rules file: ";
        cout << defualtRulesFileName.toLocal8Bit().constData() << endl;
        cout << "Please try setting the QTDIR environment variable," << endl;
        cout << "or specifying the file with the -f option" << endl;
        return 0;
    } else {
        cout << "Using rules file: ";
        cout << QDir::convertSeparators(rulesFilePath).toLocal8Bit().constData() <<endl;
    }

    //check if we have an infile
    if (!QFile::exists(inFileName)) {
        cout << "Could not find infile: ";
        cout << QDir::convertSeparators(inFileName).toLocal8Bit().constData() <<endl;
        return 0;
    }

    PortingRules::createInstance(rulesFilePath);

    //determine mode and do the port
    int retval;
    if(in.endsWith(".pro") || in.endsWith(".pri"))
        retval = projectMode(inFileName);
    else
        retval = fileMode(inFileName);

    //write log
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

    Logger::deleteInstance();
    PortingRules::deleteInstance();
    return retval;
}

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
#include <QApplication>
#include <QLibraryInfo>

#include "projectporter.h"
#include "fileporter.h"
#include "logger.h"
using std::cout;
using std::endl;

QString rulesFileName;
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

    FilePorter filePorter(rulesFilePath);
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
    cout << "Usage: " << argv[0] << " infile.cpp/h/pro/pri" << endl;
    cout << "Tool for porting Qt 3 applications to Qt 4, using the compatibility library" << endl;
    cout << "and compatibility functions in the core library." << endl;
    cout << endl;
    cout << "qt3to4 has two usage modes: " << endl;
    cout << "* File mode:     qt3to4 infile.cpp/h" << endl;
    cout << "* Project mode:  qt3to4 infile.pro/pri " << endl;
    cout << endl;
    cout << "In file mode a single file is ported, while in project mode all files specified" << endl;
    cout << "in the .pro or .pri file are ported." << endl;
    cout << endl;
    cout << "See README for more info." << endl;
}

int main(int argc, char**argv)
{
    QApplication app(argc, argv);
    applicationDirPath = app.applicationDirPath();

    QString in;
    QString out;
    if(argc !=2) {
        usage(argv);
        return 0;
    }

    in = argv[1];
    if (in == "--help" || in == "/h" || in == "-help" || in == "-h"
        || in == "-?" || in == "/?") {
        usage(argv);
        return 0;
    }

    rulesFileName="q3porting.xml";
    rulesFilePath=findRulesFile(rulesFileName, argv[0]);
    if (rulesFilePath.isEmpty()) {
        cout << "Error: Could not find rules file: " << rulesFileName.toLocal8Bit().constData() << endl;
        cout << "Please try setting the QTDIR environment variable" << endl;
        return 0;
    } else {
        cout << "Using rules file: " << QDir::convertSeparators(rulesFilePath).toLocal8Bit().constData() <<endl;
    }


    int retval;
    if(in.endsWith(".pro") || in.endsWith(".pri"))
        retval = projectMode(in);
    else
        retval = fileMode(in);

    QStringList report = Logger::instance()->fullReport();
    QString logFileName =  "portinglog.txt";
    cout << "Writing log to " << logFileName.toLocal8Bit().constData() << endl;
    QByteArray logContents;
    QBuffer logBuffer(&logContents);
    logBuffer.open(QIODevice::Text | QIODevice::WriteOnly);
    QTextStream logStream(&logBuffer);
    foreach(QString logLine, report) {
//temporary workarond for missing Translate functionality in QTextStream
#ifdef Q_OS_WIN
        logStream << logLine << "\r\n";
#else
        logStream << logLine << endl;
#endif
    }
    FileWriter fileWriter(FileWriter::AskOnOverWrite, "Overwrite file ");
    fileWriter.writeFile(logFileName, logContents);

    Logger::deleteInstance();
    return retval;
}

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

#include "projectporter.h"
#include "fileporter.h"
#include "logger.h"
using std::cout;
using std::endl;

QString rulesFileName;
QString rulesFilePath;

/*
    Rules for findng rules.xml
    1. look in current path
    2. look in program path
    3. look in $QTDIR/tools/porting/ 
    (4. use built in resource file.) (not implemented)
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
        filePath = QString (qgetenv("QTDIR")) + "/tools/porting/" + fileName;
        QFile f(filePath);
        if (!f.exists())
            filePath=QString();
    }
    return QFileInfo(filePath).absoluteFilePath();
}

int fileMode(QString inFile)
{
    QFileInfo inFileInfo(inFile);
    if(!inFileInfo.exists()) {
        cout << "Could not find file " << inFile.latin1() << endl;
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
        cout<<"Could not find file " << inFile.latin1() << endl;
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

    rulesFileName="rules.xml";
    rulesFilePath=findRulesFile(rulesFileName, argv[0]);
    if (rulesFilePath.isEmpty()) {
        cout <<"Error: Could not find " << rulesFileName.latin1() << " file" << endl;
        return 0;
    } else {
        cout << "Using rules file: " << QDir::convertSeparators(rulesFilePath).latin1() <<endl;
    }


    int retval;
    if(in.endsWith(".pro") || in.endsWith(".pri"))
        retval = projectMode(in);
    else
        retval = fileMode(in);
  
    QStringList report = Logger::instance()->cronologicalReport();
    QString logFileName =  "portinglog.txt";
    cout << "Writing log to " << logFileName.latin1() << endl;
    QByteArray logContents;
    QBuffer logBuffer(&logContents);
    logBuffer.open(QIODevice::Translate | QIODevice::WriteOnly);
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

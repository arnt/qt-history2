#include <iostream>

#include <stdio.h>
#include <QString>
#include <QFile>
#include <QFileInfo>

#include "projectporter.h"
#include "fileporter.h"
#include "logger.h"

using std::cout;
using std::endl;

QString rulesFileName;
QString rulesFilePath;

QString findRulesFile(QString fileName, QString programPath)
{
    QString filePath;

    QFile f(fileName);
    if (f.exists()) {
        filePath=fileName;
    } else {
        QString programFileName =  QFileInfo(programPath).fileName();
        programPath.chop(programFileName.size());
        filePath = programPath + fileName;
        QFile f(filePath);
        if (!f.exists())
            filePath=QString();
    }
    return filePath;
}

int fileMode(QString inFile)
{
    QFileInfo inFileInfo(inFile);
    if(!inFileInfo.exists()) {
        cout << "Could not find file" << inFile.latin1() << endl;
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
        cout<<"Could not find file" << inFile.latin1() << endl;
        return 1;
    }
 
    ProjectPorter porter(rulesFilePath);
    porter.portProject(inFileInfo.path(), inFileInfo.fileName());
    return 0;
}

void usage(char **argv)
{
    using namespace std;
    cout << "Usage: " << argv[0] << " infile.cpp/h/pro" << endl;
    cout << "Tool for porting Qt 3 applications to Qt 4, using the compatibility library" << endl;
    cout << "and compatibility functions in the core library." << endl;
    cout << endl;
    cout << "Port has two usage modes: " << endl;
    cout << "* File mode:     port infile.cpp/h" << endl;
    cout << "* Project mode:  port infile.pro " << endl;
    cout << endl;
    cout << "In file mode a single file is ported, while in project mode all files specified" << endl;
    cout << "in the .pro file is ported." << endl;
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
        printf("Error: Could not find rules.xml file\n");
        return 0;
    } else {
        printf("Using rules file: %s\n", rulesFilePath.latin1());
    }


    int retval;
    if(in.endsWith(".pro"))
        retval = projectMode(in);
    else
        retval = fileMode(in);

    Logger::instance()->print(Logger::instance()->cronologicalReport());
    QString logFileName =  "portinglog.txt";
//    printf("Writing log to %s\n", logFileName.latin1());
//    Logger::instance()->writeToFile(logFileName, Logger::instance()->cronologicalReport());

    Logger::deleteInstance();
    return retval;
}



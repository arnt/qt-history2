#include <iostream>

#include <stdio.h>
#include <QString>
#include <QFile>
#include <QFileInfo>

#include "projectporter.h"
#include "fileporter.h"
#include "logger.h"

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

int fileMode(QString inFile, QString outFile)
{
    puts("Port in file mode");
    QFileInfo inFileInfo(inFile);
    if(!inFileInfo.exists()) {
        printf("Could not find file %s\n",inFile.latin1());
        return 1;
    }
    printf("Porting %s, writing output to ", inFile.latin1());
    if (!outFile.isEmpty())
        printf("%s\n", outFile.latin1());
    else
        printf("stdout\n");

    FilePorter filePorter(rulesFilePath);
    if (QFileInfo(rulesFilePath).suffix() == "h" || (QFileInfo(rulesFilePath).suffix() == "hpp"))
        filePorter.port(QString::null, inFile, QString::null, outFile, FilePorter::Header );
    else
        filePorter.port(QString::null, inFile, QString::null, outFile, FilePorter::Source );
    return 0;
}

int projectMode(QString inFile, QString outDir)
{
    puts("Port in project mode");
    QFileInfo inFileInfo(inFile);
    QString inFilePath = inFileInfo.canonicalFilePath();
    if(inFilePath.isEmpty()) {
        printf("Could not find file %s\n",inFile.latin1());
        return 1;
    }
    QString inFileName = inFileInfo.fileName();
    int fileNamePos = inFilePath.indexOf(inFileName);
    QString dir = inFilePath.remove(fileNamePos, inFileName.size());
    printf("Porting project specified in %s\n", inFileName.latin1());
    printf("Project directory: %s\n", dir.latin1());
    printf("Output directory: %s \n", outDir.latin1());

    ProjectPorter porter(rulesFilePath);
    porter.portProject(inFilePath, inFileName, outDir);
    return 0;
}

void usage(char **argv)
{
    using namespace std;
    cout << "Usage: " << argv[0] << " infile.cpp/h outfile.cpp/h" << endl;
    cout << "       " << argv[0] << " infile.pro outdir" << endl;
    cout << "Tool for porting Qt 3 applications to Qt 4, using the compatibility library" << endl;
    cout << "and compatibility functions in the core library." << endl;
    cout << endl;
    cout << "Port has two usage modes: " << endl;
    cout << "* File mode:     port infile.cpp/h out_file" << endl;
    cout << "* Project mode:  port infile.pro   out_directory" << endl;
    cout << endl;
    cout << "See README for more info." << endl;
}

int main(int argc, char**argv)
{
    QString in;
    QString out;
    if(argc !=3) {
        usage(argv);
        return 0;
    }

    in = argv[1];
    if (in == "--help" || in == "/h" || in == "-help" || in == "-h"
        || in == "-?" || in == "/?") {
        usage(argv);
        return 0;
    }

    if(argc==3) out=argv[2];

    rulesFileName="rules.xml";
    rulesFilePath=findRulesFile(rulesFileName, argv[0]);
    if (rulesFilePath.isEmpty()) {
        printf("Error: Could not find rules.xml file\n");
        return 0;
    } else {
        printf("found rules file: %s\n", rulesFilePath.latin1());
    }


    int retval;
    if(in.endsWith(".pro"))
        retval = projectMode(in, out);
    else
        retval = fileMode(in, out);

    Logger::instance()->print(Logger::instance()->cronologicalReport());
    QString logFileName =  "portinglog.txt";
//    printf("Writing log to %s\n", logFileName.latin1());
//    Logger::instance()->writeToFile(logFileName, Logger::instance()->cronologicalReport());

    Logger::deleteInstance();
    return retval;
}



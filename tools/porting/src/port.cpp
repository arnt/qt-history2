#include <stdio.h>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <stdio.h>
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


int main(int argc, char**argv)
{
    QString in;
    QString out;
    if(argc==1 || argc >3) {
        puts("Port: The Qt porting program\n");
        puts("Port has two usage modes: ");
        puts("* File mode:     port infile.cpp/h [out_file]");
        puts("* Project mode:  port infile.pro   [out_directory]\n");

        printf("The out arguments are optional, the ported file(s) will\n");
        printf("printed to standard out if not specified.\n\n");
        printf("See the readme file for more info");  
        return 0;
    }
    
    in=argv[1];
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



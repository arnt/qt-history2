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
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QBuffer>
#include "proparser.h"
#include "textreplacement.h"
#include "fileporter.h"
#include "logger.h"
#include "translationunit.h"
#include "codemodelattributes.h"

using namespace TokenEngine;

ProjectPorter::ProjectPorter(QString basePath, QStringList includeDirectories, QString qt3HeadersFilename)
:basePath(basePath)
,includeDirectories(includeDirectories)
,defaultDefinitions(defaultMacros(preprocessorCache))
,filePorter(preprocessorCache)
,qt3HeadersFilename(qt3HeadersFilename)
,analyze(true)
,warnings(false)
{}

void ProjectPorter::enableCppParsing(bool enable)
{
    analyze = enable;
}

void ProjectPorter::enableMissingFilesWarnings(bool enable)
{
    warnings = enable;
}

void ProjectPorter::portProject(QString fileName)
{
    QFileInfo fileInfo(fileName);
    portProject(fileInfo.path(), fileInfo.fileName());
}

/*
    Port a single file
*/
void ProjectPorter::portFile(QString fileName)
{
    if (analyze) {
        IncludeFiles includeFiles(basePath, includeDirectories);

        PreprocessorController preprocessor(includeFiles, preprocessorCache, qt3HeadersFilename);
        connect(&preprocessor, SIGNAL(error(QString, QString)), SLOT(error(QString, QString)));

        Rpp::DefineMap definitionsCopy = *defaultDefinitions;
        // Preprocess
        TokenSectionSequence translationUnit = preprocessor.evaluate(fileName, &definitionsCopy);
        // Parse
        TranslationUnit translationUnitData = TranslationUnitAnalyzer().analyze(translationUnit);

        // Enable attribute generation for this file.
        enableAttributes(includeFiles, fileName);
        // Generate attributes.
        CodeModelAttributes().createAttributes(translationUnitData);
    }

    portFiles(QString(), QStringList() << fileName);
}

void ProjectPorter::error(QString type, QString text)
{
   if (warnings && type == "Error")
        cout << "Warning: " << text.toLocal8Bit().constData() << endl;
}

void ProjectPorter::portProject(QString basePath, QString proFileName)
{
    QString fullInFileName = basePath + "/" + proFileName;
    QFileInfo infileInfo(fullInFileName);
    if (!infileInfo.exists()) {
        cout << "Could not open file: " << QDir::convertSeparators(fullInFileName).toLocal8Bit().constData() << endl;
        return;
    }

    QString proFileContents = loadFile(fullInFileName);
    QMap<QString, QString> proFileMap = proFileTagMap(proFileContents, QDir(basePath).absolutePath());


    // Check if this is a TEMPLATE = subdirs .pro file, in that case we
    // process each subdir (recursively).

    QString templateTag = proFileMap["TEMPLATE"];
    if (templateTag == "subdirs") {
        QStringList subdirs = proFileMap["SUBDIRS"].split(" ", QString::SkipEmptyParts);
        foreach(QString subdir, subdirs) {
            QString newBasePath  = basePath + "/" + subdir;
            QStringList dirsInSubdir = subdir.split(QRegExp("/|\\\\"), QString::SkipEmptyParts);
            QString newProFileName = dirsInSubdir.last() + ".pro";
            portProject(newBasePath, newProFileName);
        }
        return;
    }

    // Get headers and sources file names from .pro file.
    QStringList sources = proFileMap["SOURCES"].split(" ", QString::SkipEmptyParts);
    QStringList headers = proFileMap["HEADERS"].split(" ", QString::SkipEmptyParts);
    QStringList forms = proFileMap["FORMS"].split(" ", QString::SkipEmptyParts);
    QStringList uidoth;
    for (int i = 0; i < forms.size(); ++i) {
        QString ui_h = forms.at(i) + ".h";
        if (QFile::exists(basePath + "/" + ui_h))
            uidoth += ui_h;
    }

    if (analyze) {
        cout << "Parsing";
        // Get include paths from the pro file.
        QStringList includeProPaths = proFileMap["INCLUDEPATH"].split(" ", QString::SkipEmptyParts);
        IncludeFiles includeFiles(basePath, includeDirectories + includeProPaths);

        PreprocessorController preprocessorController(includeFiles, preprocessorCache, qt3HeadersFilename);
        connect(&preprocessorController, SIGNAL(error(QString, QString)), SLOT(error(QString, QString)));

        TranslationUnitAnalyzer translationUnitAnalyzer;
        CodeModelAttributes codeModelAttributes;

        // Enable attribute generation for header files.
        foreach(QString headerFile, headers)
            enableAttributes(includeFiles, headerFile);

        // Enable attribute generation for ui.h files.
        foreach(QString headerFile, uidoth)
            enableAttributes(includeFiles, headerFile);

        // Analyze each translation unit. (one per cpp file)
        foreach(QString sourceFile, sources) {
            cout << "." << std::flush;
            Rpp::DefineMap definitionsCopy = *defaultDefinitions;
            TokenSectionSequence translationUnit =
                preprocessorController.evaluate(sourceFile, &definitionsCopy);
            TranslationUnit translationUnitData =
                translationUnitAnalyzer.analyze(translationUnit);

            // Enable attribute generation for this file.
            enableAttributes(includeFiles, sourceFile);

            codeModelAttributes.createAttributes(translationUnitData);
        }
        cout << endl;
    }


    // Port files.
    portFiles(basePath, sources);
    portFiles(basePath, headers);
    if (!uidoth.isEmpty())
        portFiles(basePath, uidoth);

    Logger::instance()->globalState["currentFileName"] = proFileName;
    Logger::instance()->beginSection();
    portProFile(fullInFileName, proFileMap);
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
        if (!fullFilePathInfo.exists()) {
            cout << "Could not find file:" <<
                QDir::convertSeparators(fullFilePath).toLocal8Bit().constData() <<endl;
            continue;
        }

        if (!processedFilesSet.contains(fullFilePath)){
            Logger::instance()->globalState["currentFileName"] = fullFilePath;
            filePorter.port(fullFilePath);
            processedFilesSet.insert(fullFilePath);
        }
    }
}

void ProjectPorter::portProFile(QString fileName, QMap<QString, QString> tagMap)
{
    // Read pro file.
    QFile proFile(fileName);
    if (!proFile.open(QIODevice::ReadOnly))
        return;
    QTextStream proTextStream(&proFile);
    QStringList lines;
    while (!proTextStream.atEnd())
        lines += proTextStream.readLine();

    proFile.close();

    // Find out what modules we should add to the QT variable.
     QSet<QByteArray> qtModules;

    // Add qt3support to the Qt tag
    qtModules.insert("qt3support");

    // Read CONFIG and add other modules.
    QStringList config = tagMap["CONFIG"].split(" ", QString::SkipEmptyParts);
    if (config.contains("opengl"))
        qtModules.insert("opengl");
    if (config.contains("xml"))
        qtModules.insert("xml");
    if (config.contains("sql"))
        qtModules.insert("sql");
    if (config.contains("network"))
        qtModules.insert("network");

    // Get set of used modules from the file porter.
    qtModules += filePorter.usedQtModules();

    // Remove gui and core.
    qtModules.remove("gui");
    qtModules.remove("core");

    // Qt3Support is already added.
    qtModules.remove("3support");

    // Remove modules already present in the QT variable.
    QStringList qt = tagMap["QT"].split(" ", QString::SkipEmptyParts);
    foreach(QString name, qt) {
        qtModules.remove(name.toLatin1());
    }

    Logger *logger = Logger::instance();
    bool changesMade = false;

    if (!qtModules.isEmpty()) {
        changesMade = true;
        QString insertText = "QT += ";
        foreach(QByteArray module, qtModules) {
            insertText += module + QLatin1Char(' ');
        }
        lines += QString("#The following line was inserted by qt3to4");
        lines += insertText;
         QString logText = "In file "
                        + logger->globalState.value("currentFileName")
                        + ": Added entry "
                        + insertText;
        logger->addEntry(new PlainLogEntry("Info", "Porting", logText));
    }

    // Add uic3 if we have forms.
    if (!tagMap["FORMS"].isEmpty() || !tagMap["INTERFACES"].isEmpty()) {
        changesMade = true;
        lines += QString("#The following line was inserted by qt3to4");
        QString insertText = "CONFIG += uic3\n";
        lines += insertText;
        QString logText = "In file "
                        + logger->globalState.value("currentFileName")
                        + ": Added entry "
                        + insertText;
        logger->addEntry(new PlainLogEntry("Info", "Porting", logText));
    }

    // Comment out any REQUIRES tag.
    if (!tagMap["REQUIRES"].isEmpty()) {
        changesMade = true;
        QString insertText("#The following line was commented out by qt3to4");
        for (int i = 0; i < lines.count(); ++i) {
            if (lines.at(i).startsWith("REQUIRES")) {
                QString lineCopy = lines.at(i);
                lineCopy.prepend('#');
                lines[i] = lineCopy;
                lines.insert(i, insertText);
                ++i; //skip ahead, we just insertet a line at i.
                QString logText = "In file "
                            + logger->globalState.value("currentFileName")
                            + ": Commented out REQUIRES section";
                logger->addEntry(new PlainLogEntry("Info", "Porting", logText));
            }
        }
    }

    // Check if any changes has been made.
    if (!changesMade) {
        Logger::instance()->addEntry(
            new PlainLogEntry("Info", "Porting",  QLatin1String("No changes made to file ") + fileName));
        Logger::instance()->commitSection();
        return;
    }

    // Write lines to array.
    QByteArray bob;
    QTextStream outProFileStream(&bob);
    foreach(QString line, lines)
        outProFileStream << line << QLatin1Char('\n');
    outProFileStream.flush();

    // Write array to file, commit log if write was successful.
    if (FileWriter::instance()->writeFileVerbously(fileName , bob)) {
        Logger::instance()->commitSection();
    } else {
        Logger::instance()->revertSection();
        Logger::instance()->addEntry(
            new PlainLogEntry("Error", "Porting",  QLatin1String("Error writing to file ") + fileName));
    }
}

/*
    Enables attribute generation for fileName. The file is looked up using the
    provied includeFiles object.
*/
void ProjectPorter::enableAttributes(const IncludeFiles &includeFiles, const QString &fileName)
{
    QString resolvedFilePath = includeFiles.resolve(fileName);
    if (!QFile::exists(resolvedFilePath))
            resolvedFilePath = includeFiles.angleBracketLookup(fileName);
    if (!QFile::exists(resolvedFilePath))
        return;

    TokenContainer tokenContainer = preprocessorCache.sourceTokens(resolvedFilePath);
    TokenAttributes *attributes = tokenContainer.tokenAttributes();
    attributes->addAttribute("CreateAttributes", "True");
}

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
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QDebug>
#include "preprocessorcontrol.h"
#include "fileporter.h"
#include "replacetoken.h"
#include "logger.h"
#include "tokenizer.h"

using namespace TokenEngine;
using namespace Rpp;
using std::cout;
using std::endl;

FilePorter::FilePorter(PreprocessorCache &preprocessorCache)
:preprocessorCache(preprocessorCache)
,tokenReplacementRules(PortingRules::instance()->getTokenReplacementRules())
,replaceToken(tokenReplacementRules)
{
    foreach(QString headerName, PortingRules::instance()->getHeaderList(PortingRules::Qt4)) {
        qt4HeaderNames.insert(headerName.toLatin1());
    }
}

/*
    Ports a file given by fileName, which should be an aboslute file path.
*/
void FilePorter::port(QString fileName)
{
    //Get file tokens from cache.
    TokenContainer sourceTokens = preprocessorCache.sourceTokens(fileName);
    if(sourceTokens.count() == 0)
        return;

    Logger::instance()->beginSection();

    //Perform token replacements.
    QByteArray portedContents =
        replaceToken.getTokenTextReplacements(sourceTokens).apply(sourceTokens.fullText());

    //This step needs to be done after the token replacements, since
    //we need to know which new class names that has been inserted in the source.
    portedContents = includeAnalyse(portedContents);

    //check if any changes has been made.
    if(portedContents == sourceTokens.fullText()) {
        Logger::instance()->addEntry(
            new PlainLogEntry("Info", "Porting",  QLatin1String("No changes made to file ") + fileName));
        Logger::instance()->commitSection();
        return;
    }

    //Write file, commit log if write was successful
    if (FileWriter::instance()->writeFileVerbously(fileName, portedContents)) {
        Logger::instance()->commitSection();
    } else {
        Logger::instance()->revertSection();
        Logger::instance()->addEntry(
            new PlainLogEntry("Error", "Porting",  QLatin1String("Error writing to file ") + fileName));
    }
}

QSet<QByteArray> FilePorter::usedQtModules()
{
    return m_usedQtModules;
}

QByteArray FilePorter::includeAnalyse(QByteArray fileContents)
{
    // Tokenize file contents agein, since it has changed.
    QVector<TokenEngine::Token> tokens  = tokenizer.tokenize(fileContents);
    TokenEngine::TokenContainer tokenContainer(fileContents, tokens);
    IncludeDirectiveAnalyzer includeDirectiveAnalyzer(tokenContainer);

    // Get list of used classes.
    QSet<QByteArray> classes = includeDirectiveAnalyzer.usedClasses();

    // Iterate class list and find which modules are used. This info is used elswhere
    // by when porting the .pro file.
    const QHash<QByteArray, QByteArray> classLibraryList = PortingRules::instance()->getClassLibraryList();
    foreach(const QByteArray className, classes) {
        m_usedQtModules.insert(classLibraryList.value(className));
    }

    // Get list of included headers.
    QSet<QByteArray> headers = includeDirectiveAnalyzer.includedHeaders();

    // Find classes that is missing an include directive and that has a needHeader rule.
    const QHash<QByteArray, QByteArray> neededHeaders = PortingRules::instance()->getNeededHeaders();
    QList<QByteArray> insertHeaders;
    foreach(const QByteArray className, classes) {
        if (!headers.contains(className.toLower() + ".h") &&
            !headers.contains(className)) {
            const QByteArray insertHeader = neededHeaders.value(className);
            if (insertHeader != QByteArray())
                insertHeaders.append("#include <" + insertHeader + ">");
        }
    }

    // Insert include directives undeclared classes.
    int insertCount = insertHeaders.count();
    if (insertCount > 0) {
        QByteArray insertText;
        QByteArray logText;

        insertText += "//Added by qt3to4:\n";
        logText += "In file ";
        logText += Logger::instance()->globalState.value("currentFileName");
        logText += ": Added the following include directives:\n";
        foreach (QByteArray headerName, insertHeaders) {
            insertText = insertText + headerName + "\n";
            logText += "\t";
            logText += headerName + " ";
        }

        const int insertLine = 0;
        Logger::instance()->updateLineNumbers(insertLine, insertCount + 1);
        const int insertPos = includeDirectiveAnalyzer.insertPos();
        fileContents.insert(insertPos, insertText);
        Logger::instance()->addEntry(new PlainLogEntry("Info", "Porting", logText));
    }

    return fileContents;
}

IncludeDirectiveAnalyzer::IncludeDirectiveAnalyzer(const TokenEngine::TokenContainer &fileContents)
:fileContents(fileContents)
{
    const QVector<Type> lexical = RppLexer().lex(fileContents);
    source = Preprocessor().parse(fileContents, lexical, &mempool);
    foundInsertPos = false;
    foundQtHeader = false;
    ifSectionCount = 0;
    insertTokenIndex = 0;

    evaluateItem(source);
}

/*
    Returns a position indicating where new include directives should be inserted.
*/
int IncludeDirectiveAnalyzer::insertPos()
{
    const TokenEngine::Token insertToken = fileContents.token(insertTokenIndex);
    return insertToken.start;
}

/*
    Returns a set of all headers included from this file.
*/
QSet<QByteArray> IncludeDirectiveAnalyzer::includedHeaders()
{
    return m_includedHeaders;
}

/*
    Returns a list of used Qt classes.
*/
QSet<QByteArray> IncludeDirectiveAnalyzer::usedClasses()
{
    return m_usedClasses;
}

/*
    Set insetionTokenindex to a token near other #include directives, preferably
    just after a block of include directives that includes other Qt headers.
*/
void IncludeDirectiveAnalyzer::evaluateIncludeDirective(const IncludeDirective *directive)
{
    const QByteArray filename = directive->filename();
    if (filename.isEmpty())
        return;

    m_includedHeaders.insert(filename);

    if (foundInsertPos || ifSectionCount > 1)
        return;

    const bool isQtHeader = (filename.at(0) == 'q' || filename.at(0) == 'Q');
    if (!isQtHeader && foundQtHeader) {
        foundInsertPos = true;
        return;
    }

    if (isQtHeader)
        foundQtHeader = true;

    // Get the last token for this directive.
    TokenEngine::TokenSection tokenSection = directive->text();
    const int newLineToken = 1;
    insertTokenIndex = tokenSection.containerIndex(tokenSection.count() - 1) + newLineToken;
}

/*
    Avoid inserting inside IfSections, except in the first one
    we see, which probably is the header multiple inclusion guard.
*/
void IncludeDirectiveAnalyzer::evaluateIfSection(const IfSection *ifSection)
{
    ++ifSectionCount;
    RppTreeWalker::evaluateIfSection(ifSection);
    --ifSectionCount;
}

/*
    Read all IdTokens and look for Qt class names.  Also, on
    the first IdToken set foundInsertPos to true

*/
void IncludeDirectiveAnalyzer::evaluateText(const Text *textLine)
{
    const int numTokens = textLine->count();
    for (int t = 0; t < numTokens; ++t) {
        Rpp::Token *token = textLine->token(t);
        if (token->toIdToken()) {
            foundInsertPos = true;
            const int containerIndex = token->index();
            const QByteArray tokenText = fileContents.text(containerIndex);
            if (tokenText[0] == 'Q')
                m_usedClasses.insert(tokenText);
        }
    }
}

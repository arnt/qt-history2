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
#include "preprocessorcontrol.h"
#include "fileporter.h"
#include "replacetoken.h"
#include "logger.h"
#include "tokenizer.h"

using namespace TokenEngine;
using std::cout;
using std::endl;


QByteArray FilePorter::noPreprocess(const QString &filePath)
{
    QFile f(filePath);
    f.open(QIODevice::ReadOnly);
    return f.readAll();
}

FilePorter::FilePorter(QString rulesfilename, PreprocessorCache &preprocessorCache)
:portingRules(rulesfilename)
,preprocessorCache(preprocessorCache)
,tokenReplacementRules(portingRules.getNoPreprocessPortingTokenRules())
,replaceToken(tokenReplacementRules)
{
    int dummy=0;
    foreach(QString headerName, portingRules.getHeaderList(PortingRules::Qt4)) {
        qt4HeaderNames.insert(headerName, dummy);
    }
}

void FilePorter::port(QString inBasePath, QString inFilePath, QString outBasePath, QString outFilePath, FileType fileType )
{
    QString fullInFileName = inBasePath + inFilePath;
    QFileInfo infileInfo(fullInFileName);
    if(!infileInfo.exists()) {
        cout<< "Could not open file: " << fullInFileName.toLocal8Bit().constData() << endl;
        return;
    }

    TokenContainer sourceTokens = preprocessorCache.sourceTokens(fullInFileName);
    Logger::instance()->globalState["currentFileName"] = inFilePath;
    QByteArray portedContents =
        replaceToken.getTokenTextReplacements(sourceTokens).apply(sourceTokens.fullText());

    //This step needs to be done after the token replacements, since
    //we need to know which new class names that has been inserted in the source
    portedContents = includeAnalyse(portedContents, fileType);

    if(!outFilePath.isEmpty()) {
        QString fullOutfileName = outBasePath + outFilePath;
        FileWriter::instance()->writeFileVerbously(fullOutfileName, portedContents);
    }
}

QByteArray FilePorter::includeAnalyse(QByteArray fileContents, FileType /*fileType*/)
{
    QList<TokenEngine::Token> tokens  = tokenizer.tokenize(fileContents);
    TokenEngine::TokenContainer tokenContainer(fileContents, tokens);

    QMap<QByteArray, int> classes;
    QMap<QByteArray, int> headers;

    //Get list of used classes and included headers
    int t = 0;
    const int numTokens = tokenContainer.count();
    while(t < numTokens )
    {
        QByteArray tokenText = tokenContainer.text(t);
        if(tokenText[0] =='Q' && qt4HeaderNames.contains(tokenText))
            classes.insert(tokenText, 0);
        else if(tokenText.startsWith("#include")) {
            QString tokenString(tokenText);
            QStringList subTokenList = tokenString.split(QRegExp("<|>|\""), QString::SkipEmptyParts);
            foreach(QString subToken, subTokenList) {
                if(subToken[0] == 'Q' || subToken[0] == 'q')
                    headers.insert(subToken.toLatin1(), 0);
            }
        }
        ++t;
    }

    //compare class and header names, find classes that lacks a
    //curresponding include directive
    QStringList neededHeaders = portingRules.getNeededHeaderList();
    QStringList headersToInsert;
    QMapIterator<QByteArray, int> c(classes);
    //loop through classes
    while (c.hasNext()) {
        c.next();
        bool foundHeader=false;
        QMapIterator<QByteArray, int> h(headers);
        //loop through headers
        while (h.hasNext() && !foundHeader) {
            h.next();
            if(h.key().toLower().startsWith(c.key().toLower()))
                foundHeader=true; //we found a header for class c
        }
        if(!foundHeader){
            //we have a class without a coresponding header.
            bool needHeader=false;
            //loop through list of headers
            foreach(QString header, neededHeaders) {
                //compare class name with header
                //TODO: assumes that only new-style headers are specified in the
                //rules. This is ok for now.
                if(header.toLatin1() == c.key()) {
                    needHeader=true;
                    break;
                }
            }
            if(needHeader) {
                headersToInsert.push_back(QString("#include <" + c.key() + ">"));
             //   printf("Must insert header file for class  %s \n ", c.key().constData());
            }
        }
    }

    //insert headers in files, at the end of the first block of
    //include files
    //TODO: make this more intelligent by not inserting inside #ifdefs
    bool includeEnd = false;
    bool includeStart = false;
    t = 0;
    while(t < numTokens && !includeEnd)
    {
        QByteArray tokenText = tokenContainer.text(t);
        if(tokenText.trimmed().startsWith("#include"))
            includeStart=true;
        else if(includeStart &&(!tokenText.trimmed().isEmpty() || tokenText == "\n" ))
            includeEnd=true;
        ++t;
    }
    /*
    //back up a bit to get just at the end of the last include
    while(inStream->currentTokenText().trimmed())  ) {
        inStream->rewind(inStream->cursor()-1);
    }
    */
    int insertPos=0;
    if(includeStart != false)
        insertPos = tokenContainer.token(t).start;

    int insertCount = headersToInsert.count();
    if(insertCount>0) {
        QByteArray insertText;
        QByteArray logText;

        insertText+="\n//Added by the Qt porting tool:\n";
        logText += "In file ";
        logText += Logger::instance()->globalState.value("currentFileName");
        logText += ": Added the following include directives:\n";
        foreach(QString headerName, headersToInsert) {
            insertText = insertText + headerName.toLatin1() + "\n";
            logText +="\t";
            logText += headerName.toLatin1() + " ";
        }
        insertText+="\n";
        fileContents.insert(insertPos, insertText);
        Logger::instance()->addEntry(new PlainLogEntry("Info", "Porting", logText));
    }

    return fileContents;
}

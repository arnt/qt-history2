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

#include "tokenreplacements.h"
#include <stdio.h>
#include <iostream>
#include "logger.h"
#include "portingrules.h"


using std::cout;
using std::endl;


void TokenReplacement::makeLogEntry(QString text, TokenStream *tokenStream)
{
    Logger *logger = Logger::instance();
    int line;
    int col;
    tokenStream->positionAtAux(tokenStream->token().position, &line, &col);
    SourcePointLogEntry *logEntry =
                new SourcePointLogEntry("Info", "Porting",
                                        logger->globalState.value("currentFileName"),
                                        line, col, text);
    logger->addEntry(logEntry);
}



IncludeTokenReplacement::IncludeTokenReplacement(QByteArray fromFile, QByteArray toFile)
:fromFile(fromFile)
,toFile(toFile)
{  }

bool IncludeTokenReplacement::doReplace(TokenStream *tokenStream, TextReplacements &textReplacements)
{
    QByteArray tokenText=tokenStream->currentTokenText();
    if(tokenText.startsWith("#include")) {
    //     printf("Include token: %s Matching aganinst %s \n", tokenText.constData(), fromFile.constData() );
        int pos=tokenText.indexOf(fromFile);
        if(pos!=-1) {
    //      printf("a match was made\n");
            Token token = tokenStream->token();
            makeLogEntry(tokenText + " -> " + toFile, tokenStream);
            textReplacements.insert(toFile, token.position+pos, fromFile.size());
            return true;
        }
    }
    return false;

}

/////////////////////
GenericTokenReplacement::GenericTokenReplacement(QByteArray oldToken, QByteArray newToken)
:oldToken(oldToken)
,newToken(newToken)
{}

QByteArray GenericTokenReplacement::getReplaceKey()
{
    return QByteArray(oldToken);
}

bool GenericTokenReplacement::doReplace(TokenStream *tokenStream, TextReplacements &textReplacements)
{
    QByteArray tokenText=tokenStream->currentTokenText();
    if(tokenText==oldToken){
        Token token = tokenStream->token();
        makeLogEntry(tokenText + " -> " + newToken, tokenStream);
        textReplacements.insert(newToken, token.position, tokenText.size());
        return true;
    }
    return false;

}

///////////////////

ScopedTokenReplacement::ScopedTokenReplacement(QByteArray oldToken,
                                               QByteArray newToken)
:oldToken(oldToken)
,newToken(newToken)
{}

bool ScopedTokenReplacement::doReplace(TokenStream *tokenStream, TextReplacements &textReplacements)
{
    QByteArray tokenText=tokenStream->currentTokenText();
//     qDebug("Scoped token: Matching (%s -> %s) against %s", oldToken.constData(),newToken.constData(), tokenText.constData());
    QByteArray oldTokenName;
    QByteArray oldTokenScope;
    if (oldToken.contains("::")) {
        oldTokenName = oldToken.mid(oldToken.lastIndexOf(':')+1);
        oldTokenScope = oldToken.mid(0, oldToken.indexOf(':'));
    } else {
        oldTokenName = oldToken;
    }

    //check token text
    if(tokenText == oldTokenName ){
        //check scope (if any)
        if(!oldTokenScope.isEmpty()) {
            int scopeTokenIndex = getNextScopeToken(tokenStream, tokenStream->cursor());
            if(scopeTokenIndex  != -1) {
                //token in tokenStream is qualified
                QByteArray scopeText = tokenStream->tokenText(scopeTokenIndex);
//                 qDebug("scopeText=%s oldTokenScope=%s", scopeText.data(), oldTokenScope.data());
                if(scopeText != oldTokenScope) {
                    // special case! if oldTokenScope is Qt, meaning the Qt class,
                    // we check if scopeText is one of the Qt3 classes that inherits Qt.
                    // This will cach cases such as QWidget::ButtonState, wich will be
                    // renamed to Qt:ButtonState
                    if(oldTokenScope == "Qt") {
                        if (!PortingRules::instance()->getInheritsQt().contains(scopeText))
                            return false;    //false alarm, scopeText is not a Qt class
                     } else {
                         return false;
                     }
                }
                Token token = tokenStream->token();
                if (newToken.contains("::")) {
                    QByteArray newTokenName = newToken.mid(newToken.lastIndexOf(':')+1);
                    QByteArray newTokenScope = newToken.mid(0, newToken.indexOf(':'));
                    if(newTokenScope == scopeText){
                        //the old and new scopes are equal, replace name part only
                        if (tokenText == newTokenName) //names are equal, no need to do anything
                            return true;
                        makeLogEntry(tokenText + " -> " + newTokenName, tokenStream);
                        textReplacements.insert(newTokenName, token.position, tokenText.size());
                        return true;
                    } else {
                        //replace scope and name
                        makeLogEntry(tokenText + " -> " + newToken, tokenStream);
                        Token scopeToken = tokenStream->tokenAt(scopeTokenIndex);
                        textReplacements.insert(newTokenScope, scopeToken.position, scopeText.size());
                        textReplacements.insert(newTokenName, token.position, tokenText.size());
                        return true;
                    }
                } else {
                    makeLogEntry(scopeText + "::" + tokenText + " -> " + newToken, tokenStream);
                    Token scopeToken = tokenStream->tokenAt(scopeTokenIndex);
                    textReplacements.insert(newToken, scopeToken.position, token.position + tokenText.size() - scopeToken.position);
                }
            } else {
                QByteArray newTokenScope = newToken.mid(0, newToken.indexOf(':'));
                // ##### this is a bit hacky. We just try to avoid replacements of
                // e.g. Vertical with QSizePolicy::Vertically. Unqualified tokens
                // can't happen for classes one does not usually inherit from, so
                // we only let them pass for stuff that people usually inherited from.
                if (newTokenScope != "Qt"
                    && newTokenScope != "QFrame"
                    && newTokenScope != "QValidator")
                    return false;
                // token in the tokenStream is not qualified
                // relplace token with qualified new token
                Token token = tokenStream->token();
                makeLogEntry(tokenText + " -> " + newToken, tokenStream);
                textReplacements.insert(newToken, token.position, tokenText.size());
                return true;
            }
        } else {
            //oldToken is not qualified - This won't happen. (use a plain TokenReplacement
            //for unqualified tokens)

            //relplace token with (non-qualified) new token
            /*
            Token token = tokenStream->token();
            makeLogEntry("ScopedReplace", tokenText + " -> " + newToken, tokenStream);
            textReplacements.insert(newToken, token.position, tokenText.size());
            return true;
            */
        }
    }

    return false;
}

QByteArray ScopedTokenReplacement::getReplaceKey()
{
    if (oldToken.contains("::")) {
        return oldToken.mid(oldToken.lastIndexOf(':')+1);
    } else {
        return oldToken;
    }
}

/*
    looks for "::" backwards from startTokenIndex, returns the token index
    for it if found. If the first non-whitespace token found is something else,
    -1 is returned
*/
int ScopedTokenReplacement::findScopeOperator(TokenStream *tokenStream, int startTokenIndex)
{
   int tokenIndex=startTokenIndex;
   QByteArray tokenText;
   //loop until we get a token containg text or we pass the start of the source
   while(tokenText.isEmpty() && tokenIndex>0) {
       --tokenIndex;
       tokenText = tokenStream->tokenText(tokenIndex).trimmed();
   }
   if(tokenText=="::")
       return tokenIndex;
   else
       return -1;
}

/*
    This function "unwinds" a qualified name like QString::null, by returning
    the token index for the next class/namespace in the qualifier chain. Thus, if null
    is the start token, getNextScopeToken will return QString's index.

    returns -1 if the end of the qualifier chain is found.
*/
int ScopedTokenReplacement::getNextScopeToken(TokenStream *tokenStream, int startTokenIndex)
{
    int tokenIndex  = findScopeOperator(tokenStream, startTokenIndex);
    if (tokenIndex == -1)
        return -1;
    QByteArray tokenText;
   //loop until we get a token containg text or we pass the start of the source
    while(tokenText.isEmpty() && tokenIndex>0) {
       --tokenIndex;
       tokenText = tokenStream->tokenText(tokenIndex).trimmed();
    }
    return tokenIndex;
}



#include "tokenreplacements.h"
#include <logger.h>
#include <stdio.h>

void TokenReplacement::makeLogEntry(QString group, QString text, TokenStream *tokenStream)
{
    Logger *logger =  Logger::instance();
    int line;
    int col;
    tokenStream->positionAtAux(tokenStream->token().position, &line, &col);
    logger->addEntry(group, text, QString(), line, col);
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
            makeLogEntry("IncludeReplace", tokenText + " -> " + toFile, tokenStream);
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
        makeLogEntry("GenericReplace", tokenText + " -> " + newToken, tokenStream);
        textReplacements.insert(newToken, token.position, tokenText.size());
        return true;
    }
    return false;

}

///////////////////

ScopedTokenReplacement::ScopedTokenReplacement(QByteArray oldToken, QByteArray newToken)

:oldToken(oldToken)
,newToken(newToken)
{}

bool ScopedTokenReplacement::doReplace(TokenStream *tokenStream, TextReplacements &textReplacements)
{
    QByteArray tokenText=tokenStream->currentTokenText();
    //printf("Scoped token: Matching %s aganinst %s \n", tokenText.constData(), oldToken.constData() );
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
                if(scopeText != oldTokenScope) {
                    return false;    //scope din not match, so we return
                } else {
                    //token in the tokenStream is qualified, oldToken is qualified,
                    //and the qualifiers match. replace token with qualified new token    
                    Token token = tokenStream->token();
                    QByteArray newTokenName;
                    QByteArray newTokenScope;
                    if (newToken.contains("::")) {
                        newTokenName = newToken.mid(newToken.lastIndexOf(':')+1);
                        newTokenScope = newToken.mid(0, newToken.indexOf(':'));
                        if(newTokenScope == oldTokenScope){
                            //the old and new scopes are equal, replace name part only
                            makeLogEntry("ScopedReplace", tokenText + " -> " + newTokenName, tokenStream);
                            textReplacements.insert(newTokenName, token.position, tokenText.size());
                            return true;
                        } else {
                            //replace scope and name
                            makeLogEntry("ScopedReplace", tokenText + " -> " + newToken, tokenStream);
                            Token scopeToken = tokenStream->tokenAt(scopeTokenIndex);
                            textReplacements.insert(newTokenScope, scopeToken.position, scopeText.size());
                            textReplacements.insert(newTokenName, token.position, tokenText.size());
                            return true;
                        }
                    } else {
                        newTokenName = newToken;
                    }
                }
            } else {
                // token in the tokenStream is not qualified
                //relplace token with qualified new token
                Token token = tokenStream->token();
                makeLogEntry("ScopedReplace", tokenText + " -> " + newToken, tokenStream);
                textReplacements.insert(newToken, token.position, tokenText.size());
                return true;
            }
        } else {
            //oldToken is not qualified
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
   //loop until we get a token containg text
   while(tokenText.isEmpty()) {
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
   //loop until we get a token containg text
    while(tokenText.isEmpty()) {
       --tokenIndex;
       tokenText = tokenStream->tokenText(tokenIndex).trimmed();
    }
    return tokenIndex;
}



/*
bool ScopedTokenReplacement::findPreTextToken(QByteArray scopeName, TokenStream *tokenStream)
{
    QByteArray preTextToken;
    int count=1;
    while(preTextToken.isEmpty() && tokenStream->cursor()- count > 0) {
        QByteArray tokenText=tokenStream->tokenText(tokenStream->cursor()-count);
        QByteArray trimmedToken = tokenText.trimmed();
        if(!trimmedToken.isEmpty() )
            preTextToken=tokenText.trimmed();
        ++count;
    }
    return (preTextToken==scopeName);
}
*/

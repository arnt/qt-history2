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
#include "logger.h"
#include "portingrules.h"
#include <iostream>
using namespace TokenEngine;
using namespace std;

void TokenReplacement::addLogSourceEntry(const QString &text, const TokenContainer &tokenContainer, const int index) const
{
    Q_UNUSED(tokenContainer);
    Q_UNUSED(index);
     Logger *logger = Logger::instance();
    //TODO: figue ut how to get line/col from a tokenContainer
    int line = 0;
    int col = 0;;
    SourcePointLogEntry *logEntry =
                new SourcePointLogEntry("Info", "Porting",
                                        logger->globalState.value("currentFileName"),
                                        line, col, text);
    logger->addEntry(logEntry);
}


void TokenReplacement::addLogWarning(const QString &text) const
{
     Logger::instance()->addEntry(new PlainLogEntry("Warning", "Porting", text));
}

QualifiedNameParser::QualifiedNameParser(const TokenContainer &tokenContainer, const int tokenIndex)
:tokenContainer(tokenContainer)
,currentIndex(tokenIndex)
{
    Q_ASSERT(isValidIndex(currentIndex));
}

bool QualifiedNameParser::isPartOfQualifiedName()
{
    return ((nextScopeToken(Left) != -1) || (nextScopeToken(Right) != -1));
}


bool QualifiedNameParser::isValidIndex(int index)
{
    return (index < tokenContainer.count() && index >= 0);
}

/*
    A qualifier is a the leftmost or middle part of a qualified name
*/
bool QualifiedNameParser::isQualifier()
{
    return (nextScopeToken(Right) != -1);
}

/*
    A name is a the rightmost part of a qualified name.
*/
bool QualifiedNameParser::isName()
{
    return (nextScopeToken(Left) != -1);
}

/*
    Peek for a qualifier or name in the given direction
*/
int QualifiedNameParser::peek(Direction direction)
{
    return nextScopeToken(direction);
}

/*
    Look for a qualifier or name in the given direction,update
    current position if found.
*/
int QualifiedNameParser::move(Direction direction)
{
    int tokenIndex = nextScopeToken(direction);
    if(tokenIndex != -1)
        currentIndex = tokenIndex;
    return tokenIndex;
}

/*
    Looks for "::" starting at currentIndex, returns the token index
    for it if found. If the first non-whitespace token found is something else,
    -1 is returned.
*/
int QualifiedNameParser::findScopeOperator(Direction direction)
{
    int tokenIndex = currentIndex;
    QByteArray tokenText;
    //loop until we get a token containg text or we pass the beginning/end of the source
    tokenIndex += direction;
    while(tokenText.isEmpty() && isValidIndex(tokenIndex)) {
        tokenText = tokenContainer.text(tokenIndex).trimmed();
        if(tokenText=="::")
           return tokenIndex;
        tokenIndex += direction;
    }
    return -1;
}
/*
    Walks a qualified name. Returns the token index
    for the next identifer in the qualified name, or -1 if its not found.
*/
int QualifiedNameParser::nextScopeToken(Direction direction)
{
    int tokenIndex  = findScopeOperator(direction);
    if (tokenIndex == -1)
        return -1;
    QByteArray tokenText;
   //loop until we get a token containg text or we pass the start of the source
    tokenIndex += direction;
    while(tokenText.isEmpty() && isValidIndex(tokenIndex)) {
       tokenText = tokenContainer.text(tokenIndex).trimmed();
       tokenIndex += direction;
    }
    return tokenIndex - direction;
}

IncludeTokenReplacement::IncludeTokenReplacement(QByteArray fromFile, QByteArray toFile)
:fromFile(fromFile)
,toFile(toFile)
{  }

bool IncludeTokenReplacement::doReplace(const TokenContainer &tokenContainer,
                                        int index, TextReplacements &textReplacements)
{
    QByteArray tokenText;
    // read a line of tokens, index points to a "#" token
    int currentIndex = index;
    while(currentIndex < tokenContainer.count()) {
        QByteArray newText = tokenContainer.text(currentIndex);
        if(newText == "\n")
            break;
        tokenText += newText;
        ++currentIndex;
    }
    if(tokenText.startsWith("#") && tokenText.contains("include") ) {
        int pos = tokenText.indexOf(fromFile);
        if(pos!=-1) {
            addLogSourceEntry(tokenText + " -> " + toFile, tokenContainer, index);
            TokenEngine::Token token = tokenContainer.token(index);
            textReplacements.insert(toFile, token.start + pos, fromFile.size());
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

bool GenericTokenReplacement::doReplace(const TokenContainer &tokenContainer,
                            int index, TextReplacements &textReplacements)
{
    QByteArray tokenText = tokenContainer.text(index);
    if(tokenText == oldToken){
        addLogSourceEntry(tokenText + " -> " + newToken, tokenContainer, index);
        TokenEngine::Token token = tokenContainer.token(index);
        textReplacements.insert(newToken, token.start, token.length);
        return true;
    }
    return false;

}

///////////////////
ClassNameReplacement::ClassNameReplacement(QByteArray oldToken, QByteArray newToken)
:oldToken(oldToken)
,newToken(newToken)
{}

QByteArray ClassNameReplacement::getReplaceKey()
{
    return QByteArray(oldToken);
}

/*
    Replace a class name token. If the class name is a scope specifier (a "qualifier")
    in a qualified name, we check if qualified name will be replaced by a porting rule.
    If so, we don't do the class name replacement.
*/
bool ClassNameReplacement::doReplace(const TokenContainer &tokenContainer, int index, TextReplacements &textReplacements)
{
    QByteArray tokenText = tokenContainer.text(index);
    if(tokenText != oldToken)
        return false;

    QualifiedNameParser nameParser(tokenContainer, index);
    if(nameParser.isPartOfQualifiedName() &&
       nameParser.peek(QualifiedNameParser::Right) != -1) {
        int nameTokenIndex = nameParser.peek(QualifiedNameParser::Right);
        QByteArray name = tokenContainer.text(nameTokenIndex);

        TextReplacements textReplacements;
        QList<TokenReplacement*> tokenReplacements
            = PortingRules::instance()->getTokenReplacementRules();
        bool changed = false;
        foreach(TokenReplacement *tokenReplacement, tokenReplacements) {
            changed = tokenReplacement->doReplace(tokenContainer, nameTokenIndex, textReplacements);
            if(changed)
                break;
        }
        if(changed)
            return false;
    }
    addLogSourceEntry(tokenText + " -> " + newToken, tokenContainer, index);
    TokenEngine::Token token = tokenContainer.token(index);
    textReplacements.insert(newToken, token.start, token.length);
    return true;
}

///////////////////

ScopedTokenReplacement::ScopedTokenReplacement(QByteArray oldToken,
                                               QByteArray newToken)
:oldToken(oldToken)
,newToken(newToken)
{}

bool ScopedTokenReplacement::doReplace(const TokenContainer &tokenContainer, int index, TextReplacements &textReplacements)
{
    if (oldToken.contains("::") == false) {
        addLogWarning("Warning: in ScopedTokenReplacement::doReplace(): token "
                            + oldToken + " is not a scoped token");
        return false;
    }

    QByteArray oldTokenName = oldToken.mid(oldToken.lastIndexOf(':')+1);
    QByteArray oldTokenScope = oldToken.mid(0, oldToken.indexOf(':'));
    Token token = tokenContainer.token(index);
    QByteArray tokenText = tokenContainer.text(index);

    if(tokenText != oldTokenName)
        return false;

    QualifiedNameParser nameParser(tokenContainer, index);

    if(nameParser.isPartOfQualifiedName() == false)
    {
        QByteArray newTokenScope = newToken.mid(0, newToken.indexOf(':'));
        // ##### this is a bit hacky. We just try to avoid replacements of
        // e.g. Vertical with QSizePolicy::Vertically. Unqualified tokens
        // can't happen for classes one does not usually inherit from, so
        // we only let them pass for stuff that people usually inherited from.
        if (newTokenScope != "Qt"
            && newTokenScope != "QFrame"
            && newTokenScope != "QValidator")
            return false;

        addLogSourceEntry(tokenText + " -> " + newToken, tokenContainer, index);
        textReplacements.insert(newToken, token.start, tokenText.size());
        return true;
    }

    if(nameParser.isName() == false) {
        //This is a pretty special case, it means that in a qualified
        //name like aaa::bbb the replacement rule has been triggered for
        //the aaa part. Since this is not what we'd normally use a
        //ScopedReplacement for, we just return here.
        return false;
    }

    int scopeTokenIndex = nameParser.peek(QualifiedNameParser::Left);
    if (scopeTokenIndex == -1 ) {
        addLogWarning( "Warning: Internal error in ScopedTokenReplacement::doReplace(): \
                        Could not find scope for token " + tokenText);
        return false;
    }

    Token scopeToken = tokenContainer.token(scopeTokenIndex);
    QByteArray scopeText = tokenContainer.text(scopeTokenIndex);

    if(scopeText != oldTokenScope) {
        // special case! if oldTokenScope is Qt, meaning the Qt class,
        // we check if scopeText is one of the Qt3 classes that inherits Qt.
        // This will cach cases such as QWidget::ButtonState, wich will be
        // renamed to Qt::ButtonState
        if(oldTokenScope != "Qt")
            return false;
        if (!PortingRules::instance()->getInheritsQt().contains(scopeText)) //TODO optimize: linear search
            //false alarm, scopeText is not a Qt class
            return false;
    }

    if (newToken.count("::") != 1 || newToken.contains("(") ) {
        //Spcecial cases, such as QIODevice::Offset -> Q_LONGLONG
        //or Qt::WType_Modal -> (Qt::WType_Dialog | Qt::WShowModal)
        addLogSourceEntry(scopeText + "::" + tokenText + " -> " + newToken, tokenContainer, index);
        int beginPos = scopeToken.start;
        int endPos = token.start - scopeToken.start + token.length;
        textReplacements.insert(newToken, beginPos, endPos);
        return true;
    }
    //The rest of the code expects that newToken contains one and only one "::"
    QByteArray newTokenName = newToken.mid(newToken.lastIndexOf(':')+1);
    QByteArray newTokenScope = newToken.mid(0, newToken.indexOf(':'));
    if(newTokenScope == scopeText){
        //the old and new scopes are equal, replace name part only
        if (tokenText == newTokenName)
            //names are equal, no need to do anything
            return true;
        addLogSourceEntry(tokenText + " -> " + newTokenName, tokenContainer, index);
        textReplacements.insert(newTokenName, token.start, token.length);
        return true;
    } else {
        //replace scope and name
        addLogSourceEntry(tokenText + " -> " + newToken, tokenContainer, index);
        textReplacements.insert(newTokenScope, scopeToken.start, scopeToken.length);
        textReplacements.insert(newTokenName, token.start, token.length);
        return true;
    }
}

QByteArray ScopedTokenReplacement::getReplaceKey()
{
    if (oldToken.contains("::")) {
        return oldToken.mid(oldToken.lastIndexOf(':')+1);
    } else {
        return oldToken;
    }
}


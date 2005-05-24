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
    int line = tokenContainer.line(index);
    int col = tokenContainer.column(index);
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
        if(newText == "\n" || newText == "\r\n")
            break;
        tokenText += newText;
        ++currentIndex;
    }

    // Also match cases where the header name contains
    // capital letters (ex. #include <QWidget.h>)
    tokenText = tokenText.toLower();

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

ScopedTokenReplacement::ScopedTokenReplacement(const QByteArray &oldToken,
                                               const QByteArray &newToken)
:newScopedName(newToken)
{
    Q_ASSERT(oldToken.contains("::"));

    // Split oldToken into scope and name parts.
    oldName = oldToken.mid(oldToken.lastIndexOf(':')+1);
    oldScope = oldToken.mid(0, oldToken.indexOf(':'));

    // Split newToken into scope and name parts, execept if we have a spcial
    // case like Qt::WType_Modal -> (Qt::WType_Dialog | Qt::WShowModal)
    if (newToken.count("::") != 1 || newToken.contains("(")) {
        newName = newToken;
    } else {
        newName = newToken.mid(newToken.lastIndexOf(':')+1);
        newScope = newToken.mid(0, newToken.indexOf(':'));
    }
}

bool ScopedTokenReplacement::doReplace(const TokenContainer &tokenContainer, int sourceIndex, TextReplacements &textReplacements)
{
    const QByteArray sourceName = tokenContainer.text(sourceIndex);

    // Check if the token texts matches.
    if (sourceName != oldName)
        return false;

    // Get token attributes. The attributes are created by the the C++ parser/analyzer.
    const TokenAttributes *attributes = tokenContainer.tokenAttributes();
    // If the declaration attribute is set we don't replace.
    if (!attributes->attribute(sourceIndex, "declaration").isEmpty())
        return false;
    // If the unknown (undeclared) attribute is set we don't replace.
    if (!attributes->attribute(sourceIndex, "unknown").isEmpty())
        return false;
    // If nameUse is set we test if the nameUse refers to the correct declaration.
    // This is done by checking the parentScope attriute, wich returns the scope
    // for the declaration associated with this name use.
    const bool haveNameUseInfo = !attributes->attribute(sourceIndex, "nameUse").isEmpty();
    if (haveNameUseInfo)
        if (attributes->attribute(sourceIndex, "parentScope") != oldScope)
            return false;

    // If we get here that means that the nameUse info was correct, or that it
    // is missing. If it is missing we assume that the C++ parsing failed for
    // some reason and just go ahead and replace.

    // The token might have a qualifier, and in that case we need to check if
    // we should replace the qualifier as well.
    QualifiedNameParser nameParser(tokenContainer, sourceIndex);

    // This is a pretty special case, it means that in a qualified
    // name like aaa::bbb the replacement rule has been triggered for
    // the aaa part. Since this is not what we'd normally use a
    // ScopedReplacement for, we just return here.
    if (nameParser.isQualifier())
        return false;

    // If the token is unqualified, just replace it.
    if (!nameParser.isPartOfQualifiedName()) {
        // If we have no name use info we try to avoid replacements of
        // e.g. Vertical with QSizePolicy::Vertically. Unqualified tokens
        // can't happen for classes one does not usually inherit from, so
        // we only let them pass for stuff that people usually inherited from.
        if (!haveNameUseInfo && newScope != "Qt" && newScope != "QFrame" && newScope != "QValidator")
            return false;

        const Token sourceToken = tokenContainer.token(sourceIndex);
        addLogSourceEntry(sourceName + " -> " + newScopedName, tokenContainer, sourceIndex);
        textReplacements.insert(newScopedName, sourceToken.start, sourceName.size());
        return true;
    }

    // Peek left for the qualifer token.
    const int sourceScopeIndex = nameParser.peek(QualifiedNameParser::Left);
    if (sourceScopeIndex == -1) {
        return false;
    }

    const Token sourceNameToken = tokenContainer.token(sourceIndex);
    const Token sourceScopeToken = tokenContainer.token(sourceScopeIndex);
    const QByteArray sourceScope = tokenContainer.text(sourceScopeIndex);

    // If we have no name use info and the source and old scopes don't match,
    // we generally dont't do a replace, unless the old scope is Qt and
    // the source scope inherits Qt. For example, QWidget::ButtonState should
    // be renamed to Qt::ButtonState.
    if (!haveNameUseInfo && sourceScope != oldScope) {
        if (oldScope != "Qt")
            return false;
        // Check if sourceScope inherits the Qt class.
        if (!PortingRules::instance()->getInheritsQt().contains(sourceScope)) //TODO optimize: linear search
            return false;
    }

    // Spcecial cases, such as QIODevice::Offset -> Q_LONGLONG
    // or Qt::WType_Modal -> (Qt::WType_Dialog | Qt::WShowModal).
    if (newScope.isEmpty()) {
        addLogSourceEntry(sourceScope + "::" + sourceName + " -> " + newScopedName, tokenContainer, sourceIndex);
        const int qualiferLength = sourceNameToken.start - sourceScopeToken.start;
        const int length = qualiferLength + sourceNameToken.length;
        textReplacements.insert(newName, sourceScopeToken.start, length);
        return true;
    }

    // If the old and new scopes are equal, we replace the name part only.
    if (newScope == sourceScope) {
        // If the names are equal, there is no need to do anything.
        if (newName == sourceName)
            return true;
        addLogSourceEntry(sourceName + " -> " + newName, tokenContainer, sourceIndex);
        textReplacements.insert(newName, sourceNameToken.start, sourceNameToken.length);
        return true;
    }

    // If the names are equal, replace scope only.
    if (newName == sourceName) {
        addLogSourceEntry(sourceScope + " -> " + newScope, tokenContainer, sourceScopeIndex);
        textReplacements.insert(newScope, sourceScopeToken.start, sourceScopeToken.length);
        return true;
    }

    // Replace scope and name.
    addLogSourceEntry(sourceScope + "::" + sourceName + " -> " + newScopedName, tokenContainer, sourceScopeIndex);
    textReplacements.insert(newScope, sourceScopeToken.start, sourceScopeToken.length);
    textReplacements.insert(newName, sourceNameToken.start, sourceNameToken.length);
    return true;
}

QByteArray ScopedTokenReplacement::getReplaceKey()
{
   return oldName;
}


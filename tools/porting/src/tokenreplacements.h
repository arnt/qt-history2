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

#ifndef TOKENREPLACEMENTS_H
#define TOKENREPLACEMENTS_H

#include <QStringList>
#include <QByteArray>
#include "tokenengine.h"
#include "textreplacement.h"

class TokenReplacement
{
public:
    virtual bool doReplace(const TokenEngine::TokenContainer& ,
                           int /*tokenIndex*/,
                           TextReplacements&){return false;};
    /*
        returns the replace key for this replacement. Every time a token matches the replace key,
        doReplace() will be called for this TokenReplacement.
    */
    virtual QByteArray getReplaceKey(){return QByteArray();};
    virtual ~TokenReplacement(){};
protected:
    void addLogSourceEntry(const QString &text, const TokenEngine::TokenContainer&, const int index) const;
    void addLogWarning(const QString &text) const;
};

/*
    A TokenReplacement that changes #include directives
*/
class IncludeTokenReplacement : public TokenReplacement
{
public:
    IncludeTokenReplacement(QByteArray fromFile, QByteArray toFile);
 //   bool doReplace(QByteArray tokenText, QByteArray &newTokenText);
    bool doReplace(const TokenEngine::TokenContainer &tokenContainer,
                   int tokenIndex, TextReplacements &textReplacements);
private:
    QByteArray fromFile;
    QByteArray toFile;
};

/*
    A TokenReplacement that change any token
*/
class GenericTokenReplacement : public TokenReplacement
{
public:
    GenericTokenReplacement(QByteArray oldToken, QByteArray newToken);
    bool doReplace(const TokenEngine::TokenContainer &tokenContainer,
                   int tokenIndex, TextReplacements &textReplacements);
    QByteArray getReplaceKey();
private:
    QByteArray oldToken;
    QByteArray newToken;
};

/*
    A TokenReplacement that changes tokens that specify class names.
    In some cases where the class name token is a part of a qualified name
    it is not correct to rename it. ex:

    QButton::toggleState

    Here it is wrong to rename QButton -> Q3Button, since there is
    a rule that says QButton::ToggleState -> QCheckBox::ToggleState,
    but no rule for Q3Button::ToggleState.
*/
class ClassNameReplacement : public TokenReplacement
{
public:
    ClassNameReplacement(QByteArray oldToken, QByteArray newToken);
    bool doReplace(const TokenEngine::TokenContainer &tokenContainer,
                   int tokenIndex, TextReplacements &textReplacements);
    QByteArray getReplaceKey();
private:
    QByteArray oldToken;
    QByteArray newToken;
};

/*
   Changes scoped tokens:
   AA::BB -> CC::DD
   oldToken corresponds to the AA::BB part, newToken corresponds CC::DD.
   Since this is a token replacement, the AA part of oldToken is typically
   unknown. This means that we might change tokens named BB that does not belong
   to the AA scope. Ast replacemnts will fix this.

*/
class ScopedTokenReplacement : public TokenReplacement
{
public:
    ScopedTokenReplacement(const QByteArray &oldToken, const QByteArray &newToken);
    bool doReplace(const TokenEngine::TokenContainer &tokenContainer,
                   int tokenIndex, TextReplacements &textReplacements);
    QByteArray getReplaceKey();
private:
    QByteArray oldName;
    QByteArray oldScope;
    QByteArray newName;
    QByteArray newScope;
    QByteArray newScopedName;
};

class QualifiedNameParser
{
public:
    QualifiedNameParser(const TokenEngine::TokenContainer &tokenContainer,
                        const int tokenIndex);
    enum Direction { Left=-1, Right=1 };
    bool isPartOfQualifiedName();
    bool isValidIndex(int index);
    bool isQualifier();
    bool isName();
    int peek(Direction direction);
    int move(Direction direction);
private:
    int nextScopeToken(Direction direction);
    int findScopeOperator(Direction direction);
    const TokenEngine::TokenContainer tokenContainer;
    int currentIndex;
};

#endif

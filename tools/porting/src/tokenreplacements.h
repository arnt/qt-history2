#ifndef TOKENREPLACEMENTS_H
#define TOKENREPLACEMENTS_H

#include <QByteArray>
#include "lexer.h"    //for TokenStream
#include "textreplacement.h"

class TokenReplacement
{
public:
    virtual bool doReplace(TokenStream * /*tokenStream*/, TextReplacements &/*textReplacements*/){return false;};
    /*
        returns the replace key for this replacement. Every time a token matches the replace key,
        doReplace() will be called for this TokenReplacement.
    */
    virtual QByteArray getReplaceKey(){return QByteArray();};
    virtual ~TokenReplacement(){};
protected:
    void makeLogEntry(QString group, QString text, TokenStream *tokenStream);
};

/*
    A TokenReplacement that changes #include directives
*/
class IncludeTokenReplacement : public TokenReplacement
{
public:
    IncludeTokenReplacement(QByteArray fromFile, QByteArray toFile);
 //   bool doReplace(QByteArray tokenText, QByteArray &newTokenText);
    bool doReplace(TokenStream *tokenStream, TextReplacements &textReplacements);
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
    bool doReplace(TokenStream *tokenStream, TextReplacements &textReplacements);
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
    ScopedTokenReplacement(QByteArray oldToken, QByteArray newToken);
    bool doReplace(TokenStream *tokenStream, TextReplacements &textReplacements);
    QByteArray getReplaceKey();
private:
  //  bool findPreTextToken(QByteArray scopeName, TokenStream *tokenStream);
    int findScopeOperator(TokenStream *tokenStream, int startTokenIndex);
    int getNextScopeToken(TokenStream *tokenStream, int startTokenIndex);
    QByteArray oldToken;
    QByteArray newToken;
};

#endif

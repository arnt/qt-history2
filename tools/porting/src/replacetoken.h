#ifndef PORT_REPLACETOKEN_H
#define PORT_REPLACETOKEN_H

#include <QList>
#include <QMultiMap>
#include <lexer.h>
#include "tokenreplacements.h"
#include "textreplacement.h"

void printTokenStream(TokenStream *stream);
void printContents(QByteArray contents);

class ReplaceToken
{
public:
    ReplaceToken(QList<TokenReplacement*> tokenReplacementRules);
    TextReplacements getTokenTextReplacements(FileSymbol *inFileSymbol);
private:
    bool isInterestingToken(QByteArray token);
    bool isPreprocessorDirective(QByteArray token);
    QMultiMap<QByteArray, TokenReplacement*> tokenRuleLookup;
    QList<TokenReplacement*> tokenReplacementRules;
};


#endif

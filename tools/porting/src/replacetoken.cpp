#include "replacetoken.h"
#include <QByteArray>
#include "tokenreplacements.h"


void printTokenStream(TokenStream *stream)
{
    printf("Printing tokens\n");
    stream->rewind(0);
    while(!stream->tokenAtEnd())
    {
        printf("%s|", stream->currentTokenText().data());
        stream->nextToken();
    }
    stream->rewind(0);
    printf("\n");
}

void printContents(QByteArray contents)
{
    printf(contents.constData());
    printf("\n");
}


ReplaceToken::ReplaceToken(QList<TokenReplacement*> tokenReplacementRules)
:tokenReplacementRules(tokenReplacementRules)
{
    foreach (TokenReplacement* rep, tokenReplacementRules) {
        QByteArray key = rep->getReplaceKey();
        if(!key.isEmpty()) {
    //        printf("Creating replace key: |%s| \n", rep->getReplaceKey().constData());
            tokenRuleLookup.insert(key, rep);
        }
    }
}

TextReplacements ReplaceToken::getTokenTextReplacements(FileSymbol *inFileSymbol)
{
    TokenStream *inStream = inFileSymbol->tokenStream;
    QByteArray  inContents = inFileSymbol->contents;
    TextReplacements textReplacements;
    inStream->rewind(0);

    while(!inStream->tokenAtEnd())
    {
        QByteArray tokenText=inStream->currentTokenText();
        bool changed=false;

        if(isPreprocessorDirective(tokenText)) {
            foreach(TokenReplacement *tokenReplacementRule, tokenReplacementRules) {
                if(!changed)
                    changed = tokenReplacementRule->doReplace(inStream, textReplacements);
                if(changed)
                    break;    
            }
        } else if (isInterestingToken(tokenText.trimmed())) {
            foreach (TokenReplacement* value, tokenRuleLookup.values(tokenText)) {
                changed = value->doReplace(inStream, textReplacements);
                if(changed)
                    break;
            }
        }
        inStream->nextToken();
    }
    return textReplacements;
}

bool ReplaceToken::isInterestingToken(QByteArray text)
{
    return !(text.isEmpty() || text==";" || text=="(" || text==")" || text=="{" || text=="}" || text=="="
            || text=="+=" || text=="-=" || text=="if" || text=="then" || text=="else"
    );
}

bool ReplaceToken::isPreprocessorDirective(QByteArray token)
{
    return (token[0]=='#');
}

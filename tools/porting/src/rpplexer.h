#ifndef RPPLEXER_H
#define RPPLEXER_H

#include <QByteArray>
#include <QList>
#include "tokens.h"
#include "tokenengine.h"
namespace Rpp {

class RppLexer
{
public:

    RppLexer();
    QList<Type> lex(TokenEngine::TokenSequence *tokenSequence);
private:
    Type indentify(QByteArray tokenText);
    void setupScanTable();

    void scanChar(int *kind);
    void scanUnicodeChar(int *kind);
    void scanNewline(int *kind);
    void scanWhiteSpaces(int *kind);
    void scanCharLiteral(int *kind);
    void scanStringLiteral(int *kind);
    void scanNumberLiteral(int *kind);
    void scanIdentifier(int *kind);
    void scanPreprocessor(int *kind);
    void scanComment(int *kind);
    void scanOperator(int *kind);
    void scanKeyword(int *kind);

    typedef void (RppLexer::*scan_fun_ptr)(int *kind);
    RppLexer::scan_fun_ptr s_scan_table[128+1];
    int s_attr_table[256];

    enum
    {
        A_Alpha = 0x01,
        A_Digit = 0x02,
        A_Alphanum = A_Alpha | A_Digit,
        A_Whitespace = 0x04
    };

    QByteArray m_buffer;
    int m_ptr;
};

} //namespace Rpp



#endif

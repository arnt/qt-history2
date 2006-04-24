#include <QtCore>

enum TokenType {
    TOK_LET,
    TOK_LETXX,
    TOK_XTRA,
    TOK_SEQ
};

#include "scanner.cpp"

int main()
{
    QString input("LETXTRA");
    TestScanner scanner(input);
    int tok;
    do {
        tok = scanner.lex();
        if (tok != -1) {
            qDebug() << "scanned" << tok << "lexem" << scanner.lexem();
        }
    } while (tok != -1);
    return 0;
}

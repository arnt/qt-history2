#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <qstring.h>
#include <qvector.h>
#include "token.h"

struct Symbol
{
    Symbol():lineNum(-1),token(NOTOKEN), from(0),len(-1){}
    Symbol(int lineNum, Token token, const QByteArray &lexem, int from = 0, int len=-1):
	lineNum(lineNum), token(token),lexem_data(lexem),from(from), len(len){}
    Symbol(int lineNum, PP_Token token, const QByteArray &lexem, int from = 0, int len=-1):
	lineNum(lineNum), pp_token(token),lexem_data(lexem),from(from), len(len){}
    int lineNum;
    union {Token token; PP_Token pp_token;};
    inline QByteArray lexem() const { return lexem_data.mid(from, len); }
    QByteArray lexem_data;
    int from, len;
};
Q_DECLARE_TYPEINFO(Symbol, Q_MOVABLE_TYPE);

typedef QVector<Symbol> Symbols;

#endif

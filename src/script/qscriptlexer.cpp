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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qscriptengine_p.h"
#include "qscriptlexer_p.h"
#include "qscriptgrammar_p.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define shiftWindowsLineBreak() if(current == '\r' && next1 == '\n') shift(1);


QScript::Lexer::Lexer(QScriptEngine *eng)
    : driver(0),
      yylineno(0),
      size8(128), size16(128), restrKeyword(false),
      extraIdentifiers(false),
      stackToken(-1), pos(0),
      code(0), length(0),
      bol(true),
      current(0), next1(0), next2(0), next3(0),
      check_reserved(true)
{
    if (eng)
        driver = QScriptEnginePrivate::get(eng);
    // allocate space for read buffers
    buffer8 = new char[size8];
    buffer16 = new QChar[size16];
    pattern = flags = 0;

}

QScript::Lexer::~Lexer()
{
    delete [] buffer8;
    delete [] buffer16;
}

void QScript::Lexer::setCode(const QString &c, int lineno)
{
    errmsg = QString();
    yylineno = lineno;
    restrKeyword = false;
    delimited = false;
    stackToken = -1;
    pos = 0;
    code = c.unicode();
    length = c.length();
    bol = true;

    // read first characters
    current = (length > 0) ? code[0].unicode() : 0;
    next1 = (length > 1) ? code[1].unicode() : 0;
    next2 = (length > 2) ? code[2].unicode() : 0;
    next3 = (length > 3) ? code[3].unicode() : 0;
}

void QScript::Lexer::shift(uint p)
{
    while (p--) {
        pos++;
        current = next1;
        next1 = next2;
        next2 = next3;
        next3 = (pos + 3 < length) ? code[pos+3].unicode() : 0;
    }
}

void QScript::Lexer::setDone(State s)
{
    state = s;
    done = true;
}

int QScript::Lexer::findReservedWord(const QChar *c, int size) const
{
    switch (size) {
    case 2: {
        if (c[0] == QLatin1Char('d') && c[1] == QLatin1Char('o'))
            return QScriptGrammar::T_DO;
        else if (c[0] == QLatin1Char('i') && c[1] == QLatin1Char('f'))
            return QScriptGrammar::T_IF;
        else if (c[0] == QLatin1Char('i') && c[1] == QLatin1Char('n'))
            return QScriptGrammar::T_IN;
    }   break;

    case 3: {
        if (c[0] == QLatin1Char('f') && c[1] == QLatin1Char('o') && c[2] == QLatin1Char('r'))
            return QScriptGrammar::T_FOR;
        else if (c[0] == QLatin1Char('n') && c[1] == QLatin1Char('e') && c[2] == QLatin1Char('w'))
            return QScriptGrammar::T_NEW;
        else if (c[0] == QLatin1Char('t') && c[1] == QLatin1Char('r') && c[2] == QLatin1Char('y'))
            return QScriptGrammar::T_TRY;
        else if (c[0] == QLatin1Char('v') && c[1] == QLatin1Char('a') && c[2] == QLatin1Char('r'))
            return QScriptGrammar::T_VAR;
        else if (check_reserved) {
            if (c[0] == QLatin1Char('i') && c[1] == QLatin1Char('n') && c[2] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 4: {
        if (c[0] == QLatin1Char('c') && c[1] == QLatin1Char('a')
                && c[2] == QLatin1Char('s') && c[3] == QLatin1Char('e'))
            return QScriptGrammar::T_CASE;
        else if (c[0] == QLatin1Char('e') && c[1] == QLatin1Char('l')
                && c[2] == QLatin1Char('s') && c[3] == QLatin1Char('e'))
            return QScriptGrammar::T_ELSE;
        else if (c[0] == QLatin1Char('t') && c[1] == QLatin1Char('h')
                && c[2] == QLatin1Char('i') && c[3] == QLatin1Char('s'))
            return QScriptGrammar::T_THIS;
        else if (c[0] == QLatin1Char('v') && c[1] == QLatin1Char('o')
                && c[2] == QLatin1Char('i') && c[3] == QLatin1Char('d'))
            return QScriptGrammar::T_VOID;
        else if (c[0] == QLatin1Char('w') && c[1] == QLatin1Char('i')
                && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('h'))
            return QScriptGrammar::T_WITH;
        else if (!extraIdentifiers && c[0] == QLatin1Char('t') && c[1] == QLatin1Char('r')
                && c[2] == QLatin1Char('u') && c[3] == QLatin1Char('e'))
            return QScriptGrammar::T_TRUE;
        else if (!extraIdentifiers && c[0] == QLatin1Char('n') && c[1] == QLatin1Char('u')
                && c[2] == QLatin1Char('l') && c[3] == QLatin1Char('l'))
            return QScriptGrammar::T_NULL;
        else if (check_reserved) {
            if (c[0] == QLatin1Char('e') && c[1] == QLatin1Char('n')
                    && c[2] == QLatin1Char('u') && c[3] == QLatin1Char('m'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('b') && c[1] == QLatin1Char('y')
                    && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('e'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('l') && c[1] == QLatin1Char('o')
                    && c[2] == QLatin1Char('n') && c[3] == QLatin1Char('g'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('c') && c[1] == QLatin1Char('h')
                    && c[2] == QLatin1Char('a') && c[3] == QLatin1Char('r'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('g') && c[1] == QLatin1Char('o')
                    && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('o'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 5: {
        if (c[0] == QLatin1Char('b') && c[1] == QLatin1Char('r')
                && c[2] == QLatin1Char('e') && c[3] == QLatin1Char('a')
                && c[4] == QLatin1Char('k'))
            return QScriptGrammar::T_BREAK;
        else if (c[0] == QLatin1Char('c') && c[1] == QLatin1Char('a')
                && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('c')
                && c[4] == QLatin1Char('h'))
            return QScriptGrammar::T_CATCH;
        else if (c[0] == QLatin1Char('t') && c[1] == QLatin1Char('h')
                && c[2] == QLatin1Char('r') && c[3] == QLatin1Char('o')
                && c[4] == QLatin1Char('w'))
            return QScriptGrammar::T_THROW;
        else if (c[0] == QLatin1Char('w') && c[1] == QLatin1Char('h')
                && c[2] == QLatin1Char('i') && c[3] == QLatin1Char('l')
                && c[4] == QLatin1Char('e'))
            return QScriptGrammar::T_WHILE;
        else if (!extraIdentifiers && c[0] == QLatin1Char('f') && c[1] == QLatin1Char('a')
                && c[2] == QLatin1Char('l') && c[3] == QLatin1Char('s')
                && c[4] == QLatin1Char('e'))
            return QScriptGrammar::T_FALSE;
        else if (check_reserved) {
            if (c[0] == QLatin1Char('s') && c[1] == QLatin1Char('h')
                    && c[2] == QLatin1Char('o') && c[3] == QLatin1Char('r')
                    && c[4] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('s') && c[1] == QLatin1Char('u')
                    && c[2] == QLatin1Char('p') && c[3] == QLatin1Char('e')
                    && c[4] == QLatin1Char('r'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('f') && c[1] == QLatin1Char('i')
                    && c[2] == QLatin1Char('n') && c[3] == QLatin1Char('a')
                    && c[4] == QLatin1Char('l'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('c') && c[1] == QLatin1Char('l')
                    && c[2] == QLatin1Char('a') && c[3] == QLatin1Char('s')
                    && c[4] == QLatin1Char('s'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('f') && c[1] == QLatin1Char('l')
                    && c[2] == QLatin1Char('o') && c[3] == QLatin1Char('a')
                    && c[4] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('c') && c[1] == QLatin1Char('o')
                    && c[2] == QLatin1Char('n') && c[3] == QLatin1Char('s')
                    && c[4] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 6: {
        if (c[0] == QLatin1Char('d') && c[1] == QLatin1Char('e')
                && c[2] == QLatin1Char('l') && c[3] == QLatin1Char('e')
                && c[4] == QLatin1Char('t') && c[5] == QLatin1Char('e'))
            return QScriptGrammar::T_DELETE;
        else if (c[0] == QLatin1Char('r') && c[1] == QLatin1Char('e')
                && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('u')
                && c[4] == QLatin1Char('r') && c[5] == QLatin1Char('n'))
            return QScriptGrammar::T_RETURN;
        else if (c[0] == QLatin1Char('s') && c[1] == QLatin1Char('w')
                && c[2] == QLatin1Char('i') && c[3] == QLatin1Char('t')
                && c[4] == QLatin1Char('c') && c[5] == QLatin1Char('h'))
            return QScriptGrammar::T_SWITCH;
        else if (c[0] == QLatin1Char('t') && c[1] == QLatin1Char('y')
                && c[2] == QLatin1Char('p') && c[3] == QLatin1Char('e')
                && c[4] == QLatin1Char('o') && c[5] == QLatin1Char('f'))
            return QScriptGrammar::T_TYPEOF;
        else if (check_reserved) {
            if (c[0] == QLatin1Char('e') && c[1] == QLatin1Char('x')
                    && c[2] == QLatin1Char('p') && c[3] == QLatin1Char('o')
                    && c[4] == QLatin1Char('r') && c[5] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('s') && c[1] == QLatin1Char('t')
                    && c[2] == QLatin1Char('a') && c[3] == QLatin1Char('t')
                    && c[4] == QLatin1Char('i') && c[5] == QLatin1Char('c'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('d') && c[1] == QLatin1Char('o')
                    && c[2] == QLatin1Char('u') && c[3] == QLatin1Char('b')
                    && c[4] == QLatin1Char('l') && c[5] == QLatin1Char('e'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('i') && c[1] == QLatin1Char('m')
                    && c[2] == QLatin1Char('p') && c[3] == QLatin1Char('o')
                    && c[4] == QLatin1Char('r') && c[5] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('p') && c[1] == QLatin1Char('u')
                    && c[2] == QLatin1Char('b') && c[3] == QLatin1Char('l')
                    && c[4] == QLatin1Char('i') && c[5] == QLatin1Char('c'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('n') && c[1] == QLatin1Char('a')
                    && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('i')
                    && c[4] == QLatin1Char('v') && c[5] == QLatin1Char('e'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('t') && c[1] == QLatin1Char('h')
                    && c[2] == QLatin1Char('r') && c[3] == QLatin1Char('o')
                    && c[4] == QLatin1Char('w') && c[5] == QLatin1Char('s'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 7: {
        if (c[0] == QLatin1Char('d') && c[1] == QLatin1Char('e')
                && c[2] == QLatin1Char('f') && c[3] == QLatin1Char('a')
                && c[4] == QLatin1Char('u') && c[5] == QLatin1Char('l')
                && c[6] == QLatin1Char('t'))
            return QScriptGrammar::T_DEFAULT;
        else if (c[0] == QLatin1Char('f') && c[1] == QLatin1Char('i')
                && c[2] == QLatin1Char('n') && c[3] == QLatin1Char('a')
                && c[4] == QLatin1Char('l') && c[5] == QLatin1Char('l')
                && c[6] == QLatin1Char('y'))
            return QScriptGrammar::T_FINALLY;
        else if (check_reserved) {
            if (c[0] == QLatin1Char('b') && c[1] == QLatin1Char('o')
                    && c[2] == QLatin1Char('o') && c[3] == QLatin1Char('l')
                    && c[4] == QLatin1Char('e') && c[5] == QLatin1Char('a')
                    && c[6] == QLatin1Char('n'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('e') && c[1] == QLatin1Char('x')
                    && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('e')
                    && c[4] == QLatin1Char('n') && c[5] == QLatin1Char('d')
                    && c[6] == QLatin1Char('s'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('p') && c[1] == QLatin1Char('a')
                    && c[2] == QLatin1Char('c') && c[3] == QLatin1Char('k')
                    && c[4] == QLatin1Char('a') && c[5] == QLatin1Char('g')
                    && c[6] == QLatin1Char('e'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('p') && c[1] == QLatin1Char('r')
                    && c[2] == QLatin1Char('i') && c[3] == QLatin1Char('v')
                    && c[4] == QLatin1Char('a') && c[5] == QLatin1Char('t')
                    && c[6] == QLatin1Char('e'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 8: {
        if (c[0] == QLatin1Char('c') && c[1] == QLatin1Char('o')
                && c[2] == QLatin1Char('n') && c[3] == QLatin1Char('t')
                && c[4] == QLatin1Char('i') && c[5] == QLatin1Char('n')
                && c[6] == QLatin1Char('u') && c[7] == QLatin1Char('e'))
            return QScriptGrammar::T_CONTINUE;
        else if (c[0] == QLatin1Char('f') && c[1] == QLatin1Char('u')
                && c[2] == QLatin1Char('n') && c[3] == QLatin1Char('c')
                && c[4] == QLatin1Char('t') && c[5] == QLatin1Char('i')
                && c[6] == QLatin1Char('o') && c[7] == QLatin1Char('n'))
            return QScriptGrammar::T_FUNCTION;
        else if (check_reserved) {
            if (c[0] == QLatin1Char('a') && c[1] == QLatin1Char('b')
                    && c[2] == QLatin1Char('s') && c[3] == QLatin1Char('t')
                    && c[4] == QLatin1Char('r') && c[5] == QLatin1Char('a')
                    && c[6] == QLatin1Char('c') && c[7] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('d') && c[1] == QLatin1Char('e')
                    && c[2] == QLatin1Char('b') && c[3] == QLatin1Char('u')
                    && c[4] == QLatin1Char('g') && c[5] == QLatin1Char('g')
                    && c[6] == QLatin1Char('e') && c[7] == QLatin1Char('r'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('v') && c[1] == QLatin1Char('o')
                    && c[2] == QLatin1Char('l') && c[3] == QLatin1Char('a')
                    && c[4] == QLatin1Char('t') && c[5] == QLatin1Char('i')
                    && c[6] == QLatin1Char('l') && c[7] == QLatin1Char('e'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 9: {
        if (check_reserved) {
            if (c[0] == QLatin1Char('i') && c[1] == QLatin1Char('n')
                    && c[2] == QLatin1Char('t') && c[3] == QLatin1Char('e')
                    && c[4] == QLatin1Char('r') && c[5] == QLatin1Char('f')
                    && c[6] == QLatin1Char('a') && c[7] == QLatin1Char('c')
                    && c[8] == QLatin1Char('e'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('t') && c[1] == QLatin1Char('r')
                    && c[2] == QLatin1Char('a') && c[3] == QLatin1Char('n')
                    && c[4] == QLatin1Char('s') && c[5] == QLatin1Char('i')
                    && c[6] == QLatin1Char('e') && c[7] == QLatin1Char('n')
                    && c[8] == QLatin1Char('t'))
                return QScriptGrammar::T_RESERVED;
            else if (c[0] == QLatin1Char('p') && c[1] == QLatin1Char('r')
                    && c[2] == QLatin1Char('o') && c[3] == QLatin1Char('t')
                    && c[4] == QLatin1Char('e') && c[5] == QLatin1Char('c')
                    && c[6] == QLatin1Char('t') && c[7] == QLatin1Char('e')
                    && c[8] == QLatin1Char('d'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 10: {
        if (c[0] == QLatin1Char('i') && c[1] == QLatin1Char('n')
                && c[2] == QLatin1Char('s') && c[3] == QLatin1Char('t')
                && c[4] == QLatin1Char('a') && c[5] == QLatin1Char('n')
                && c[6] == QLatin1Char('c') && c[7] == QLatin1Char('e')
                && c[8] == QLatin1Char('o') && c[9] == QLatin1Char('f'))
            return QScriptGrammar::T_INSTANCEOF;
        else if (check_reserved) {
            if (c[0] == QLatin1Char('i') && c[1] == QLatin1Char('m')
                    && c[2] == QLatin1Char('p') && c[3] == QLatin1Char('l')
                    && c[4] == QLatin1Char('e') && c[5] == QLatin1Char('m')
                    && c[6] == QLatin1Char('e') && c[7] == QLatin1Char('n')
                    && c[8] == QLatin1Char('t') && c[9] == QLatin1Char('s'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    case 12: {
        if (check_reserved) {
            if (c[0] == QLatin1Char('s') && c[1] == QLatin1Char('y')
                    && c[2] == QLatin1Char('n') && c[3] == QLatin1Char('c')
                    && c[4] == QLatin1Char('h') && c[5] == QLatin1Char('r')
                    && c[6] == QLatin1Char('o') && c[7] == QLatin1Char('n')
                    && c[8] == QLatin1Char('i') && c[9] == QLatin1Char('z')
                    && c[10] == QLatin1Char('e') && c[11] == QLatin1Char('d'))
                return QScriptGrammar::T_RESERVED;
        }
    }   break;

    } // switch

    return -1;
}

int QScript::Lexer::lex()
{
    int token = 0;
    state = Start;
    ushort stringType = 0; // either single or double quotes
    pos8 = pos16 = 0;
    done = false;
    terminator = false;

    // did we push a token on the stack previously ?
    // (after an automatic semicolon insertion)
    if (stackToken >= 0) {
        setDone(Other);
        token = stackToken;
        stackToken = 0;
    }

    while (!done) {
        switch (state) {
        case Start:
            if (isWhiteSpace()) {
                // do nothing
            } else if (current == '/' && next1 == '/') {
                shift(1);
                state = InSingleLineComment;
            } else if (current == '/' && next1 == '*') {
                shift(1);
                state = InMultiLineComment;
            } else if (current == 0) {
                if (!terminator && !delimited) {
                    // automatic semicolon insertion if program incomplete
                    token = ';';
                    stackToken = 0;
                    setDone(Other);
                } else
                    setDone(Eof);
            } else if (isLineTerminator()) {
                shiftWindowsLineBreak();
                yylineno++;
                bol = true;
                terminator = true;
                if (restrKeyword) {
                    token = ';';
                    setDone(Other);
                }
            } else if (current == '"' || current == '\'') {
                state = InString;
                stringType = current;
            } else if (isIdentLetter(current)) {
                record16(current);
                state = InIdentifier;
            } else if (current == '0') {
                record8(current);
                state = InNum0;
            } else if (isDecimalDigit(current)) {
                record8(current);
                state = InNum;
            } else if (current == '.' && isDecimalDigit(next1)) {
                record8(current);
                state = InDecimal;
            } else {
                token = matchPunctuator(current, next1, next2, next3);
                if (token != -1)
                    setDone(Other);
                else {
                    setDone(Bad);
                    errmsg = QLatin1String("Illegal character");
                }
            }
            break;
        case InString:
            if (current == stringType) {
                shift(1);
                setDone(String);
            } else if (current == 0 || isLineTerminator()) {
                setDone(Bad);
                errmsg = QLatin1String("Unclosed string at end of line");
            } else if (current == '\\') {
                state = InEscapeSequence;
            } else {
                record16(current);
            }
            break;
            // Escape Sequences inside of strings
        case InEscapeSequence:
            if (isOctalDigit(current)) {
                if (current >= '0' && current <= '3' &&
                     isOctalDigit(next1) && isOctalDigit(next2)) {
                    record16(convertOctal(current, next1, next2));
                    shift(2);
                    state = InString;
                } else if (isOctalDigit(current) &&
                            isOctalDigit(next1)) {
                    record16(convertOctal('0', current, next1));
                    shift(1);
                    state = InString;
                } else if (isOctalDigit(current)) {
                    record16(convertOctal('0', '0', current));
                    state = InString;
                } else {
                    setDone(Bad);
                    errmsg = QLatin1String("Illegal escape squence");
                }
            } else if (current == 'x')
                state = InHexEscape;
            else if (current == 'u')
                state = InUnicodeEscape;
            else {
                record16(singleEscape(current));
                state = InString;
            }
            break;
        case InHexEscape:
            if (isHexDigit(current) && isHexDigit(next1)) {
                state = InString;
                record16(QLatin1Char(convertHex(current, next1)));
                shift(1);
            } else if (current == stringType) {
                record16(QLatin1Char('x'));
                shift(1);
                setDone(String);
            } else {
                record16(QLatin1Char('x'));
                record16(current);
                state = InString;
            }
            break;
        case InUnicodeEscape:
            if (isHexDigit(current) && isHexDigit(next1) &&
                 isHexDigit(next2) && isHexDigit(next3)) {
                record16(convertUnicode(current, next1, next2, next3));
                shift(3);
                state = InString;
            } else if (current == stringType) {
                record16(QLatin1Char('u'));
                shift(1);
                setDone(String);
            } else {
                setDone(Bad);
                errmsg = QLatin1String("Illegal unicode escape sequence");
            }
            break;
        case InSingleLineComment:
            if (isLineTerminator()) {
                shiftWindowsLineBreak();
                yylineno++;
                terminator = true;
                bol = true;
                if (restrKeyword) {
                    token = ';';
                    setDone(Other);
                } else
                    state = Start;
            } else if (current == 0) {
                setDone(Eof);
            }
            break;
        case InMultiLineComment:
            if (current == 0) {
                setDone(Bad);
                errmsg = QLatin1String("Unclosed comment at end of file");
            } else if (isLineTerminator()) {
                shiftWindowsLineBreak();
                yylineno++;
            } else if (current == '*' && next1 == '/') {
                state = Start;
                shift(1);
            }
            break;
        case InIdentifier:
            if (isIdentLetter(current) || isDecimalDigit(current)) {
                record16(current);
                break;
            }
            setDone(Identifier);
            break;
        case InNum0:
            if (current == 'x' || current == 'X') {
                record8(current);
                state = InHex;
            } else if (current == '.') {
                record8(current);
                state = InDecimal;
            } else if (current == 'e' || current == 'E') {
                record8(current);
                state = InExponentIndicator;
            } else if (isOctalDigit(current)) {
                record8(current);
                state = InOctal;
            } else if (isDecimalDigit(current)) {
                record8(current);
                state = InDecimal;
            } else {
                setDone(Number);
            }
            break;
        case InHex:
            if (isHexDigit(current))
                record8(current);
            else
                setDone(Hex);
            break;
        case InOctal:
            if (isOctalDigit(current)) {
                record8(current);
            } else if (isDecimalDigit(current)) {
                record8(current);
                state = InDecimal;
            } else {
                setDone(Octal);
            }
            break;
        case InNum:
            if (isDecimalDigit(current)) {
                record8(current);
            } else if (current == '.') {
                record8(current);
                state = InDecimal;
            } else if (current == 'e' || current == 'E') {
                record8(current);
                state = InExponentIndicator;
            } else {
                setDone(Number);
            }
            break;
        case InDecimal:
            if (isDecimalDigit(current)) {
                record8(current);
            } else if (current == 'e' || current == 'E') {
                record8(current);
                state = InExponentIndicator;
            } else {
                setDone(Number);
            }
            break;
        case InExponentIndicator:
            if (current == '+' || current == '-') {
                record8(current);
            } else if (isDecimalDigit(current)) {
                record8(current);
                state = InExponent;
            } else {
                setDone(Bad);
                errmsg = QLatin1String("Illegal syntax for exponential number");
            }
            break;
        case InExponent:
            if (isDecimalDigit(current)) {
                record8(current);
            } else {
                setDone(Number);
            }
            break;
        default:
            Q_ASSERT_X(0, "Lexer::lex", "Unhandled state in switch statement");
        }

        // move on to the next character
        if (!done)
            shift(1);
        if (state != Start && state != InSingleLineComment)
            bol = false;
    }

    // no identifiers allowed directly after numeric literal, e.g. "3in" is bad
    if ((state == Number || state == Octal || state == Hex)
         && isIdentLetter(current)) {
        state = Bad;
        errmsg = QLatin1String("Identifier cannot start with numeric literal");
    }

    // terminate string
    buffer8[pos8] = '\0';

    double dval = 0;
    if (state == Number) {
#if defined(Q_WS_WIN) || defined(Q_OS_SOLARIS)
        // ### This may cause autotest failure, but that's just plain weird...
        dval = strtod(buffer8, 0L);
#else
        dval = strtold(buffer8, 0L);
#endif
    } else if (state == Hex) { // scan hex numbers
        quint64 i;
#if defined(_MSC_VER) && _MSC_VER >= 1400
        sscanf_s(buffer8, "%Lx", &i);
#else
        sscanf(buffer8, "%Lx", &i);
#endif
        dval = i;
        state = Number;
    } else if (state == Octal) {   // scan octal number
        quint64 ui;
#if defined(_MSC_VER) && _MSC_VER >= 1400
        sscanf_s(buffer8, "%Lo", &ui);
#else
        sscanf(buffer8, "%Lo", &ui);
#endif
        dval = ui;
        state = Number;
    }

    restrKeyword = false;
    delimited = false;

    switch (state) {
    case Eof:
        return 0;
    case Other:
        if(token == '}' || token == ';')
            delimited = true;
        return token;
    case Identifier:
        if ((token = findReservedWord(buffer16, pos16)) < 0) {
            /* TODO: close leak on parse error. same holds true for String */
            if (driver)
                qsyylval.ustr = driver->intern(buffer16, pos16);
            else
                qsyylval.ustr = 0;
            return QScriptGrammar::T_IDENTIFIER;
        }
        if (token == QScriptGrammar::T_CONTINUE || token == QScriptGrammar::T_BREAK ||
             token == QScriptGrammar::T_RETURN || token == QScriptGrammar::T_THROW)
            restrKeyword = true;
        return token;
    case String:
        if (driver)
            qsyylval.ustr = driver->intern(buffer16, pos16);
        else
            qsyylval.ustr = 0;
        return QScriptGrammar::T_STRING_LITERAL;
    case Number:
        qsyylval.dval = dval;
        return QScriptGrammar::T_NUMERIC_LITERAL;
    case Bad:
        return -1;
    default:
        assert(!"unhandled numeration value in switch");
        return -1;
    }
}

bool QScript::Lexer::isWhiteSpace() const
{
    return (current == ' ' || current == '\t' ||
             current == 0x0b || current == 0x0c);
}

bool QScript::Lexer::isLineTerminator() const
{
    return (current == '\n' || current == '\r');
}

bool QScript::Lexer::isIdentLetter(ushort c)
{
    /* TODO: allow other legitimate unicode chars */
    return (c >= 'a' && c <= 'z' ||
             c >= 'A' && c <= 'Z' ||
             c == '$' || c == '_');
}

bool QScript::Lexer::isDecimalDigit(ushort c)
{
    return (c >= '0' && c <= '9');
}

bool QScript::Lexer::isHexDigit(ushort c) const
{
    return (c >= '0' && c <= '9' ||
             c >= 'a' && c <= 'f' ||
             c >= 'A' && c <= 'F');
}

bool QScript::Lexer::isOctalDigit(ushort c) const
{
    return (c >= '0' && c <= '7');
}

int QScript::Lexer::matchPunctuator(ushort c1, ushort c2,
                            ushort c3, ushort c4)
{
    if (c1 == '>' && c2 == '>' && c3 == '>' && c4 == '=') {
        shift(4);
        return QScriptGrammar::T_GT_GT_GT_EQ;
    } else if (c1 == '=' && c2 == '=' && c3 == '=') {
        shift(3);
        return QScriptGrammar::T_EQ_EQ_EQ;
    } else if (c1 == '!' && c2 == '=' && c3 == '=') {
        shift(3);
        return QScriptGrammar::T_NOT_EQ_EQ;
    } else if (c1 == '>' && c2 == '>' && c3 == '>') {
        shift(3);
        return QScriptGrammar::T_GT_GT_GT;
    } else if (c1 == '<' && c2 == '<' && c3 == '=') {
        shift(3);
        return QScriptGrammar::T_LT_LT_EQ;
    } else if (c1 == '>' && c2 == '>' && c3 == '=') {
        shift(3);
        return QScriptGrammar::T_GT_GT_EQ;
    } else if (c1 == '<' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_LE;
    } else if (c1 == '>' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_GE;
    } else if (c1 == '!' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_NOT_EQ;
    } else if (c1 == '+' && c2 == '+') {
        shift(2);
        return QScriptGrammar::T_PLUS_PLUS;
    } else if (c1 == '-' && c2 == '-') {
        shift(2);
        return QScriptGrammar::T_MINUS_MINUS;
    } else if (c1 == '=' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_EQ_EQ;
    } else if (c1 == '+' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_PLUS_EQ;
    } else if (c1 == '-' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_MINUS_EQ;
    } else if (c1 == '*' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_STAR_EQ;
    } else if (c1 == '/' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_DIVIDE_EQ;
    } else if (c1 == '&' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_AND_EQ;
    } else if (c1 == '^' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_XOR_EQ;
    } else if (c1 == '%' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_REMAINDER_EQ;
    } else if (c1 == '|' && c2 == '=') {
        shift(2);
        return QScriptGrammar::T_OR_EQ;
    } else if (c1 == '<' && c2 == '<') {
        shift(2);
        return QScriptGrammar::T_LT_LT;
    } else if (c1 == '>' && c2 == '>') {
        shift(2);
        return QScriptGrammar::T_GT_GT;
    } else if (c1 == '&' && c2 == '&') {
        shift(2);
        return QScriptGrammar::T_AND_AND;
    } else if (c1 == '|' && c2 == '|') {
        shift(2);
        return QScriptGrammar::T_OR_OR;
    }

    switch(c1) {
        case '=': shift(1); return QScriptGrammar::T_EQ;
        case '>': shift(1); return QScriptGrammar::T_GT;
        case '<': shift(1); return QScriptGrammar::T_LT;
        case ',': shift(1); return QScriptGrammar::T_COMMA;
        case '!': shift(1); return QScriptGrammar::T_NOT;
        case '~': shift(1); return QScriptGrammar::T_TILDE;
        case '?': shift(1); return QScriptGrammar::T_QUESTION;
        case ':': shift(1); return QScriptGrammar::T_COLON;
        case '.': shift(1); return QScriptGrammar::T_DOT;
        case '+': shift(1); return QScriptGrammar::T_PLUS;
        case '-': shift(1); return QScriptGrammar::T_MINUS;
        case '*': shift(1); return QScriptGrammar::T_STAR;
        case '/': shift(1); return QScriptGrammar::T_DIVIDE;
        case '&': shift(1); return QScriptGrammar::T_AND;
        case '|': shift(1); return QScriptGrammar::T_OR;
        case '^': shift(1); return QScriptGrammar::T_XOR;
        case '%': shift(1); return QScriptGrammar::T_REMAINDER;
        case '(': shift(1); return QScriptGrammar::T_LPAREN;
        case ')': shift(1); return QScriptGrammar::T_RPAREN;
        case '{': shift(1); return QScriptGrammar::T_LBRACE;
        case '}': shift(1); return QScriptGrammar::T_RBRACE;
        case '[': shift(1); return QScriptGrammar::T_LBRACKET;
        case ']': shift(1); return QScriptGrammar::T_RBRACKET;
        case ';': shift(1); return QScriptGrammar::T_SEMICOLON;

        default: return -1;
    }
}

ushort QScript::Lexer::singleEscape(ushort c) const
{
    switch(c) {
    case 'b':
        return 0x08;
    case 't':
        return 0x09;
    case 'n':
        return 0x0A;
    case 'v':
        return 0x0B;
    case 'f':
        return 0x0C;
    case 'r':
        return 0x0D;
    case '"':
        return 0x22;
    case '\'':
        return 0x27;
    case '\\':
        return 0x5C;
    default:
        return c;
    }
}

ushort QScript::Lexer::convertOctal(ushort c1, ushort c2,
                            ushort c3) const
{
    return ((c1 - '0') * 64 + (c2 - '0') * 8 + c3 - '0');
}

unsigned char QScript::Lexer::convertHex(ushort c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    else if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    else
        return (c - 'A' + 10);
}

unsigned char QScript::Lexer::convertHex(ushort c1, ushort c2)
{
    return ((convertHex(c1) << 4) + convertHex(c2));
}

QChar QScript::Lexer::convertUnicode(ushort c1, ushort c2,
                             ushort c3, ushort c4)
{
    return QChar((convertHex(c3) << 4) + convertHex(c4),
                  (convertHex(c1) << 4) + convertHex(c2));
}

void QScript::Lexer::record8(ushort c)
{
    assert(c <= 0xff);

    // enlarge buffer if full
    if (pos8 >= size8 - 1) {
        char *tmp = new char[2 * size8];
        memcpy(tmp, buffer8, size8 * sizeof(char));
        delete [] buffer8;
        buffer8 = tmp;
        size8 *= 2;
    }

    buffer8[pos8++] = (char) c;
}

void QScript::Lexer::record16(QChar c)
{
    // enlarge buffer if full
    if (pos16 >= size16 - 1) {
        QChar *tmp = new QChar[2 * size16];
        memcpy(tmp, buffer16, size16 * sizeof(QChar));
        delete [] buffer16;
        buffer16 = tmp;
        size16 *= 2;
    }

    buffer16[pos16++] = c;
}

bool QScript::Lexer::scanRegExp()
{
    pos16 = 0;
    bool lastWasEscape = false;

    while (1) {
        if (isLineTerminator() || current == 0)
            return false;
        else if (current != '/' || lastWasEscape == true)
            {
                record16(current);
                lastWasEscape = !lastWasEscape && (current == '\\');
            }
        else {
            if (driver)
                pattern = driver->intern(buffer16, pos16);
            else
                pattern = 0;
            pos16 = 0;
            shift(1);
            break;
        }
        shift(1);
    }

    while (isIdentLetter(current)) {
        record16(current);
        shift(1);
    }

    if (driver)
        flags = driver->intern(buffer16, pos16);
    else
        flags = 0;

    return true;
}




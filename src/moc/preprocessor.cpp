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

#include "preprocessor.h"
#include "utils.h"
#include "ppkeywords.cpp"
#include <qstringlist.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>

QList<QByteArray> Preprocessor::includes;
Macros Preprocessor::macros;
bool Preprocessor::onlyPreprocess = false;
QByteArray Preprocessor::protocol;


static inline bool hasNext(const Symbols &symbols, int i)
{ return (i < symbols.size()); }

static inline const Symbol &next(const Symbols &symbols, int &i)
{ return symbols.at(i++); }

static void skipUntilEndif(const Symbols &symbols, int &i)
{
    while(i < symbols.size() - 1 && symbols.at(i).pp_token != PP_ENDIF){
        switch (symbols.at(i).pp_token) {
        case PP_IF:
        case PP_IFDEF:
        case PP_IFNDEF:
            ++i;
            skipUntilEndif(symbols, i);
            break;
        default:
            ;
        }
        ++i;
    }
}
static bool skipBranch(const Symbols &symbols, int &i)
{
    while(i < symbols.size() - 1
          && (symbols.at(i).pp_token != PP_ENDIF
               && symbols.at(i).pp_token != PP_ELIF
               && symbols.at(i).pp_token != PP_ELSE)
       ){
        switch (symbols.at(i).pp_token) {
        case PP_IF:
        case PP_IFDEF:
        case PP_IFNDEF:
            ++i;
            skipUntilEndif(symbols, i);
            break;
        default:
            ;
        }
        ++i;
    }
    return (i < symbols.size() - 1);
}

static QByteArray cleaned(const QByteArray &input)
{
    QByteArray output;
    output.reserve(input.size());
    const char *data = input;

    int newlines = 0;
    while (*data) {
        while (*data && is_space(*data))
            ++data;
        bool takeLine = (*data == '#');
        if (*data == '%' && *(data+1) == ':') {
            takeLine = true;
            ++data;
        }
        if (takeLine) {
            output += '#';
            do ++data; while (*data && is_space(*data));
        }
        while (*data) {
            if (*data == '\\' && *(data+1) == '\n') {
                ++newlines;
                data += 2;
                continue;
            }
            output += *data;
            if (*data == '\n') {
                while (newlines) {
                    output += '\n';
                    --newlines;
                }
                ++data;
                break;
            }
            ++data;
        }
    }
    return output;
}

static QByteArray cleanedAndFiltered(const QByteArray &input)
{
    QByteArray output;
    output.reserve(input.size());
    const char *data = input;

    while (*data) {
        while (*data && is_space(*data))
            ++data;
        bool takeLine = (*data == '#');
        if (*data == '%' && *(data+1) == ':') {
            takeLine = true;
            ++data;
        }
        if (takeLine){
            output += '#';
            do ++data; while (*data && is_space(*data));
            while (*data && *data != '\n') {
                if (*data == '\\' && *(data+1) == '\n') {
                    data += 2;
                    continue;
                }
                output += *data;
                ++data;
            }
            output += '\n';
        } else { // skip line
            while (*data) {
                if (*data == '\n') {
                    ++data;
                    break;
                }
                if (*data == '\\' && *(data+1) == '\n')
                    ++data;
                ++data;
            }
        }
    }
    return output;
}


enum TokenizeMode { TokenizeFile, TokenizeLine };
static Symbols tokenize(const QByteArray &input, int lineNum = 1, TokenizeMode mode = TokenizeFile)
{
    Symbols symbols;
    const char *begin = input;
    const char *data = begin;
    while (*data) {
        const char *lexem = data;
        int state = 0;
        PP_Token token = PP_NOTOKEN;
        for (;;) {
            if (static_cast<signed char>(*data) < 0) {
                ++data;
                continue;
            }
            int nextindex = pp_keywords[state].next;
            int next = 0;
            if (*data == pp_keywords[state].defchar)
                next = pp_keywords[state].defnext;
            else if (!state || nextindex)
                    next = pp_keyword_trans[nextindex][(int)*data];
            if (!next)
                break;
            state = next;
            token = pp_keywords[state].token;
            ++data;
        }
        // suboptimal, is_ident_char  should use a table
        if (pp_keywords[state].ident && is_ident_char(*data))
            token = pp_keywords[state].ident;

        if (mode == TokenizeFile && token >= PP_FIRST_STATEMENT && token <= PP_LAST_STATEMENT) {
            while (*data && *data != '\n')
                ++data;
        } else switch (token) {
        case NOTOKEN:
            ++data;
            break;
        case PP_IFDEF:
            symbols += Symbol(lineNum, PP_IF, QByteArray());
            symbols += Symbol(lineNum, PP_DEFINED, QByteArray());
            continue;
        case PP_IFNDEF:
            symbols += Symbol(lineNum, PP_IF, QByteArray());
            symbols += Symbol(lineNum, PP_NOT, QByteArray());
            symbols += Symbol(lineNum, PP_DEFINED, QByteArray());
            continue;
        case PP_QUOTE:
            while (*data && (*data != '\"'
                             || (*(data-1)=='\\'
                                  && *(data-2)!='\\')))
                ++data;
            if (*data)
                ++data;
            token = PP_STRING_LITERAL;
            break;
        case PP_SINGLEQUOTE:
            while (*data && (*data != '\''
                             || (*(data-1)=='\\'
                                  && *(data-2)!='\\')))
                ++data;
            if (*data)
                ++data;
            token = PP_CHARACTER_LITERAL;
            break;
        case PP_DIGIT:
            while (is_digit_char(*data))
                ++data;
            if (!*data || *data != '.') {
                token = PP_INTEGER_LITERAL;
                if (data - lexem == 1 &&
                     (*data == 'x' || *data == 'X')
                     && *lexem == '0') {
                    ++data;
                    while (is_hex_char(*data))
                        ++data;
                }
                break;
            }
            token = PP_FLOATING_LITERAL;
            ++data;
            // fall through
        case PP_FLOATING_LITERAL:
            while (is_digit_char(*data))
                ++data;
            if (*data == '+' || *data == '-')
                ++data;
            if (*data == 'e' || *data == 'E') {
                ++data;
                while (is_digit_char(*data))
                    ++data;
            }
            if (*data == 'f' || *data == 'F'
                || *data == 'l' || *data == 'L')
                ++data;
            break;
        case PP_CHARACTER:
            while (is_ident_char(*data))
                ++data;
            token = PP_IDENTIFIER;
            break;
        case PP_C_COMMENT:
            while (*data && (*(data-1) != '/' || *(data-2) != '*')) {
                if (*data == '\n')
                    ++lineNum;
                ++data;
            }
            token = PP_WHITESPACE; // one comment, one whitespace
            // fall through;
        case PP_WHITESPACE:
            while (*data && (*data == ' ' || *data == '\t'))
                ++data;
            break;
        case PP_CPP_COMMENT:
            while (*data && *data != '\n')
                ++data;
            continue; // ignore safly, the newline is a seperator
        case PP_NEWLINE:
            if (mode == TokenizeLine)
                goto exit;
            ++lineNum;
            break;
        default:
            break;
        }
        symbols += Symbol(lineNum, token, input, lexem-begin, data-lexem);
    }
exit:
    symbols += Symbol(); // eof symbol
    return symbols;
}

static Symbols tokenize(const Symbol &symbol)
{ return tokenize(symbol.lexem(), symbol.lineNum, TokenizeLine); }

static Symbols substitute(const Macros &macros, const Symbols& symbols, int &i,
                          bool discardWhitespace = false, QList<QByteArray> safeset = QList<QByteArray>())
{
    QByteArray lexem = symbols.at(i-1).lexem();

    if (!macros.contains(lexem) || safeset.contains(lexem))
        return Symbols(1, symbols.at(i-1));

    QByteArray macro = macros.value(lexem);

                       // ### cannot do parameters yet, TODO
    if (macro.size() && macro.at(0) == '(')
        return Symbols(1, symbols.at(i-1));

    safeset += lexem;
    Symbols syms = tokenize(macro, symbols.at(i-1).lineNum, TokenizeLine);

    Symbols result;
    int j = 0;
    bool skip = false;
    while (hasNext(syms, j+1)) {
        const Symbol &sym = next(syms, j);
        if (discardWhitespace && sym.pp_token == PP_WHITESPACE)
            continue;
        if (sym.pp_token == PP_IDENTIFIER && !skip)
            result += substitute(macros, syms, j, discardWhitespace, safeset);
        else
            result += sym;
        skip = (sym.pp_token == PP_DEFINED || skip && sym.pp_token == PP_LPAREN);
    }
    return result;
}

struct PP_Expression
{
    PP_Expression():i(0){}
    Macros macros;
    Symbols symbols;
    int i;

    int value() { i = 0; return unary_expression_lookup() ?  conditional_expression() : 0; }

    inline bool hasNext() const { return (i <= symbols.size()); }
    inline PP_Token next() { return symbols.at(i++).pp_token; }
    bool test(PP_Token);
    inline void prev() {--i;}
    PP_Token lookup(int k = 1);
    inline PP_Token token() { return symbols.at(i-1).pp_token;}
    inline QByteArray lexem() { return symbols.at(i-1).lexem();}

    int conditional_expression();
    int logical_OR_expression();
    int logical_AND_expression();
    int inclusive_OR_expression();
    int exclusive_OR_expression();
    int AND_expression();
    int equality_expression();
    int relational_expression();
    int shift_expression();
    int additive_expression();
    int multiplicative_expression();
    int unary_expression();
    bool unary_expression_lookup();
    int primary_expression();
    bool primary_expression_lookup();
};

inline bool PP_Expression::test(PP_Token token)
{
    if (i < symbols.size() && symbols.at(i).pp_token == token) {
        ++i;
        return true;
    }
    return false;
}

inline PP_Token PP_Expression::lookup(int k)
{
    const int l = i - 1 + k;
    return l < symbols.size() ? symbols.at(l).pp_token : PP_NOTOKEN;
}

int PP_Expression::conditional_expression()
{
    int value = logical_OR_expression();
    if (test(PP_QUESTION)) {
        int alt1 = conditional_expression();
        int alt2 = test(PP_COLON) ? conditional_expression() : 0;
        return value ? alt1 : alt2;
    }
    return value;
}

int PP_Expression::logical_OR_expression()
{
    int value = logical_AND_expression();
    if (test(PP_OROR))
        return value || logical_OR_expression();
    return value;
}

int PP_Expression::logical_AND_expression()
{
    int value = inclusive_OR_expression();
    if (test(PP_ANDAND))
        return value && logical_AND_expression();
    return value;
}

int PP_Expression::inclusive_OR_expression()
{
    int value = exclusive_OR_expression();
    if (test(PP_OR))
        return value | inclusive_OR_expression();
    return value;
}

int PP_Expression::exclusive_OR_expression()
{
    int value = AND_expression();
    if (test(PP_HAT))
        return value ^ exclusive_OR_expression();
    return value;
}

int PP_Expression::AND_expression()
{
    int value = equality_expression();
    if (test(PP_AND))
        return value & AND_expression();
    return value;
}

int PP_Expression::equality_expression()
{
    int value = relational_expression();
    switch (next()) {
    case PP_EQEQ:
        return value == equality_expression();
    case PP_NE:
        return value != equality_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::relational_expression()
{
    int value = shift_expression();
    switch (next()) {
    case PP_LANGLE:
        return value < relational_expression();
    case PP_RANGLE:
        return value > relational_expression();
    case PP_LE:
        return value <= relational_expression();
    case PP_GE:
        return value >= relational_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::shift_expression()
{
    int value = additive_expression();
    switch (next()) {
    case PP_LTLT:
        return value << shift_expression();
    case PP_GTGT:
        return value >> shift_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::additive_expression()
{
    int value = multiplicative_expression();
    switch (next()) {
    case PP_PLUS:
        return value + additive_expression();
    case PP_MINUS:
        return value - additive_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::multiplicative_expression()
{
    int value = unary_expression();
    switch (next()) {
    case PP_STAR:
        return value * multiplicative_expression();
    case PP_PERCENT:
        return value % multiplicative_expression();
    case PP_SLASH:
    {
        int div = multiplicative_expression();
        return div ? value / div : 0;
    }
    default:
        prev();
        return value;
    };
}

int PP_Expression::unary_expression()
{
    switch (next()) {
    case PP_PLUS:
        return unary_expression();
    case PP_MINUS:
        return -unary_expression();
    case PP_NOT:
        return !unary_expression();
    case PP_TILDE:
        return ~unary_expression();
    case PP_DEFINED:
    {
        QByteArray identifier;
        if (test(PP_IDENTIFIER)) {
            identifier = lexem();
        } else if (test(PP_LPAREN)) {
            if (test(PP_IDENTIFIER))
                identifier = lexem();
            test(PP_RPAREN);
        }
        return macros.contains(identifier);
    }
    default:
        prev();
        return primary_expression();
    }
}

bool PP_Expression::unary_expression_lookup()
{
    PP_Token t = lookup();
    return (primary_expression_lookup()
            || t == PP_PLUS
            || t == PP_MINUS
            || t == PP_NOT
            || t == PP_TILDE
            || t == PP_DEFINED);
}

int PP_Expression::primary_expression()
{
    int value;
    if (test(PP_LPAREN)) {
        value = conditional_expression();
        test(PP_RPAREN);
    } else {
        next();
        value = QString(lexem()).toInt(0, 0);
    }
    return value;
}

bool PP_Expression::primary_expression_lookup()
{
    PP_Token t = lookup();
    return (t == PP_IDENTIFIER
            || t == PP_INTEGER_LITERAL
            || t == PP_FLOATING_LITERAL
            || t == PP_LPAREN);
}

static int evaluateCondition(const Macros &macros, const Symbol &symbol)
{
    PP_Expression expression;
    expression.macros = macros;
    Symbols symbols = tokenize(symbol);
    int i = 1;
    bool skip = false;
    while (hasNext(symbols, i)) {
        const Symbol &sym = next(symbols, i);
        if (sym.pp_token == PP_WHITESPACE)
            continue;
        if (sym.pp_token == PP_IDENTIFIER && macros.contains(sym.lexem()) && !skip)
            expression.symbols += substitute(macros, symbols, i, true);
        else
            expression.symbols += sym;
        skip = (sym.pp_token == PP_DEFINED || skip && sym.pp_token == PP_LPAREN);
    }

    return expression.value();
}


static Symbols preprocess(const QByteArray &filename, const Symbols &symbols, Macros &macros)
{
    static int depth = 0;
    Symbols preprocessed;
    preprocessed.reserve(symbols.size());
    int i = 0;
    while (hasNext(symbols,i)) {
        Symbol sym = next(symbols, i);
        switch (sym.pp_token) {
        case PP_INCLUDE:
        {
            QByteArray include = sym.lexem();
            const char *data = include.constData() + 8;
            while (*data && is_whitespace(*data))
                ++data;
            const char *name = data++;
            if (*name == '\"')
                while (*data && *data != '\"')
                    ++data;
            else if (*name == '<')
                while (*data && *data != '>')
                    ++data;
            else
                continue;
            bool local = (*name == '\"');
            include = QByteArray(name + 1, data - name - 1);

            // #### stringery
            QFileInfo fi;
            if (local)
                fi.setFile(QFileInfo(QString::fromLocal8Bit(filename)).dir(), QString::fromLocal8Bit(include));
            for (int j = 0; j < Preprocessor::includes.size() && !fi.exists(); ++j)
                fi.setFile(QString::fromLocal8Bit(Preprocessor::includes.at(j)), QString::fromLocal8Bit(include));
            if (!fi.exists())
                continue;
            include = fi.filePath().toLocal8Bit();
            QFile file(QString::fromLocal8Bit(include));
            if (!file.open(QFile::ReadOnly))
                continue;

            if (Preprocessor::onlyPreprocess) {
                Preprocessor::protocol += "#";
                Preprocessor::protocol += QByteArray(depth * 2, ' ');
                Preprocessor::protocol += "include \"";
                Preprocessor::protocol += include;
                Preprocessor::protocol += "\"\n";
            }
//            qDebug("... include %s", include.constData());
            QByteArray input = file.readAll();
            file.close();
            if (input.isEmpty())
                continue;
            // phase 1: get rid of backslash-newlines and everything
            // not preprocessor related
            QByteArray phase1 = cleanedAndFiltered(input);
            // phase 2: tokenize for the preprocessor
            Symbols symbols = tokenize(phase1);
            // phase 3: preprocess conditions and substitute macros
            ++depth;
            symbols = preprocess(include, symbols, macros);
            --depth;
            continue;
        }
        case PP_DEFINE:
        {
            QByteArray macro = sym.lexem();
            const char *data = macro.constData() + 7;
            while (*data && is_whitespace(*data))
                ++data;
            if (!is_ident_start(*data))
                continue;
            const char *ident = data++;
            while (*data && is_ident_char(*data))
                ++data;
            QByteArray name(ident, data - ident);
            macros[name] = data;
            if (Preprocessor::onlyPreprocess) {
                Preprocessor::protocol += "#";
                Preprocessor::protocol += QByteArray(depth * 2, ' ');
                Preprocessor::protocol += "define ";
                Preprocessor::protocol += name;
                Preprocessor::protocol += "\n";
            }
            continue;
        }
        case PP_IDENTIFIER:
            // we _could_ easily substitute macros by the following
            // four lines, but we choose not to.
            /*
            if (macros.contains(sym.lexem())) {
                preprocessed += substitute(macros, symbols, i);
                continue;
            }
            */
            break;
        case PP_HASH:
            continue; // skip unknown preprocessor statement
        case PP_IFDEF:
        case PP_IFNDEF:
        case PP_IF:
            while (!evaluateCondition(macros, sym)) {
                if (!skipBranch(symbols, i))
                    break;
                sym = next(symbols, i);
                if (sym.pp_token != PP_ELIF)
                    break;
            }
            continue;
        case PP_ELIF:
        case PP_ELSE:
            skipUntilEndif(symbols, i);
            // fall through
        case PP_ENDIF:
            continue;
        default:
            break;
        }
        preprocessed += sym;
    }
    return preprocessed;
}

QByteArray Preprocessor::preprocessed(const QByteArray &filename, FILE *file)
{
    QByteArray output;
    QFile qfile;
    qfile.open(QFile::ReadOnly|QFile::Text, file);
    QByteArray input = qfile.readAll();
    if (input.isEmpty())
        return output;

    // phase 1: get rid of backslash-newlines
    QByteArray phase1 = cleaned(input);

    // phase 2: tokenize for the preprocessor
    Symbols symbols = tokenize(phase1);

#if 0
    for (int j = 0; j < symbols.size(); ++j)
        qDebug("line %d: %s(%d)",
               symbols[j].lineNum,
               symbols[j].lexem().constData(),
               symbols[j].token);
#endif

    // phase 3: preprocess conditions and substitute macros
    symbols = preprocess(filename, symbols, macros);

    // final phase: compose string for the C++ scanner
    int lineNum = 1;
    PP_Token last = PP_NOTOKEN;
    PP_Token secondlast = last;
    int i = 0;
    while (hasNext(symbols, i)) {
        Symbol sym = next(symbols, i);
        switch (sym.pp_token) {
        case PP_NEWLINE:
        case PP_WHITESPACE:
            if (last != PP_WHITESPACE) {
                secondlast = last;
                last = PP_WHITESPACE;
                output += ' ';
            }
            continue;
        case PP_STRING_LITERAL:
            if (last == PP_STRING_LITERAL)
                output.chop(1);
            else if (secondlast == PP_STRING_LITERAL && last == PP_WHITESPACE)
                output.chop(2);
            else
                break;
            output += sym.lexem().mid(1);
            secondlast = last;
            last = PP_STRING_LITERAL;
            continue;
        default:
            break;
        }
        secondlast = last;
        last = sym.pp_token;
        while (sym.lineNum > lineNum) {
            output += '\n';
            ++lineNum;
        }
        output += sym.lexem();
    }

    return output;
}



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
#include "rpplexer.h"
#include <iostream>
#include <locale>
#include <QChar>
#include <QList>


using namespace std;
using namespace TokenEngine;

namespace Rpp {
RppLexer::RppLexer()
{
    setupScanTable();
}

void RppLexer::setupScanTable()
{
    memset(s_attr_table, 0, 256);

    for (int i=0; i<128; ++i) {
        switch (i) {
        case ':':
        case '*':
        case '%':
        case '^':
        case '=':
        case '!':
        case '&':
        case '|':
        case '+':
        case '<':
        case '>':
        case '-':
        case '.':
            s_scan_table[i] = &RppLexer::scanOperator;
            break;

        case '\n':
            s_scan_table[i] = &RppLexer::scanNewline;
            break;

        case '\'':
            s_scan_table[i] = &RppLexer::scanCharLiteral;
            break;

        case '"':
            s_scan_table[i] = &RppLexer::scanStringLiteral;
            break;
        case '#':
            s_scan_table[i] = &RppLexer::scanPreprocessor;
            break;

        case '/':
            s_scan_table[i] = &RppLexer::scanComment;
            break;

        default:
            if (isspace(i)) {
                s_scan_table[i] = &RppLexer::scanWhiteSpaces;
                s_attr_table[i] |= A_Whitespace;
            } else if (isalpha(i) || i == '_') {
                s_scan_table[i] = &RppLexer::scanKeyword;
                s_attr_table[i] |= A_Alpha;
            } else if (isdigit(i)) {
                s_scan_table[i] = &RppLexer::scanNumberLiteral;
                s_attr_table[i] |= A_Digit;
            } else
                s_scan_table[i] = &RppLexer::scanChar;
        }
    }

    s_scan_table[128] = &RppLexer::scanUnicodeChar;
}

QList<Type> RppLexer::lex(TokenSequence *tokenSequence)
{
    QList<Type> tokenTypes;
    const int numTokens = tokenSequence->count();
    for(int t=0; t<numTokens; ++t) {
        tokenTypes.append(indentify(tokenSequence->text(t)));
    }
    return tokenTypes;
}

Type RppLexer::indentify(QByteArray tokenText)
{
    Q_ASSERT(tokenText.count() > 0 );
    const unsigned char ch = tokenText[0];
    m_buffer = tokenText;
    m_ptr = 0;
    int kind = 0;
    (this->*s_scan_table[ch < 128 ? ch : 128])(&kind);
    return (Type)kind;
}

void RppLexer::scanChar(int *kind)
{
    *kind = m_buffer[m_ptr++];
}

void RppLexer::scanWhiteSpaces(int *kind)
{
    *kind = Token_whitespaces;

    while (unsigned char ch = m_buffer[m_ptr]) {
        if (s_attr_table[ch] & A_Whitespace)
            ++m_ptr;
        else
            break;
    }
}

void RppLexer::scanNewline(int *kind)
{
    *kind = m_buffer[0];
}

void RppLexer::scanUnicodeChar(int *kind)
{
    *kind = m_buffer[0];
}

void RppLexer::scanCharLiteral(int *kind)
{
    *kind = Token_char_literal;
}

void RppLexer::scanStringLiteral(int *kind)
{
    *kind = Token_string_literal;
}

void RppLexer::scanIdentifier(int *kind)
{
    *kind = Token_identifier;
}

void RppLexer::scanNumberLiteral(int *kind)
{
    *kind = Token_number_literal;
}

void RppLexer::scanPreprocessor(int *kind)
{
    *kind = Token_preproc;
}

void RppLexer::scanComment(int *kind)
{
    switch(m_buffer[m_ptr + 1]) {
    case '/':
        *kind = Token_line_comment;
        break;
    case '*':
        *kind = Token_multiline_comment;
        break;
    default:
        scanOperator(kind);
    }
}

void RppLexer::scanOperator(int *kind)
{
    switch (m_buffer[m_ptr]) {
    case ':':
        if (m_buffer[m_ptr+1] == ':') {
            *kind = Token_scope;
            return;
        }
        break;

    case '*':
    case '/':
    case '%':
    case '^':
        if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_assign;
            return;
        }
        break;

    case '=':
       if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_eq;
            return;
        }
        break;
    case '!':
       if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_not_eq;
            return;
        }
        break;

    case '&':
        if (m_buffer[m_ptr+1] == '&') {
            *kind = Token_and;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_assign;
            return;
        }
        break;

    case '|':
        if (m_buffer[m_ptr+1] == '|' ) {
            *kind = Token_or;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_assign;
            return;
        }
        break;

    case '+':
        if (m_buffer[m_ptr+1] == '+' ) {
            *kind = Token_incr;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_assign;
            return;
        }
        break;

    case '<':
        if (m_buffer[m_ptr+1] == '<') {
            if (m_buffer[m_ptr+2] == '=') {
                *kind = Token_assign;
                return;
            }
            *kind = Token_left_shift;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_leq;
            return;
        }
        break;

    case '>':
        if (m_buffer[m_ptr+1] == '>') {
            if (m_buffer[m_ptr+2] == '=') {
                *kind = Token_assign;
                return;
            }
            *kind = Token_right_shift;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_geq;
            return;
        }
        break;

    case '-':
        if (m_buffer[m_ptr+1] == '>') {
            if (m_buffer[m_ptr+2] == '*') {
                *kind = Token_ptrmem;
                return;
            }
            *kind = Token_arrow;
            return;
        } else if (m_buffer[m_ptr+1] == '-') {
            *kind = Token_decr;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            *kind = Token_assign;
            return;
        }
        break;

    case '.':
        if (m_buffer[m_ptr+1] == '.' && m_buffer[m_ptr+2] == '.') {
            *kind = Token_ellipsis;
            return;
        } else if (m_buffer[m_ptr+1] == '*') {
            *kind = Token_ptrmem;
            return;
        }
        break;

    }

    *kind = m_buffer[m_ptr++];
}

void RppLexer::scanKeyword(int *kind)
{
    if(m_buffer == "if")
        *kind = Token_directive_if;
    else if(m_buffer == "elif")
        *kind = Token_directive_elif;
    else if (m_buffer == "else")
        *kind = Token_directive_else;
    else if (m_buffer == "line")
        *kind = Token_directive_line;
    else if (m_buffer == "else")
        *kind = Token_directive_else;
    else if (m_buffer == "line")
        *kind = Token_directive_line;
    else if (m_buffer == "endif")
        *kind = Token_directive_endif;
    else if (m_buffer == "ifdef")
        *kind = Token_directive_ifdef;
    else if (m_buffer == "error")
        *kind = Token_directive_error;
    else if (m_buffer == "undef")
        *kind = Token_directive_undef;
    else if (m_buffer == "pragma")
        *kind = Token_directive_pragma;
    else if (m_buffer == "ifndef")
        *kind = Token_directive_ifndef;
    else if (m_buffer == "define")
        *kind = Token_directive_define;
    else if (m_buffer == "include")
        *kind = Token_directive_include;
    else if (m_buffer == "defined")
        *kind = Token_defined;
    else
        *kind = Token_identifier;
}

} //namespace Rpp

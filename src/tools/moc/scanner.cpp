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

#include "scanner.h"
#include "moc.h"
#include "utils.h"

#include "keywords.cpp"


Symbols Scanner::scan(const QByteArray &input)
{
    Symbols symbols;
    const char *begin = input;
    const char *data = begin;
    int lineNum = 1;
    while (*data) {
        const char *lexem = data;
        int state = 0;
        Token token = NOTOKEN;
        for (;;) {
            if (static_cast<signed char>(*data) < 0) {
                ++data;
                continue;
            }
            int nextindex = keywords[state].next;
            int next = 0;
            if (*data == keywords[state].defchar)
                next = keywords[state].defnext;
            else if (!state || nextindex)
                    next = keyword_trans[nextindex][(int)*data];
            if (!next)
                break;
            state = next;
            token = keywords[state].token;
            ++data;
        }

        // suboptimal, is_ident_char  should use a table
        if (keywords[state].ident && is_ident_char(*data))
            token = keywords[state].ident;

        if (token == NOTOKEN) {
            // an error really
            ++data;
            continue;
        }

        if (token > SPECIAL_TREATMENT_MARK) {
            switch (token) {
            case QUOTE:
                while (*data && (*data != '\"'
                                 || (*(data-1)=='\\'
                                     && *(data-2)!='\\')))
                    ++data;
                if (*data)
                    ++data;
                token = STRING_LITERAL;
                break;
            case SINGLEQUOTE:
                while (*data && (*data != '\''
                                 || (*(data-1)=='\\'
                                     && *(data-2)!='\\')))
                    ++data;
                if (*data)
                    ++data;
                token = CHARACTER_LITERAL;
                break;
            case DIGIT:
                while (is_digit_char(*data))
                    ++data;
                if (!*data || *data != '.') {
                    token = INTEGER_LITERAL;
                    if (data - lexem == 1 &&
                         (*data == 'x' || *data == 'X')
                         && *lexem == '0') {
                        ++data;
                        while (is_hex_char(*data))
                            ++data;
                    }
                    break;
                }
                token = FLOATING_LITERAL;
                ++data;
                // fall through
            case FLOATING_LITERAL:
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
            case HASH:
                // not sure about this one, can only happen through
                // macro substitution. Ignore until next line?
                break;
            case MOC_INCLUDE_BEGIN:
                lineNum = 0;
                break;
            case MOC_INCLUDE_END:
                symbols += Symbol(lineNum, token, input, lexem-begin, data-lexem);
                ++data;
                lineNum = 0;
                while (is_digit_char(*data)) {
                    lineNum = lineNum * 10 + *data - '0';
                    ++data;
                }
                continue;
            case NEWLINE:
                ++lineNum;
                continue;
            case CHARACTER:
                while (is_ident_char(*data))
                    ++data;
                token = IDENTIFIER;
                break;
            case WHITESPACE:
            default:
                continue; //ignore
            }
        }

        symbols += Symbol(lineNum, token, input, lexem-begin, data-lexem);
        //qDebug("%d (%d): %s", lineNum, token, QByteArray(lexem, data-lexem).constData());
    }

    symbols += Symbol(); // eof symbol
    return symbols;
};





/* This file is part of KDevelop
    Copyright (C) 2002,2003,2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "lexer.h"
#include "tokens.h"
#include <qdatetime.h>
#include <cctype>

#include <qhash.h>

static QHash<QByteArray, bool> preprocessed;

bool Lexer::s_initialized = false;
scan_fun_ptr Lexer::s_scan_table[];
int Lexer::s_attr_table[];

scan_fun_ptr Lexer::s_scan_keyword_table[] = {
    &Lexer::scanKeyword0, &Lexer::scanKeyword0, &Lexer::scanKeyword2, &Lexer::scanKeyword3,
    &Lexer::scanKeyword4, &Lexer::scanKeyword5, &Lexer::scanKeyword6, &Lexer::scanKeyword7,
    &Lexer::scanKeyword8, &Lexer::scanKeyword9, &Lexer::scanKeyword10, &Lexer::scanKeyword11,
    &Lexer::scanKeyword12, &Lexer::scanKeyword0, &Lexer::scanKeyword14, &Lexer::scanKeyword0,
    &Lexer::scanKeyword16
};

Lexer::Lexer()
{
    if (!s_initialized)
        setupScanTable();
}

Lexer::~Lexer()
{
}

void Lexer::scanKeyword0(int *kind)
{
    *kind = Token_identifier;
}

void Lexer::scanKeyword2(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'i':
        if (m_buffer[m_ptr+1] == 'f')
        {
            *kind = Token_if;
            return;
        }
        break;

        case 'd':
        if (m_buffer[m_ptr+1] == 'o')
        {
            *kind = Token_do;
            return;
        }
        break;

        case 'o':
        if (m_buffer[m_ptr+1] == 'r')
        {
            *kind = Token_or;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword3(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'a':
        if (m_buffer[m_ptr+1] == 'n' &&
            m_buffer[m_ptr+2] == 'd')
        {
            *kind = Token_and;
            return;
        }
        if (m_buffer[m_ptr+1] == 's' &&
            m_buffer[m_ptr+2] == 'm')
        {
            *kind = Token_asm;
            return;
        }
        break;

        case 'f':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'r')
        {
            *kind = Token_for;
            return;
        }
        break;

        case 'i':
        if (m_buffer[m_ptr+1] == 'n' &&
            m_buffer[m_ptr+2] == 't')
        {
            *kind = Token_int;
            return;
        }
        break;

        case 'n':
        if (m_buffer[m_ptr+1] == 'e' &&
            m_buffer[m_ptr+2] == 'w')
        {
            *kind = Token_new;
            return;
        }
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 't')
        {
            *kind = Token_not;
            return;
        }
        break;

        case 't':
        if (m_buffer[m_ptr+1] == 'r' &&
            m_buffer[m_ptr+2] == 'y')
        {
            *kind = Token_try;
            return;
        }
        break;

        case 'x':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'r')
        {
            *kind = Token_xor;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword4(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'a':
        if (m_buffer[m_ptr+1] == 'u' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'o')
        {
            *kind = Token_auto;
            return;
        }
        break;

        case 'c':
        if (m_buffer[m_ptr+1] == 'a' &&
            m_buffer[m_ptr+2] == 's' &&
            m_buffer[m_ptr+3] == 'e')
        {
            *kind = Token_case;
            return;
        }
        if (m_buffer[m_ptr+1] == 'h' &&
            m_buffer[m_ptr+2] == 'a' &&
            m_buffer[m_ptr+3] == 'r')
        {
            *kind = Token_char;
            return;
        }
        break;

        case 'b':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'o' &&
            m_buffer[m_ptr+3] == 'l')
        {
            *kind = Token_bool;
            return;
        }
        break;

        case 'e':
        if (m_buffer[m_ptr+1] == 'l' &&
            m_buffer[m_ptr+2] == 's' &&
            m_buffer[m_ptr+3] == 'e')
        {
            *kind = Token_else;
            return;
        }
        if (m_buffer[m_ptr+1] == 'm' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 't')
        {
            *kind = Token_emit;
            return;
        }
        if (m_buffer[m_ptr+1] == 'n' &&
            m_buffer[m_ptr+2] == 'u' &&
            m_buffer[m_ptr+3] == 'm')
        {
            *kind = Token_enum;
            return;
        }
        break;

        case 'g':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'o')
        {
            *kind = Token_goto;
            return;
        }
        break;

        case 'l':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'n' &&
            m_buffer[m_ptr+3] == 'g')
        {
            *kind = Token_long;
            return;
        }
        break;

        case 't':
        if (m_buffer[m_ptr+1] == 'h' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 's')
        {
            *kind = Token_this;
            return;
        }
        break;

        case 'v':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 'd')
        {
            *kind = Token_void;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword5(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'c':
        if (m_buffer[m_ptr+1] == 'a' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'c' &&
            m_buffer[m_ptr+4] == 'h')
        {
            *kind = Token_catch;
            return;
        }
        if (m_buffer[m_ptr+1] == 'l' &&
            m_buffer[m_ptr+2] == 'a' &&
            m_buffer[m_ptr+3] == 's' &&
            m_buffer[m_ptr+4] == 's')
        {
            *kind = Token_class;
            return;
        }
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'm' &&
            m_buffer[m_ptr+3] == 'p' &&
            m_buffer[m_ptr+4] == 'l')
        {
            *kind = Token_compl;
            return;
        }
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'n' &&
            m_buffer[m_ptr+3] == 's' &&
            m_buffer[m_ptr+4] == 't')
        {
            *kind = Token_const;
            return;
        }
        break;

        case 'b':
        if (m_buffer[m_ptr+1] == 'i' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'o' &&
            m_buffer[m_ptr+4] == 'r')
        {
            *kind = Token_bitor;
            return;
        }
        if (m_buffer[m_ptr+1] == 'r' &&
            m_buffer[m_ptr+2] == 'e' &&
            m_buffer[m_ptr+3] == 'a' &&
            m_buffer[m_ptr+4] == 'k')
        {
            *kind = Token_break;
            return;
        }
        break;

        case 'f':
        if (m_buffer[m_ptr+1] == 'l' &&
            m_buffer[m_ptr+2] == 'o' &&
            m_buffer[m_ptr+3] == 'a' &&
            m_buffer[m_ptr+4] == 't')
        {
            *kind = Token_float;
            return;
        }
        break;

        case 'o':
        if (m_buffer[m_ptr+1] == 'r' &&
            m_buffer[m_ptr+2] == '_' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 'q')
        {
            *kind = Token_or_eq;
            return;
        }
        break;

        case 's':
        if (m_buffer[m_ptr+1] == 'h' &&
            m_buffer[m_ptr+2] == 'o' &&
            m_buffer[m_ptr+3] == 'r' &&
            m_buffer[m_ptr+4] == 't')
        {
            *kind = Token_short;
            return;
        }
        if (m_buffer[m_ptr+1] == 'l' &&
            m_buffer[m_ptr+2] == 'o' &&
            m_buffer[m_ptr+3] == 't' &&
            m_buffer[m_ptr+4] == 's')
        {
            *kind = Token_slots;
            return;
        }
        break;

        case 'u':
        if (m_buffer[m_ptr+1] == 'n' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 'o' &&
            m_buffer[m_ptr+4] == 'n')
        {
            *kind = Token_union;
            return;
        }
        if (m_buffer[m_ptr+1] == 's' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 'n' &&
            m_buffer[m_ptr+4] == 'g')
        {
            *kind = Token_using;
            return;
        }
        break;

        case 't':
        if (m_buffer[m_ptr+1] == 'h' &&
            m_buffer[m_ptr+2] == 'r' &&
            m_buffer[m_ptr+3] == 'o' &&
            m_buffer[m_ptr+4] == 'w')
        {
            *kind = Token_throw;
            return;
        }
        break;

        case 'w':
        if (m_buffer[m_ptr+1] == 'h' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 'l' &&
            m_buffer[m_ptr+4] == 'e')
        {
            *kind = Token_while;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword6(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'a':
        if (m_buffer[m_ptr+1] == 'n' &&
            m_buffer[m_ptr+2] == 'd' &&
            m_buffer[m_ptr+3] == '_' &&
            m_buffer[m_ptr+4] == 'e' &&
            m_buffer[m_ptr+5] == 'q')
        {
            *kind = Token_and_eq;
            return;
        }
        break;

        case 'b':
        if (m_buffer[m_ptr+1] == 'i' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'a' &&
            m_buffer[m_ptr+4] == 'n' &&
            m_buffer[m_ptr+5] == 'd')
        {
            *kind = Token_bitand;
            return;
        }
        break;

        case 'e':
        if (m_buffer[m_ptr+1] == 'x' &&
            m_buffer[m_ptr+2] == 'p' &&
            m_buffer[m_ptr+3] == 'o' &&
            m_buffer[m_ptr+4] == 'r' &&
            m_buffer[m_ptr+5] == 't')
        {
            *kind = Token_export;
            return;
        }
        if (m_buffer[m_ptr+1] == 'x' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 'r' &&
            m_buffer[m_ptr+5] == 'n')
        {
            *kind = Token_extern;
            return;
        }
        break;

        case 'd':
        if (m_buffer[m_ptr+1] == 'e' &&
            m_buffer[m_ptr+2] == 'l' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 't' &&
            m_buffer[m_ptr+5] == 'e')
        {
            *kind = Token_delete;
            return;
        }
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'u' &&
            m_buffer[m_ptr+3] == 'b' &&
            m_buffer[m_ptr+4] == 'l' &&
            m_buffer[m_ptr+5] == 'e')
        {
            *kind = Token_double;
            return;
        }
        break;

        case 'f':
        if (m_buffer[m_ptr+1] == 'r' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 'n' &&
            m_buffer[m_ptr+5] == 'd')
        {
            *kind = Token_friend;
            return;
        }
        break;

        case 'i':
        if (m_buffer[m_ptr+1] == 'n' &&
            m_buffer[m_ptr+2] == 'l' &&
            m_buffer[m_ptr+3] == 'i' &&
            m_buffer[m_ptr+4] == 'n' &&
            m_buffer[m_ptr+5] == 'e')
        {
            *kind = Token_inline;
            return;
        }
        break;

        case 'K':
        if (m_buffer[m_ptr+1] == '_' &&
            m_buffer[m_ptr+2] == 'D' &&
            m_buffer[m_ptr+3] == 'C' &&
            m_buffer[m_ptr+4] == 'O' &&
            m_buffer[m_ptr+5] == 'P')
        {
            *kind = Token_K_DCOP;
            return;
        }
        break;

        case 'n':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == '_' &&
            m_buffer[m_ptr+4] == 'e' &&
            m_buffer[m_ptr+5] == 'q')
        {
            *kind = Token_not_eq;
            return;
        }
        break;

        case 'p':
        if (m_buffer[m_ptr+1] == 'u' &&
            m_buffer[m_ptr+2] == 'b' &&
            m_buffer[m_ptr+3] == 'l' &&
            m_buffer[m_ptr+4] == 'i' &&
            m_buffer[m_ptr+5] == 'c')
        {
            *kind = Token_public;
            return;
        }
        break;

        case 's':
        if (m_buffer[m_ptr+1] == 'i' &&
            m_buffer[m_ptr+2] == 'g' &&
            m_buffer[m_ptr+3] == 'n' &&
            m_buffer[m_ptr+4] == 'e' &&
            m_buffer[m_ptr+5] == 'd')
        {
            *kind = Token_signed;
            return;
        }
        if (m_buffer[m_ptr+1] == 'i' &&
            m_buffer[m_ptr+2] == 'z' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 'o' &&
            m_buffer[m_ptr+5] == 'f')
        {
            *kind = Token_sizeof;
            return;
        }
        if (m_buffer[m_ptr+1] == 't' &&
            m_buffer[m_ptr+2] == 'a' &&
            m_buffer[m_ptr+3] == 't' &&
            m_buffer[m_ptr+4] == 'i' &&
            m_buffer[m_ptr+5] == 'c')
        {
            *kind = Token_static;
            return;
        }
        if (m_buffer[m_ptr+1] == 't' &&
            m_buffer[m_ptr+2] == 'r' &&
            m_buffer[m_ptr+3] == 'u' &&
            m_buffer[m_ptr+4] == 'c' &&
            m_buffer[m_ptr+5] == 't')
        {
            *kind = Token_struct;
            return;
        }
        if (m_buffer[m_ptr+1] == 'w' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 't' &&
            m_buffer[m_ptr+4] == 'c' &&
            m_buffer[m_ptr+5] == 'h')
        {
            *kind = Token_switch;
            return;
        }
        break;

        case 'r':
        if (m_buffer[m_ptr+1] == 'e' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'u' &&
            m_buffer[m_ptr+4] == 'r' &&
            m_buffer[m_ptr+5] == 'n')
        {
            *kind = Token_return;
            return;
        }
        break;

        case 't':
        if (m_buffer[m_ptr+1] == 'y' &&
            m_buffer[m_ptr+2] == 'p' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 'i' &&
            m_buffer[m_ptr+5] == 'd')
        {
            *kind = Token_typeid;
            return;
        }
        break;

        case 'x':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'r' &&
            m_buffer[m_ptr+3] == '_' &&
            m_buffer[m_ptr+4] == 'e' &&
            m_buffer[m_ptr+5] == 'q')
        {
            *kind = Token_xor_eq;
            return;
        }
        break;

        case 'k':
        if (m_buffer[m_ptr+1] == '_' &&
            m_buffer[m_ptr+2] == 'd' &&
            m_buffer[m_ptr+3] == 'c' &&
            m_buffer[m_ptr+4] == 'o' &&
            m_buffer[m_ptr+5] == 'p')
        {
            *kind = Token_k_dcop;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword7(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'd':
        if (m_buffer[m_ptr+1] == 'e' &&
            m_buffer[m_ptr+2] == 'f' &&
            m_buffer[m_ptr+3] == 'a' &&
            m_buffer[m_ptr+4] == 'u' &&
            m_buffer[m_ptr+5] == 'l' &&
            m_buffer[m_ptr+6] == 't')
        {
            *kind = Token_default;
            return;
        }
        break;

        case 'm':
        if (m_buffer[m_ptr+1] == 'u' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'a' &&
            m_buffer[m_ptr+4] == 'b' &&
            m_buffer[m_ptr+5] == 'l' &&
            m_buffer[m_ptr+6] == 'e')
        {
            *kind = Token_mutable;
            return;
        }
        break;

        case 'p':
        if (m_buffer[m_ptr+1] == 'r' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 'v' &&
            m_buffer[m_ptr+4] == 'a' &&
            m_buffer[m_ptr+5] == 't' &&
            m_buffer[m_ptr+6] == 'e')
        {
            *kind = Token_private;
            return;
        }
        break;
#if 0
        case 's':
        if (m_buffer[m_ptr+1] == 'i' &&
            m_buffer[m_ptr+2] == 'g' &&
            m_buffer[m_ptr+3] == 'n' &&
            m_buffer[m_ptr+4] == 'a' &&
            m_buffer[m_ptr+5] == 'l' &&
            m_buffer[m_ptr+6] == 's')
        {
            *kind = Token_signals;
            return;
        }
        break;
#endif
        case 't':
        if (m_buffer[m_ptr+1] == 'y' &&
            m_buffer[m_ptr+2] == 'p' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 'd' &&
            m_buffer[m_ptr+5] == 'e' &&
            m_buffer[m_ptr+6] == 'f')
        {
            *kind = Token_typedef;
            return;
        }
        break;

        case 'v':
        if (m_buffer[m_ptr+1] == 'i' &&
            m_buffer[m_ptr+2] == 'r' &&
            m_buffer[m_ptr+3] == 't' &&
            m_buffer[m_ptr+4] == 'u' &&
            m_buffer[m_ptr+5] == 'a' &&
            m_buffer[m_ptr+6] == 'l')
        {
            *kind = Token_virtual;
            return;
        }
        break;
    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword8(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case '_':
        if (m_buffer[m_ptr+1] == '_' &&
            m_buffer[m_ptr+2] == 't' &&
            m_buffer[m_ptr+3] == 'y' &&
            m_buffer[m_ptr+4] == 'p' &&
            m_buffer[m_ptr+5] == 'e' &&
            m_buffer[m_ptr+6] == 'o' &&
            m_buffer[m_ptr+7] == 'f')
        {
            *kind = Token___typeof;
            return;
        }
        break;

        case 'c':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'n' &&
            m_buffer[m_ptr+3] == 't' &&
            m_buffer[m_ptr+4] == 'i' &&
            m_buffer[m_ptr+5] == 'n' &&
            m_buffer[m_ptr+6] == 'u' &&
            m_buffer[m_ptr+7] == 'e')
        {
            *kind = Token_continue;
            return;
        }
        break;

        case 'e':
        if (m_buffer[m_ptr+1] == 'x' &&
            m_buffer[m_ptr+2] == 'p' &&
            m_buffer[m_ptr+3] == 'l' &&
            m_buffer[m_ptr+4] == 'i' &&
            m_buffer[m_ptr+5] == 'c' &&
            m_buffer[m_ptr+6] == 'i' &&
            m_buffer[m_ptr+7] == 't')
        {
            *kind = Token_explicit;
            return;
        }
        break;

        case 'o':
        if (m_buffer[m_ptr+1] == 'p' &&
            m_buffer[m_ptr+2] == 'e' &&
            m_buffer[m_ptr+3] == 'r' &&
            m_buffer[m_ptr+4] == 'a' &&
            m_buffer[m_ptr+5] == 't' &&
            m_buffer[m_ptr+6] == 'o' &&
            m_buffer[m_ptr+7] == 'r')
        {
            *kind = Token_operator;
            return;
        }
        break;

        case 'Q':
        if (m_buffer[m_ptr+1] == '_' &&
            m_buffer[m_ptr+2] == 'O' &&
            m_buffer[m_ptr+3] == 'B' &&
            m_buffer[m_ptr+4] == 'J' &&
            m_buffer[m_ptr+5] == 'E' &&
            m_buffer[m_ptr+6] == 'C' &&
            m_buffer[m_ptr+7] == 'T')
        {
            *kind = Token_Q_OBJECT;
            return;
        }
        break;

        case 'r':
        if (m_buffer[m_ptr+1] == 'e' &&
            m_buffer[m_ptr+2] == 'g' &&
            m_buffer[m_ptr+3] == 'i' &&
            m_buffer[m_ptr+4] == 's' &&
            m_buffer[m_ptr+5] == 't' &&
            m_buffer[m_ptr+6] == 'e' &&
            m_buffer[m_ptr+7] == 'r')
        {
            *kind = Token_register;
            return;
        }
        break;

        case 'u':
        if (m_buffer[m_ptr+1] == 'n' &&
            m_buffer[m_ptr+2] == 's' &&
            m_buffer[m_ptr+3] == 'i' &&
            m_buffer[m_ptr+4] == 'g' &&
            m_buffer[m_ptr+5] == 'n' &&
            m_buffer[m_ptr+6] == 'e' &&
            m_buffer[m_ptr+7] == 'd')
        {
            *kind = Token_unsigned;
            return;
        }
        break;

        case 't':
        if (m_buffer[m_ptr+1] == 'e' &&
            m_buffer[m_ptr+2] == 'm' &&
            m_buffer[m_ptr+3] == 'p' &&
            m_buffer[m_ptr+4] == 'l' &&
            m_buffer[m_ptr+5] == 'a' &&
            m_buffer[m_ptr+6] == 't' &&
            m_buffer[m_ptr+7] == 'e')
        {
            *kind = Token_template;
            return;
        }
        if (m_buffer[m_ptr+1] == 'y' &&
            m_buffer[m_ptr+2] == 'p' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 'n' &&
            m_buffer[m_ptr+5] == 'a' &&
            m_buffer[m_ptr+6] == 'm' &&
            m_buffer[m_ptr+7] == 'e')
        {
            *kind = Token_typename;
            return;
        }
        break;

        case 'v':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'l' &&
            m_buffer[m_ptr+3] == 'a' &&
            m_buffer[m_ptr+4] == 't' &&
            m_buffer[m_ptr+5] == 'i' &&
            m_buffer[m_ptr+6] == 'l' &&
            m_buffer[m_ptr+7] == 'e')
        {
            *kind = Token_volatile;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword9(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'p':
        if (m_buffer[m_ptr+1] == 'r' &&
            m_buffer[m_ptr+2] == 'o' &&
            m_buffer[m_ptr+3] == 't' &&
            m_buffer[m_ptr+4] == 'e' &&
            m_buffer[m_ptr+5] == 'c' &&
            m_buffer[m_ptr+6] == 't' &&
            m_buffer[m_ptr+7] == 'e' &&
            m_buffer[m_ptr+8] == 'd')
        {
            *kind = Token_protected;
            return;
        }
        break;

        case 'n':
        if (m_buffer[m_ptr+1] == 'a' &&
            m_buffer[m_ptr+2] == 'm' &&
            m_buffer[m_ptr+3] == 'e' &&
            m_buffer[m_ptr+4] == 's' &&
            m_buffer[m_ptr+5] == 'p' &&
            m_buffer[m_ptr+6] == 'a' &&
            m_buffer[m_ptr+7] == 'c' &&
            m_buffer[m_ptr+8] == 'e')
        {
            *kind = Token_namespace;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword10(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'c':
        if (m_buffer[m_ptr+1] == 'o' &&
            m_buffer[m_ptr+2] == 'n' &&
            m_buffer[m_ptr+3] == 's' &&
            m_buffer[m_ptr+4] == 't' &&
            m_buffer[m_ptr+5] == '_' &&
            m_buffer[m_ptr+6] == 'c' &&
            m_buffer[m_ptr+7] == 'a' &&
            m_buffer[m_ptr+8] == 's' &&
            m_buffer[m_ptr+9] == 't')
        {
            *kind = Token_const_cast;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword11(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 's':
        if (m_buffer[m_ptr+1] == 't' &&
            m_buffer[m_ptr+2] == 'a' &&
            m_buffer[m_ptr+3] == 't' &&
            m_buffer[m_ptr+4] == 'i' &&
            m_buffer[m_ptr+5] == 'c' &&
            m_buffer[m_ptr+6] == '_' &&
            m_buffer[m_ptr+7] == 'c' &&
            m_buffer[m_ptr+8] == 'a' &&
            m_buffer[m_ptr+9] == 's' &&
            m_buffer[m_ptr+10] == 't')
        {
            *kind = Token_static_cast;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword12(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'd':
        if (m_buffer[m_ptr+1] == 'y' &&
            m_buffer[m_ptr+2] == 'n' &&
            m_buffer[m_ptr+3] == 'a' &&
            m_buffer[m_ptr+4] == 'm' &&
            m_buffer[m_ptr+5] == 'i' &&
            m_buffer[m_ptr+6] == 'c' &&
            m_buffer[m_ptr+7] == '_' &&
            m_buffer[m_ptr+8] == 'c' &&
            m_buffer[m_ptr+9] == 'a' &&
            m_buffer[m_ptr+10] == 's' &&
            m_buffer[m_ptr+11] == 't')
        {
            *kind = Token_dynamic_cast;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword14(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'k':
        if (m_buffer[m_ptr+1] == '_' &&
            m_buffer[m_ptr+2] == 'd' &&
            m_buffer[m_ptr+3] == 'c' &&
            m_buffer[m_ptr+4] == 'o' &&
            m_buffer[m_ptr+5] == 'p' &&
            m_buffer[m_ptr+6] == '_' &&
            m_buffer[m_ptr+7] == 's' &&
            m_buffer[m_ptr+8] == 'i' &&
            m_buffer[m_ptr+9] == 'g' &&
            m_buffer[m_ptr+10] == 'n' &&
            m_buffer[m_ptr+11] == 'a' &&
            m_buffer[m_ptr+12] == 'l' &&
            m_buffer[m_ptr+13] == 's')
        {
            *kind = Token_k_dcop_signals;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

void Lexer::scanKeyword16(int *kind)
{
    switch (m_buffer[m_ptr]) {
        case 'r':
        if (m_buffer[m_ptr+1] == 'e' &&
            m_buffer[m_ptr+2] == 'i' &&
            m_buffer[m_ptr+3] == 'n' &&
            m_buffer[m_ptr+4] == 't' &&
            m_buffer[m_ptr+5] == 'e' &&
            m_buffer[m_ptr+6] == 'r' &&
            m_buffer[m_ptr+7] == 'p' &&
            m_buffer[m_ptr+8] == 'r' &&
            m_buffer[m_ptr+9] == 'e' &&
            m_buffer[m_ptr+10] == 't' &&
            m_buffer[m_ptr+11] == '_' &&
            m_buffer[m_ptr+12] == 'c' &&
            m_buffer[m_ptr+13] == 'a' &&
            m_buffer[m_ptr+14] == 's' &&
            m_buffer[m_ptr+15] == 't')
        {
            *kind = Token_reinterpret_cast;
            return;
        }
        break;

    }
    *kind = Token_identifier;
}

enum
{
    A_Alpha = 0x01,
    A_Digit = 0x02,
    A_Alphanum = A_Alpha | A_Digit,
    A_Whitespace = 0x04
};

void Lexer::setupScanTable()
{
    s_initialized = true;

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
            s_scan_table[i] = &Lexer::scanOperator;
            break;

        case '\n':
            s_scan_table[i] = &Lexer::scanNewline;
            break;

        case '#':
            s_scan_table[i] = &Lexer::scanPreprocessor;
            break;

        case '/':
            s_scan_table[i] = &Lexer::scanComment;
            break;

        case '\'':
            s_scan_table[i] = &Lexer::scanCharLiteral;
            break;

        case '"':
            s_scan_table[i] = &Lexer::scanStringLiteral;
            break;

        default:
            if (isspace(i)) {
                s_scan_table[i] = &Lexer::scanWhiteSpaces;
                s_attr_table[i] |= A_Whitespace;
            } else if (isalpha(i) || i == '_') {
                s_scan_table[i] = &Lexer::scanIdentifier;
                s_attr_table[i] |= A_Alpha;
            } else if (isdigit(i)) {
                s_scan_table[i] = &Lexer::scanNumberLiteral;
                s_attr_table[i] |= A_Digit;
            } else
                s_scan_table[i] = &Lexer::scanChar;
        }
    }

    s_scan_table[128] = &Lexer::scanUnicodeChar;
}

TokenStream *Lexer::tokenize(const FileSymbol *fileSymbol, bool)
{
    QTime t;
    t.start();

    m_fileName = fileSymbol->fileName;
    m_currentFileName = m_fileName;

    m_tokens.clear();

    m_contents = fileSymbol->contents;
    m_buffer = fileSymbol->contents;
    m_ptr = 0;

    m_lines.clear();
    m_lines.append(0);

    m_pplines.clear();

    m_skipping = false;

    // tokenize
    for (;;) {
        Token tk;
        nextToken(tk);
        tk.node = 0;

        if (!m_skipping)
            m_tokens.append(tk);

        if (tk.kind == 0)
            break;
    }

    TokenStream *tokenStream = new TokenStream;
    tokenStream->m_contents = m_contents;

    tokenStream->m_lines = m_lines;
    tokenStream->m_pplines = m_pplines;
    tokenStream->m_tokens = m_tokens;

    tokenStream->m_cursor = -1;

    //printf("tokenized in %d produced %d tokens\n", t.elapsed(), m_tokens.size());
    return tokenStream;
}

void Lexer::nextToken(Token &tok)
{
    int start = m_ptr;
    unsigned char ch = (unsigned char)m_buffer[m_ptr];

    int kind = 0;
    (this->*s_scan_table[ch < 128 ? ch : 128])(&kind);
    tok.kind = kind;

    tok.position = start;
    tok.length = m_ptr - start;

    switch (kind) {
        case Token_comment:
        case Token_whitespaces:
        case Token_preproc:
        case '\n':
            tok.hidden = 1;
            break;

        default:
            tok.hidden = 0;
            break;
    }
}

void Lexer::scanChar(int *kind)
{
    *kind = m_buffer[m_ptr++];
}

void Lexer::scanWhiteSpaces(int *kind)
{
    *kind = Token_whitespaces;

    while (unsigned char ch = m_buffer[m_ptr]) {
        if (s_attr_table[ch] & A_Whitespace)
            ++m_ptr;
        else
            break;
    }
}

void Lexer::scanNewline(int *kind)
{
    m_lines.append(m_ptr);

    *kind = m_buffer[m_ptr++];
}

void Lexer::scanUnicodeChar(int *kind)
{
    *kind = m_buffer[m_ptr++];
}

void Lexer::scanCharLiteral(int *kind)
{
    ++m_ptr;
    for (;;) {
        unsigned char ch = m_buffer[m_ptr];
        switch (ch) {
        case '\0':
        case '\n':
            // ### error
            *kind = Token_char_literal;
            return;
        case '\\':
            if (m_buffer[m_ptr+1] == '\'' || m_buffer[m_ptr+1] == '\\')
                m_ptr += 2;
            else
                ++m_ptr;
            break;
        case '\'':
            ++m_ptr;
            *kind = Token_char_literal;
            return;
        default:
            ++m_ptr;
            break;
        }
    }

    // ### error
    *kind = Token_char_literal;
}

void Lexer::scanStringLiteral(int *kind)
{
    ++m_ptr;
    while (m_buffer[m_ptr]) {
        switch (m_buffer[m_ptr]) {
        case '\n':
            // ### error
            *kind = Token_string_literal;
            return;
        case '\\':
            if (m_buffer[m_ptr+1] == '"' || m_buffer[m_ptr+1] == '\\')
                m_ptr += 2;
            else
                ++m_ptr;
            break;
        case '"':
            ++m_ptr;
            *kind = Token_string_literal;
            return;
        default:
            ++m_ptr;
            break;
        }
    }

    // ### error
    *kind = Token_string_literal;
}

void Lexer::scanIdentifier(int *kind)
{
    int start = m_ptr;
    unsigned char ch;
    for (;;) {
        ch = m_buffer[m_ptr];
        if (s_attr_table[ch] & A_Alphanum)
            ++m_ptr;
        else
            break;
    }

    int len = m_ptr - start;
    m_ptr -= len;
    (this->*s_scan_keyword_table[len < 17 ? len : 0])(kind);
    m_ptr += len;
}

void Lexer::scanNumberLiteral(int *kind)
{
    unsigned char ch;
    for (;;) {
        ch = m_buffer[m_ptr];
        if (s_attr_table[ch] & A_Alphanum || ch == '.')
            ++m_ptr;
        else
            break;
    }

    // ### finish to implement me!!
    *kind = Token_number_literal;
}

void Lexer::scanComment(int *kind)
{
    if (!(m_buffer[m_ptr+1] == '/' || m_buffer[m_ptr+1] == '*'))
        return scanOperator(kind);

    ++m_ptr; // skip '/'

    bool multiLineComment = m_buffer[m_ptr++] == '*';

    while (m_buffer[m_ptr]) {
        switch (m_buffer[m_ptr]) {
        case '\n':
            if (!multiLineComment) {
                *kind = Token_comment;
                return;
            }

            (void) scanNewline(kind);
            break;

        case '*':
            if (multiLineComment && m_buffer[m_ptr+1] == '/') {
                m_ptr += 2;
                *kind = Token_comment;
                return;
            }
            ++m_ptr;
            break;

        default:
            ++m_ptr;
        }
    }

    // ### error
    *kind = Token_comment;
}


void Lexer::scanPreprocessor(int *kind)
{
    int s = qMax(0, m_ptr - 1);

    while (m_buffer[s]) {
        if (m_buffer[s] == ' ' || m_buffer[s] == '\t')
            --s;
        else
            break;
    }

    if (!(m_ptr <= 0 || m_buffer[s] == '\n')) {
        // ### error
        *kind = m_buffer[m_ptr++];
        return;
    }

    ++m_ptr; // skip #
    *kind = Token_preproc;

    int pos = m_ptr;

    PPLine pp;
    if (scanPPLine(&pp.line, &pp.fileName)) {
        pp.position = pos;
        m_pplines.append(pp);
    }

    bool ignoreNewline = false;
    while (unsigned char ch = m_buffer[m_ptr]) {
        switch (ch) {
        case '\n':
            if (!ignoreNewline)
                return;

            ignoreNewline = false;
            (void) scanNewline(kind);
            break;

        case ' ':
        case '\t':
            ++m_ptr;
            break;

        case '\\':
            ++m_ptr;
            ignoreNewline = true;
            break;

        default:
            ++m_ptr;
            ignoreNewline = false;
            break;
        }
    }
}


void Lexer::scanOperator(int *kind)
{
    switch (m_buffer[m_ptr]) {
    case ':':
        if (m_buffer[m_ptr+1] == ':') {
            m_ptr += 2;
            *kind = Token_scope;
            return;
        }
        break;

    case '*':
    case '/':
    case '%':
    case '^':
        if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_assign;
            return;
        }
        break;

    case '=':
    case '!':
        if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_eq;
            return;
        }
        break;

    case '&':
        if (m_buffer[m_ptr+1] == '&') {
            m_ptr += 2;
            *kind = Token_and;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_assign;
            return;
        }
        break;

    case '|':
        if (m_buffer[m_ptr+1] == '|' ) {
            m_ptr += 2;
            *kind = Token_or;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_assign;
            return;
        }
        break;

    case '+':
        if (m_buffer[m_ptr+1] == '+' ) {
            m_ptr += 2;
            *kind = Token_incr;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_assign;
            return;
        }
        break;

    case '<':
        if (m_buffer[m_ptr+1] == '<') {
            if (m_buffer[m_ptr+2] == '=') {
                m_ptr += 3;
                *kind = Token_assign;
                return;
            }
            m_ptr += 2;
            *kind = Token_shift;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_leq;
            return;
        }
        break;

    case '>':
        if (m_buffer[m_ptr+1] == '>') {
            if (m_buffer[m_ptr+2] == '=') {
                m_ptr += 3;
                *kind = Token_assign;
                return;
            }
            m_ptr += 2;
            *kind = Token_shift;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_geq;
            return;
        }
        break;

    case '-':
        if (m_buffer[m_ptr+1] == '>') {
            if (m_buffer[m_ptr+2] == '*') {
                m_ptr += 3;
                *kind = Token_ptrmem;
                return;
            }
            m_ptr += 2;
            *kind = Token_arrow;
            return;
        } else if (m_buffer[m_ptr+1] == '-') {
            m_ptr += 2;
            *kind = Token_decr;
            return;
        } else if (m_buffer[m_ptr+1] == '=') {
            m_ptr += 2;
            *kind = Token_assign;
            return;
        }
        break;

    case '.':
        if (m_buffer[m_ptr+1] == '.' && m_buffer[m_ptr+2] == '.') {
            m_ptr += 3;
            *kind = Token_ellipsis;
            return;
        } else if (m_buffer[m_ptr+1] == '*') {
            m_ptr += 2;
            *kind = Token_ptrmem;
            return;
        }
        break;

    }

    *kind = m_buffer[m_ptr++];
}

void TokenStream::positionAtAux(int position, int *line, int *column) const
{
    if (!(line && column && !m_lines.isEmpty()))
        return;

    int first = 0;
    int len = m_lines.size();
    int half;
    int middle;

    while (len > 0) {
        half = len >> 1;
        middle = first;

        middle += half;

        if (m_lines[middle] < position) {
            first = middle;
            ++first;
            len = len - half - 1;
        }
        else
            len = half;
    }

    *line = qMax(first - 1, 0);
    *column = position - m_lines.at(*line);

    Q_ASSERT( *column >= 0 );
}

void TokenStream::positionAt(int position, int *line, int *column, QByteArray *fileName) const
{
    positionAtAux(position, line, column);

    PPLine pp;
    for (int i=0; (i<m_pplines.size()) && (m_pplines.at(i).position < position); ++i)
        pp = m_pplines.at(i);

    int line2, col2;
    positionAtAux(pp.position, &line2, &col2);

    *line -= line2;
    *line += pp.line - 1;

    if (fileName)
        *fileName = pp.fileName;
}

bool Lexer::scanPPLine(int *line, QByteArray *fileName)
{
    Token tk;
    nextToken(tk);

    if (tk.kind == Token_whitespaces) {
        nextToken(tk);
        if (tk.kind == Token_number_literal) {
            *line = m_contents.mid(tk.position, tk.length).toInt();
            nextToken(tk);
            if (tk.kind == Token_whitespaces) {
                nextToken(tk);
                if (tk.kind == Token_string_literal) {
                    *fileName = m_contents.mid(tk.position+1, tk.length-2);
                    m_currentFileName = m_contents.mid(tk.position+1, tk.length-2);
                    // fprintf(stdout, "parsing %s...\n", m_currentFileName.constData());
                    //m_skipping = m_currentFileName != m_fileName;
                    return true;
                }
            }
        }
    }

    return false;
}

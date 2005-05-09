/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
** Copyright (C) 2001-2004 Roberto Raggi
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef CPPLEXER_H
#define CPPLEXER_H

#include <QVector>
#include "tokenengine.h"
#include "tokens.h"

class CppLexer
{
public:
    CppLexer();
    typedef void (CppLexer::*scan_fun_ptr)(int *kind);
    QVector<Type> lex(TokenEngine::TokenSectionSequence tokenContainer);
private:
    Type identify(const TokenEngine::TokenTempRef &tokenTempRef);
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
    void scanOperator(int *kind);

    void scanKeyword0(int *kind);
    void scanKeyword2(int *kind);
    void scanKeyword3(int *kind);
    void scanKeyword4(int *kind);
    void scanKeyword5(int *kind);
    void scanKeyword6(int *kind);
    void scanKeyword7(int *kind);
    void scanKeyword8(int *kind);
    void scanKeyword9(int *kind);
    void scanKeyword10(int *kind);
    void scanKeyword11(int *kind);
    void scanKeyword12(int *kind);
    void scanKeyword14(int *kind);
    void scanKeyword16(int *kind);

    CppLexer::scan_fun_ptr s_scan_table[128+1];
    int s_attr_table[256];
    CppLexer::scan_fun_ptr s_scan_keyword_table[17];

    enum
    {
        A_Alpha = 0x01,
        A_Digit = 0x02,
        A_Alphanum = A_Alpha | A_Digit,
        A_Whitespace = 0x04
    };

    const char *m_buffer;
    int m_ptr;
    int m_len;
};

#endif

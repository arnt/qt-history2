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
#include "rpp.h"

#include <iostream>
#include "rppexpressionbuilder.h"

using namespace std;
using namespace TokenEngine;

namespace Rpp
{

Preprocessor::Preprocessor()
{

}

Source *Preprocessor::parse(TokenEngine::TokenContainer tokenContainer,
                            QList<Type> tokenTypeList, TypedPool<Item> *memoryPool)
{
    m_memoryPool = memoryPool;
    Source *m_source = createNode<Source>(m_memoryPool); //node whith no parent
    m_tokenContainer = tokenContainer;
    m_tokenTypeList = tokenTypeList;
    lexerTokenIndex = 0;
    numTokens = m_tokenContainer.count();

    if(m_tokenContainer.count() != tokenTypeList.count()) {
        emit error("Error", "Internal error in preprocessor: Number of tokens is not equal to number of types in type list");
        return m_source;
    }

    if(tokenTypeList.isEmpty()) {
        emit error("Warning:", "Trying to parse empty source file");
        return m_source;
    }
    Q_ASSERT(m_source->toItemComposite());
    parseGroup(m_source);

    return m_source;
}

// group-part
// group group-part
bool Preprocessor::parseGroup(Item *group)
{
    Q_ASSERT(group->toItemComposite());
    bool gotGroup = false;
    while(lexerTokenIndex < numTokens) {
        if (!parseGroupPart(group))
            break;
        gotGroup = true;
    }
    return gotGroup;
}

//if-section        (# if / # ifdef / #ifndef )
//control-line      ( #include / etc )
//# non-directive   ( # text newline
//text-line         (text newline )
bool Preprocessor::parseGroupPart(Item *group)
{
    //cout << "parse group part" << endl;
    Q_ASSERT(group->toItemComposite());

    //look up first significant token
    Type token = lookAhead();
    if(token == Token_eof)
        return false;

    //look for '#'
    if(token != Token_preproc)
        return parseTextLine(group);

    //look up first significant token after the '#'
    token = lookAheadSkipHash();
    if(token == Token_eof)
        return false;

    // Check if we are at the end of a group. This is not an neccesarely an
    // error, it happens when we reach an #endif for example.
    if (token == Token_directive_elif || token == Token_directive_else ||
        token == Token_directive_endif)
        return false;

    // if-section?
    if(token == Token_directive_if || token == Token_directive_ifdef ||
        token == Token_directive_ifndef)
        return parseIfSection(group);

    // control-line?
    if (token == Token_directive_define)
        return parseDefineDirective(group);
    if (token ==  Token_directive_undef)
        return parseUndefDirective(group);
    if (token ==  Token_directive_include)
        return parseIncludeDirective(group);
    if (token == Token_directive_error)
        return parseErrorDirective(group);
    if (token ==  Token_directive_pragma)
        return parsePragmaDirective(group);

    return parseNonDirective(group);
}

// if-section -> if-group elif-groups[opt] else-group[opt] endif-line
bool Preprocessor::parseIfSection(Item *group)
{
   // cout << "parse if section" << endl ;
    Q_ASSERT(group->toItemComposite());
    IfSection *ifSection = createNode<IfSection>(m_memoryPool, group);
    group->toItemComposite()->add(ifSection);

    if (!parseIfGroup(ifSection))
        return false;

    Type type = lookAheadSkipHash();
    if(type == Token_directive_elif)
        if(!parseElifGroups(ifSection))
            return false;

    type = lookAheadSkipHash();
    if(type == Token_directive_else)
        if(!parseElseGroup(ifSection))
            return false;

    return parseEndifLine(ifSection);
}

bool Preprocessor::parseNonDirective(Item *group)
{
 //  cout << "parsenondirective" << endl;
    Q_ASSERT(group->toItemComposite());
    TokenSection tokenSection = readLine();
    if(tokenSection.count() == 0)
        return false;

    NonDirective *nonDirective = createNode<NonDirective>(m_memoryPool, group);
    group->toItemComposite()->add(nonDirective);
    nonDirective->setTokenSection(tokenSection);
    return true;
}


bool Preprocessor::parseTextLine(Item *group)
{
    //cout << "parsetextline" << endl;
    Q_ASSERT(group->toItemComposite());
    const TokenSection tokenSection = readLine();
   // cout << tokenSection.fullText().constData() << endl;

    if(tokenSection.count() == 0)
        return false;

    Text *text = createNode<Text>(m_memoryPool, group);
    group->toItemComposite()->add(text);
    text->setTokenSection(tokenSection);

    // create Token-derived nodes and atach to text
    for (int t=0; t<tokenSection.count(); ++t) {
        Token *node = 0;
        const int containerIndex = tokenSection.containerIndex(t);
        switch(m_tokenTypeList.at(containerIndex)) {
            case Token_identifier:
            case Token_defined:
            case Token_directive_if:
            case Token_directive_elif:
            case Token_directive_else:
            case Token_directive_undef:
            case Token_directive_endif:
            case Token_directive_ifdef:
            case Token_directive_ifndef:
            case Token_directive_define:
            case Token_directive_include:
                node = createNode<IdToken>(m_memoryPool, text);
            break;
            case Token_line_comment:
                node = createNode<LineComment>(m_memoryPool, text);
            break;
            case Token_multiline_comment:
                node = createNode<MultiLineComment>(m_memoryPool, text);
            break;
            case Token_whitespaces:
            case Token_char_literal:
            case Token_string_literal:
            default:
                node = createNode<NonIdToken>(m_memoryPool, text);
            break;
        }
        Q_ASSERT(node);
        node->setToken(containerIndex);
        text->addToken(node);
    }

    return true;
}

// if-group -> ifDirective
// if-group -> ifdefDirevtive
// if-group -> ifndefDirevtive
bool Preprocessor::parseIfGroup(IfSection *ifSection)
{
    //  cout << "parse if group" << endl;
    Q_ASSERT(ifSection->toItemComposite());
    bool result;
    const Type type = lookAheadSkipHash();
    if (type == Token_directive_ifdef) {
        IfdefDirective *d = createNode<IfdefDirective>(m_memoryPool, ifSection);
        result = parseIfdefLikeDirective(d);
        ifSection->setIfGroup(d);
    } else if (type == Token_directive_ifndef) {
        IfndefDirective *d = createNode<IfndefDirective>(m_memoryPool, ifSection);
        result = parseIfdefLikeDirective(d);
        ifSection->setIfGroup(d);
    } else  if (type == Token_directive_if) {
        IfDirective *d = createNode<IfDirective>(m_memoryPool, ifSection);
        result = parseIfLikeDirective(d);
        ifSection->setIfGroup(d);
    } else {
        result = false;
    }
    return result;
}

bool Preprocessor::parseElifGroups(IfSection *ifSection)
{
    //cout << "parse ElifGroups" << endl;
    bool gotElif = false;
    while(lookAheadSkipHash() == Token_directive_elif ) {
        if (!parseElifGroup(ifSection))
            break;
        gotElif = true;
    }
    return gotElif;
}

bool Preprocessor::parseElifGroup(IfSection *ifSection)
{
    //cout << "parse ElifGroup" << endl;
    ElifDirective *elifDirective = createNode<ElifDirective>(m_memoryPool, ifSection);
    ifSection->addElifGroup(elifDirective);
    return parseIfLikeDirective(elifDirective);
}

bool Preprocessor::parseElseGroup(IfSection *ifSection)
{
    //cout << "parse else group" << endl;
    TokenSection tokenSection = readLine();
    if(tokenSection.count() == 0)
        return false;

    ElseDirective *elseDirective = createNode<ElseDirective>(m_memoryPool, ifSection);
    ifSection->setElseGroup(elseDirective);
    elseDirective->setTokenSection(tokenSection);
    parseGroup(elseDirective);
    return true;
}

//# endif newline
bool Preprocessor::parseEndifLine(IfSection *ifSection)
{
    //cout << "parse endifline" << endl;
    TokenSection tokenSection = readLine();
    if(tokenSection.count() == 0)
        return false;

    EndifDirective *endifDirective = createNode<EndifDirective>(m_memoryPool, ifSection);
    ifSection->setEndifLine(endifDirective);
    endifDirective->setTokenSection(tokenSection);

    return true;
}

//parses an "ifdef-like" directive, like #ifdef and #ifndef :)
//# ifdef identifier newline group[opt]
bool Preprocessor::parseIfdefLikeDirective(IfdefLikeDirective *node)
{
    Q_ASSERT(node->toItemComposite());
    const TokenSection tokenSection = readLine();
    const QList<int> cleanedLine = cleanTokenRange(tokenSection);

    if(cleanedLine.count() < 3)
        return false;

    node->setTokenSection(tokenSection);
    node->setIdentifier(TokenList(m_tokenContainer, QList<int>() << cleanedLine.at(2)));
    parseGroup(node);

    return true;
}

//# if constant-expression newline group[opt]
bool Preprocessor::parseIfLikeDirective(IfLikeDirective *node)
{
    //cout << "parse if-like directive" << endl;
    Q_ASSERT(node->toItemComposite());
    TokenSection tokenSection = readLine();
    QList<int> cleanedSection = cleanTokenRange(tokenSection);
    if(cleanedSection.count() < 3)
        return false;

    cleanedSection.removeFirst(); //remove #
    cleanedSection.removeFirst(); //remove if
    cleanedSection.removeLast(); //remove endl;

    const TokenList sectionList(m_tokenContainer, cleanedSection);
    ExpressionBuilder expressionBuilder(sectionList, m_tokenTypeList, m_memoryPool);
    Expression *expr = expressionBuilder.parse();
    node->setTokenSection(tokenSection);
    node->setExpression(expr);

    parseGroup(node);
    return true;
}

/*
   # define identifier                               replacement-list new-line
   # define identifier lparen identifier-list[opt] ) replacement-list new-line
   # define identifier lparen ... )                  replacement-list new-line
   # define identifier lparen identifier-list, ... ) replacement-list new-line
*/
bool Preprocessor::parseDefineDirective(Item *group)
{
    Q_ASSERT(group->toItemComposite());
    const TokenSection line = readLine();
    const QList<int> cleanedLine = cleanTokenRange(line);
    if(cleanedLine.count() < 3)
        return false;

    // get identifier
    const int identifier = cleanedLine.at(2); //skip "#" and "define"

    // check if this is a macro function
    MacroDefinition *macro;
    macro =  createNode<MacroDefinition>(m_memoryPool, group);
    int replacementListStart;
    if(m_tokenContainer.text(cleanedLine.at(3)) == "("
        && !isWhiteSpace(cleanedLine.at(3) - 1)) {
        //TODO: Handle macro function definition here
        replacementListStart = 3;
    } else {

        replacementListStart = 3;
    }

    const int skipNewLine = 1;
    QList<int> replacementList = cleanedLine.mid(
            replacementListStart, cleanedLine.count() - replacementListStart - skipNewLine );

    macro->setTokenSection(line);
    macro->setIdentifier(TokenList(m_tokenContainer, QList<int>() << identifier));
    macro->setReplacementList(TokenList(m_tokenContainer, replacementList));
    group->toItemComposite()->add(macro);
    return true;
}


// # undef identifier newline
bool Preprocessor::parseUndefDirective(Item *group)
{
    Q_ASSERT(group->toItemComposite());
    const TokenSection tokenSection = readLine();
    const QList<int> cleanedLine = cleanTokenRange(tokenSection);

    if(cleanedLine.count() < 3)
        return false;

    UndefDirective *undefDirective = createNode<UndefDirective>(m_memoryPool, group);
    group->toItemComposite()->add(undefDirective);
    undefDirective->setIdentifier(TokenList(m_tokenContainer, QList<int>() << cleanedLine.at(2)));
    undefDirective->setTokenSection(tokenSection);
    return true;
}

//include pp-tokens new-line
bool Preprocessor::parseIncludeDirective(Item *group)
{
  //  cout << "parseIncludeDirective" << endl;
    Q_ASSERT(group->toItemComposite());
    TokenSection tokenSection = readLine();
    if(tokenSection.count() == 0)
        return false;

    IncludeDirective *includeDirective =  createNode<IncludeDirective>(m_memoryPool, group);
    group->toItemComposite()->add(includeDirective);
    includeDirective->setTokenSection(tokenSection);

    //remove whitepspace and comment tokens
    TokenList tokenList(m_tokenContainer, cleanTokenRange(tokenSection));

    //iterate through the tokens, look for a string literal or a '<'.
    int tokenIndex = 0;
    const int endIndex = tokenList.count();
    while (tokenIndex < endIndex) {
        if(m_tokenTypeList.at(tokenList.containerIndex(tokenIndex)) == Token_string_literal) {
            QByteArray tokenText = tokenList.text(tokenIndex);
            includeDirective->setFilename(tokenText.mid(1, tokenText.size() -2)); //remove quotes
            includeDirective->setIncludeType(IncludeDirective::QuoteInclude);
            break;
        } else if(tokenList.text(tokenIndex) == "<") {
            // We found a <, all following tokens until we read a
            // > is a part of the file anme
            QByteArray filename;
            ++tokenIndex; //skip '<'
            includeDirective->setFilenameToken(tokenIndex);
            while(tokenIndex < endIndex) {
                const QByteArray tokenText = tokenList.text(tokenIndex);
                if(tokenText == ">")
                    break;
                filename += tokenText;
                ++tokenIndex;
            }
            if(tokenIndex < endIndex) {
                includeDirective->setFilename(filename);
                includeDirective->setIncludeType(IncludeDirective::AngleBracketInclude);
            }
            break;
        }
        ++tokenIndex;
    }

    return true;
}

//# error pp-tokens[opt] new-line
bool Preprocessor::parseErrorDirective(Item *group)
{
    Q_ASSERT(group->toItemComposite());
    TokenSection tokenSection = readLine();
    if(tokenSection.count() == 0)
        return false;

    ErrorDirective *errorDirective = createNode<ErrorDirective>(m_memoryPool, group);
    group->toItemComposite()->add(errorDirective);
    errorDirective->setTokenSection(tokenSection);
    return true;
}

//# pragma pp-tokens[opt] new-line
bool Preprocessor::parsePragmaDirective(Item *group)
{
    Q_ASSERT(group->toItemComposite());
    TokenSection tokenSection = readLine();
    if(tokenSection.count() == 0)
        return false;

    PragmaDirective *pragmaDirective = createNode<PragmaDirective>(m_memoryPool, group);
    group->toItemComposite()->add(pragmaDirective);
    pragmaDirective->setTokenSection(tokenSection);
    return true;
}

TokenSection Preprocessor::readLine()
{
    const int startIndex = lexerTokenIndex;
    bool gotNewline = false;

    while(isValidIndex(lexerTokenIndex) && !gotNewline) {
        if(m_tokenTypeList.at(lexerTokenIndex) == Token_newline) {

            if (lexerTokenIndex == 0 || m_tokenTypeList.at(lexerTokenIndex-1) != '\\') {
                gotNewline = true;
                break;
            }
        }
        ++lexerTokenIndex;
    }

    if(gotNewline)
        ++lexerTokenIndex; //include newline
    else
        emit error("Error", "Unexprected end of source");

    return TokenSection(m_tokenContainer, startIndex, lexerTokenIndex - startIndex);
}

inline bool Preprocessor::isValidIndex(const int index) const
{
    return  (index < m_tokenContainer.count());
}

inline bool Preprocessor::isWhiteSpace(const int index) const
{
    return (m_tokenTypeList.at(index) == Token_whitespaces);
}

Type Preprocessor::lookAhead() const
{
    const int index = skipWhiteSpaceAndComments();
    if (index == -1)
        return Token_eof;
    return m_tokenTypeList.at(index);
}

Type Preprocessor::lookAheadSkipHash() const
{
     const int index = skipWhiteSpaceCommentsHash();
     if (index == -1)
        return Token_eof;
    return m_tokenTypeList.at(index);
}

inline int Preprocessor::skipWhiteSpaceAndComments() const
{
    int index = lexerTokenIndex;
    if(!isValidIndex(index))
           return -1;
    while(m_tokenTypeList.at(index) == Token_whitespaces
             || m_tokenTypeList.at(index) == Token_comment
             || m_tokenTypeList.at(index) == Token_line_comment
             || m_tokenTypeList.at(index) == Token_multiline_comment ) {
       ++index;
       if(!isValidIndex(index))
           return -1;
    }
    return index;
}

inline int Preprocessor::skipWhiteSpaceCommentsHash() const
{
    int index = lexerTokenIndex;
    if(!isValidIndex(index))
           return -1;
    while(m_tokenTypeList.at(index) == Token_whitespaces
             || m_tokenTypeList.at(index) == Token_comment
             || m_tokenTypeList.at(index) == Token_line_comment
             || m_tokenTypeList.at(index) == Token_multiline_comment
             || m_tokenTypeList.at(index) == Token_preproc ) {
       ++index;
       if(!isValidIndex(index))
           return -1;
    }
    return index;
}

QList<int> Preprocessor::cleanTokenRange(const TokenSection &tokenSection) const
{
    QList<int> indexList;

    int t = 0;
    const int numTokens = tokenSection.count();
    while (t < numTokens) {
        const int containerIndex = tokenSection.containerIndex(t);
        const Type tokenType = m_tokenTypeList.at(containerIndex);
        const int currentToken = t;
        ++t;

        if(tokenType == Token_whitespaces ||
           tokenType == Token_line_comment ||
           tokenType == Token_multiline_comment )
            continue;

        //handle escaped newlines
        if(tokenSection.text(currentToken) == "\\" &&
           currentToken + 1 < numTokens &&
           tokenSection.text(currentToken+1) == "\n")
            continue;

        indexList.append(containerIndex);
    }
    return indexList;
}

QByteArray visitGetText(Item *item)
{
    QByteArray text;

    text += item->text().fullText();

    if(item->toItemComposite()) {
        ItemComposite *composite = item->toItemComposite();
        for (int i=0; i <composite->count(); ++i)
            text += visitGetText(composite->item(i));
    }

    return text;
}

void Source::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

} // namespace Rpp

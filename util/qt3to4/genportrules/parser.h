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

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>

class FileSymbol;
class TokenStream;
class Error;

class Parser
{
public:
    Parser();
    ~Parser();

    TranslationUnitAST *parse(FileSymbol *file, pool *p);

private:
    bool reportError(const Error& err);
    /** @todo remove*/ bool reportError(const QString& msg);
    /** @todo remove*/ void syntaxError();

public /*rules*/ :

    bool parseTranslationUnit(TranslationUnitAST *&node);

    bool parseDeclaration(DeclarationAST *&node);
    bool parseBlockDeclaration(DeclarationAST *&node);
    bool parseLinkageSpecification(DeclarationAST *&node);
    bool parseLinkageBody(LinkageBodyAST *&node);
    bool parseNamespace(DeclarationAST *&node);
    bool parseNamespaceAliasDefinition(DeclarationAST *&node);
    bool parseUsing(DeclarationAST *&node);
    bool parseUsingDirective(DeclarationAST *&node);
    bool parseTypedef(DeclarationAST *&node);
    bool parseAsmDefinition(DeclarationAST *&node);
    bool parseTemplateDeclaration(DeclarationAST *&node);
    bool parseDeclarationInternal(DeclarationAST *&node);

    bool parseStringLiteral(AST *&node);
    bool parseUnqualifiedName(ClassOrNamespaceNameAST *&node, bool parseTemplateId = true);
    bool parseName(NameAST *&node, bool parseTemplateId = true);
    bool parseOperatorFunctionId(AST *&node);
    bool parseTemplateArgumentList(TemplateArgumentListAST *&node, bool reportError = true);
    bool parseOperator(AST *&node);
    bool parseCvQualify(AST *&node);
    bool parseSimpleTypeSpecifier(TypeSpecifierAST *&node, bool onlyIntegral = false);
    bool parsePtrOperator(AST *&node);
    bool parseTemplateArgument(AST *&node);
    bool parseTypeSpecifier(TypeSpecifierAST *&node);
    bool parseTypeSpecifierOrClassSpec(TypeSpecifierAST *&node);
    bool parseDeclarator(DeclaratorAST *&node);
    bool parseTemplateParameterList(TemplateParameterListAST *&node);
    bool parseTemplateParameter(TemplateParameterAST *&node);
    bool parseStorageClassSpecifier(AST *&node);
    bool parseFunctionSpecifier(AST *&node);
    bool parseInitDeclaratorList(InitDeclaratorListAST *&node);
    bool parseInitDeclarator(InitDeclaratorAST *&node);
    bool parseParameterDeclarationClause(ParameterDeclarationClauseAST *&node);
    bool parseCtorInitializer(AST *&node);
    bool parsePtrToMember(AST *&node);
    bool parseEnumSpecifier(TypeSpecifierAST *&node);
    bool parseClassSpecifier(TypeSpecifierAST *&node);
    bool parseWinDeclSpec(AST *&node);
    bool parseElaboratedTypeSpecifier(TypeSpecifierAST *&node);
    bool parseDeclaratorId(NameAST *&node);
    bool parseExceptionSpecification(AST *&node);
    bool parseEnumerator(EnumeratorAST *&node);
    bool parseTypeParameter(TypeParameterAST *&node);
    bool parseParameterDeclaration(ParameterDeclarationAST *&node);
    bool parseTypeId(TypeIdAST *&node);
    bool parseAbstractDeclarator(DeclaratorAST *&node);
    bool parseParameterDeclarationList(ParameterDeclarationListAST *&node);
    bool parseMemberSpecification(DeclarationAST *&node);
    bool parseAccessSpecifier(AST *&node);
    bool parseTypeIdList(AST *&node);
    bool parseMemInitializerList(AST *&node);
    bool parseMemInitializer(AST *&node);
    bool parseInitializer(AST *&node);
    bool parseBaseClause(BaseClauseAST *&node);
    bool parseBaseSpecifier(BaseSpecifierAST *&node);
    bool parseInitializerClause(AST *&node);
    bool parseMemInitializerId(NameAST *&node);
    bool parseFunctionBody(StatementListAST *&node);

    // expression
    bool skipExpression(AbstractExpressionAST *&node);
    bool skipCommaExpression(AbstractExpressionAST *&node);
    bool skipExpressionStatement(StatementAST *&node);

    bool parseExpression(AbstractExpressionAST *&node);
    bool parsePrimaryExpression(AbstractExpressionAST *&node);
    bool parsePostfixExpression(AbstractExpressionAST *&node);
    bool parsePostfixExpressionInternal(AbstractExpressionAST *expr, AbstractExpressionAST *&node);
    bool parseUnaryExpression(AbstractExpressionAST *&node);
    bool parseNewExpression(AbstractExpressionAST *&node);
    bool parseNewTypeId(AbstractExpressionAST *&node);
    bool parseNewDeclarator(AbstractExpressionAST *&node);
    bool parseNewInitializer(AbstractExpressionAST *&node);
    bool parseDeleteExpression(AbstractExpressionAST *&node);
    bool parseCastExpression(AbstractExpressionAST *&node);
    bool parsePmExpression(AbstractExpressionAST *&node);
    bool parseMultiplicativeExpression(AbstractExpressionAST *&node);
    bool parseAdditiveExpression(AbstractExpressionAST *&node);
    bool parseShiftExpression(AbstractExpressionAST *&node);
    bool parseRelationalExpression(AbstractExpressionAST *&node, bool templArgs = false);
    bool parseEqualityExpression(AbstractExpressionAST *&node, bool templArgs = false);
    bool parseAndExpression(AbstractExpressionAST *&node, bool templArgs = false);
    bool parseExclusiveOrExpression(AbstractExpressionAST *&node, bool templArgs = false);
    bool parseInclusiveOrExpression(AbstractExpressionAST *&node, bool templArgs = false);
    bool parseLogicalAndExpression(AbstractExpressionAST *&node, bool templArgs = false);
    bool parseLogicalOrExpression(AbstractExpressionAST *&node, bool templArgs = false);
    bool parseConditionalExpression(AbstractExpressionAST *&node);
    bool parseAssignmentExpression(AbstractExpressionAST *&node);
    bool parseConstantExpression(AbstractExpressionAST *&node);
    bool parseCommaExpression(AbstractExpressionAST *&node);
    bool parseThrowExpression(AbstractExpressionAST *&node);

    // statement
    bool parseCondition(ConditionAST *&node);
    bool parseStatement(StatementAST *&node);
    bool parseWhileStatement(StatementAST *&node);
    bool parseDoStatement(StatementAST *&node);
    bool parseForStatement(StatementAST *&node);
    bool parseCompoundStatement(StatementAST *&node);
    bool parseForInitStatement(StatementAST *&node);
    bool parseIfStatement(StatementAST *&node);
    bool parseSwitchStatement(StatementAST *&node);
    bool parseLabeledStatement(StatementAST *&node);
    bool parseDeclarationStatement(StatementAST *&node);
    bool parseTryBlockStatement(StatementAST *&node);

    // objective c
    bool parseObjcDef(DeclarationAST *&node);
    bool parseObjcClassDef(DeclarationAST *&node);
    bool parseObjcClassDecl(DeclarationAST *&node);
    bool parseObjcProtocolDecl(DeclarationAST *&node);
    bool parseObjcAliasDecl(DeclarationAST *&node);
    bool parseObjcProtocolDef(DeclarationAST *&node);
    bool parseObjcMethodDef(DeclarationAST *&node);

    bool parseIvarDeclList(AST *&node);
    bool parseIvarDecls(AST *&node);
    bool parseIvarDecl(AST *&node);
    bool parseIvars(AST *&node);
    bool parseIvarDeclarator(AST *&node);
    bool parseMethodDecl(AST *&node);
    bool parseUnarySelector(AST *&node);
    bool parseKeywordSelector(AST *&node);
    bool parseSelector(AST *&node);
    bool parseKeywordDecl(AST *&node);
    bool parseReceiver(AST *&node);
    bool parseObjcMessageExpr(AST *&node);
    bool parseMessageArgs(AST *&node);
    bool parseKeywordExpr(AST *&node);
    bool parseKeywordArgList(AST *&node);
    bool parseKeywordArg(AST *&node);
    bool parseReservedWord(AST *&node);
    bool parseMyParms(AST *&node);
    bool parseMyParm(AST *&node);
    bool parseOptParmList(AST *&node);
    bool parseObjcSelectorExpr(AST *&node);
    bool parseSelectorArg(AST *&node);
    bool parseKeywordNameList(AST *&node);
    bool parseKeywordName(AST *&node);
    bool parseObjcEncodeExpr(AST *&node);
    bool parseObjcString(AST *&node);
    bool parseProtocolRefs(AST *&node);
    bool parseIdentifierList(AST *&node);
    bool parseIdentifierColon(AST *&node);
    bool parseObjcProtocolExpr(AST *&node);
    bool parseObjcOpenBracketExpr(AST *&node);
    bool parseObjcCloseBracket(AST *&node);

    bool skipUntil(int token);
    bool skipUntilDeclaration();
    bool skipUntilStatement();
    bool skip(int l, int r);

    void advance();

private:
    int m_problems;
    int m_maxProblems;
    bool objcp;
    TokenStream *tokenStream;
    pool *m_pool;
    FileSymbol *m_file;

private:
    Parser(const Parser& source);
    void operator = (const Parser& source);
};


#endif

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
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <QObject>
#include <QStack>
#include <QList>
#include <QByteArray>

#include "treewalker.h"
#include "codemodel.h"
#include "tokenstreamadapter.h"

class TokenStream;
class QByteArray;

class Semantic: public QObject, public TreeWalker
{
Q_OBJECT
public:
    Semantic();
    virtual ~Semantic();

    CodeModel::NamespaceScope *parseTranslationUnit( TranslationUnitAST *node,
            TokenStreamAdapter::TokenStream *tokenStream, TypedPool<CodeModel::Item> *storage);
signals:
    void error(const QByteArray &message);
    void warning(const QByteArray &message);
protected:
    virtual void parseNamespace(NamespaceAST *);
    virtual void parseClassSpecifier(ClassSpecifierAST *);

    virtual void parseSimpleDeclaration(SimpleDeclarationAST *);
    virtual void parseDeclaration(AST *funSpec, AST *storageSpec, TypeSpecifierAST *typeSpec, InitDeclaratorAST *decl);
    virtual void parseFunctionDeclaration(AST *funSpec, AST *storageSpec, TypeSpecifierAST *typeSpec, InitDeclaratorAST *decl);
    virtual void parseFunctionArguments(const DeclaratorAST *declarator, CodeModel::FunctionMember *method);

    virtual void parseFunctionDefinition(FunctionDefinitionAST *);
    virtual void parseStatementList(StatementListAST *);

    virtual void parseBaseClause(BaseClauseAST *baseClause, CodeModel::ClassScope * klass);

    virtual void parseLinkageSpecification(LinkageSpecificationAST *);
    virtual void parseUsing(UsingAST *);
    virtual void parseUsingDirective(UsingDirectiveAST *);

    virtual void parseExpression(AbstractExpressionAST*);
    virtual void parseExpressionStatement(ExpressionStatementAST *node);
    virtual void parseClassMemberAccess(ClassMemberAccessAST *node);

    virtual void parseNameUse(NameAST*);

    virtual void parseEnumSpecifier(EnumSpecifierAST *);
    virtual void parseTypedef(TypedefAST *);

    virtual void parseTypeSpecifier(TypeSpecifierAST *);
    CodeModel::Scope *lookupScope(CodeModel::Scope *baseScope, const NameAST* name);

    QList<CodeModel::Member *> nameLookup(CodeModel::Scope *baseScope, const NameAST* name);
    QList<CodeModel::Member *> unqualifiedNameLookup(CodeModel::Scope *baseScope, const NameAST* name);
    QList<CodeModel::Member *> qualifiedNameLookup(CodeModel::Scope *baseScope, const NameAST* name);
    QList<CodeModel::Member *> lookupNameInScope(CodeModel::Scope *scope, const NameAST* name);
    CodeModel::TypeMember *typeLookup(CodeModel::Scope *baseScope, const NameAST* name);

    CodeModel::FunctionMember *functionLookup(CodeModel::Scope *baseScope,  const DeclaratorAST *functionDeclarator);
    CodeModel::FunctionMember *selectFunction(QList<CodeModel::Member*> candidatateList, const DeclaratorAST *functionDeclarator);
private:
    CodeModel::NamespaceScope *findOrInsertNamespace(NamespaceAST *ast, const QByteArray &name);
    QByteArray typeOfDeclaration(TypeSpecifierAST *typeSpec, DeclaratorAST *declarator);
    QList<QByteArray> scopeOfName(NameAST *id, const QList<QByteArray> &scope);
    QList<QByteArray> scopeOfDeclarator(DeclaratorAST *d, const QList<QByteArray> &scope);

    QByteArray declaratorToString(DeclaratorAST* declarator, const QByteArray& scope = QByteArray(), bool skipPtrOp = false);
    QByteArray typeSpecToString(TypeSpecifierAST* typeSpec);
    QByteArray textOf(AST *node) const;
    void createNameUse(CodeModel::Member *member, NameAST *name);
    void addNameUse(AST *node, CodeModel::NameUse *nameUse);
    CodeModel::NameUse *findNameUse(AST *node);
private:
    TokenStreamAdapter::TokenStream *m_tokenStream;
    TypedPool<CodeModel::Item> *m_storage;

    QList<QList<QByteArray> > m_imports;
    CodeModel::Member::Access m_currentAccess;
    bool m_inSlots;
    bool m_inSignals;
    int m_anon;
    bool m_inStorageSpec;
    bool m_inTypedef;

    QMap<int, CodeModel::NameUse *> m_nameUses;
    QStack<CodeModel::Scope *> currentScope;
private:
    Semantic(const Semantic &source);
    void operator = (const Semantic &source);
};

#endif // SEMANTIC_H

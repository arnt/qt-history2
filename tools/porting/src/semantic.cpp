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
#include <QDebug>
#include <QString>
#include <QRegExp>

#include "semantic.h"
#include "smallobject.h"
#include "tokenengine.h"

using namespace TokenStreamAdapter;
using namespace TokenEngine;
using namespace CodeModel;
using namespace std;

// ### FIXME!! typeSpecToString is invoked only for sub-declarators?!!?
QByteArray Semantic::typeSpecToString(TypeSpecifierAST* typeSpec)
{
    if (!typeSpec)
        return QByteArray();

    QByteArray tp;
    if (typeSpec->cvQualify()) {
        tp += "const ";
    }

    tp += (QString(textOf(typeSpec)).replace(QRegExp(" :: "), QString::fromUtf8("::"))).toLatin1();
    return tp;
}

QByteArray Semantic::declaratorToString(DeclaratorAST* declarator, const QByteArray& scope, bool skipPtrOp)
{
    if (!declarator)
        return QByteArray();

    QByteArray text;

    if (!skipPtrOp && declarator->ptrOpList()){
        List<AST*> ptrOpList = *declarator->ptrOpList();
        foreach (AST *current, ptrOpList) {
            text += textOf(current);
        }
        text += " ";
    }

    text += scope;

    if (declarator->subDeclarator())
        text += "(" + declaratorToString(declarator->subDeclarator()) + ")";

    if (declarator->declaratorId())
        text += textOf(declarator->declaratorId());

    if (declarator->arrayDimensionList()) {
        List<AST*> arrays = *declarator->arrayDimensionList();
        foreach (AST *current, arrays) {
            current=current;    //silence unused symbol warning
            text += "[]";
        }
    }

    if (declarator->parameterDeclarationClause()){
        text += "(";

        ParameterDeclarationListAST* l = declarator->parameterDeclarationClause()->parameterDeclarationList();
        if (l != 0){
            List<ParameterDeclarationAST*> params = *l->parameterList();
            foreach (ParameterDeclarationAST *current, params) {
                QByteArray type = typeSpecToString(current->typeSpec());
                text += type;
                if (!type.isEmpty())
                    text += " ";
                text += declaratorToString(current->declarator());

                // ### FIXME if (it.current())
                    text += ", ";
            }
        }

        text += ")";

        if (declarator->constant() != 0)
            text += " const";
    }

    return QString(text).replace(QRegExp(" :: "), "::").simplified().toLatin1();
}

/*
    looks up name used in basescope. If name->isGlobal() is true or if classOrNamespaceList()
    returns a non-emty list, the C++ qualified name lookup rules are used. Otherwise the
    unquialified name lookup rules are used.  Returns the a list of members that was found,
    In most cases this list will contain zero or one element, exept in the case of overloaded functions.
    TODO: Argument-dependent name lookup
*/
QList<CodeModel::Member *> Semantic::nameLookup(CodeModel::Scope *baseScope, const NameAST* name)
{
    if (name->isGlobal() || (name->classOrNamespaceNameList()
                              && name->classOrNamespaceNameList()->size()>0 )) {
        return qualifiedNameLookup(baseScope, name);
    } else {
        return unqualifiedNameLookup(baseScope, name);
    }
}

//look up an unqualified name
QList<CodeModel::Member *> Semantic::unqualifiedNameLookup(CodeModel::Scope *baseScope, const NameAST* name)
{
    //TODO: handle using-directives
    CodeModel::Scope *currentScope = baseScope;
    QList<CodeModel::Member *>  entities;
    while (currentScope != 0 && entities.isEmpty()) {
        entities = lookupNameInScope(currentScope, name);
        currentScope = currentScope->parent();
    }
    return entities;
}

//look up a qualified name
QList<CodeModel::Member *> Semantic::qualifiedNameLookup(CodeModel::Scope *baseScope, const NameAST* name)
{
    //TODO: handle using-directives
    QList<CodeModel::Member *> entities;
    CodeModel::Scope *currentScope = baseScope;

    if(name->isGlobal()) {
        while (currentScope->parent())
            currentScope = currentScope->parent();
    }

    while (entities.isEmpty() && currentScope != 0) {
        CodeModel::Scope *targetScope = lookupScope(currentScope, name);
        entities = lookupNameInScope(targetScope, name);
        currentScope = currentScope->parent();
    }

    return entities;
}

//looks up a name in a scope, includes base classes if scope is a class scope
QList<CodeModel::Member *> Semantic::lookupNameInScope(CodeModel::Scope *scope, const NameAST* name)
{
    QList<CodeModel::Member *> entities;

    if(!scope || !name)
        return entities;

    QByteArray nameText = textOf(name->unqualifiedName()->name());


    //look up name in members of current scope
    CodeModel::MemberCollection *members = scope->members();
    if(members)
    for (int i=0; i<members->count(); ++i) {
        CodeModel::Member *member = members->item(i);
        Q_ASSERT(member);
        if (member->name() == nameText)
            entities.append(member);
    }

    // if not found, look up name in  base classes (if any)
    CodeModel::ClassScope *classScope = scope->toClassScope();
    if(entities.isEmpty() && classScope )  {
        CodeModel::TypeCollection *baseClasses = classScope->baseClasses();
        if(baseClasses) {
            for(int i=0; i<baseClasses->count(); ++i) {
                CodeModel::Type *baseClass = baseClasses->item(i);
                Q_ASSERT(baseClass);
                if(!baseClass->toClassType()) {
                    emit error("Error in Semantic::lookupNameInScope: base class"
                               + baseClass->name() + " undeclared");
                    break;
                }
                entities += lookupNameInScope(baseClass->toClassType()->scope(), name);
            }
        }
        if(entities.count()>1)
            emit error("Error in Semantic::lookupNameInScope: name "
            + nameText + " is ambigous");
    }
    return entities;
}

CodeModel::Scope *Semantic::lookupScope(CodeModel::Scope *baseScope, const NameAST* name)
{
    CodeModel::Scope *currentScope = baseScope;

    List<ClassOrNamespaceNameAST *> *namespaceList = name->classOrNamespaceNameList();
    if(!namespaceList)
        return currentScope;

    CodeModel::Scope *targetScope = 0;
    while(currentScope!=0 && targetScope == 0)
    {
        //look for targetScope
        int nestingCounter = 0;
        CodeModel::Scope *nestedScope = currentScope;

        do {
            CodeModel::ScopeCollection *childScopes = nestedScope->scopes();
            nestedScope = 0;

            for (int i=0; i<childScopes->count(); ++i) {
                CodeModel::Scope *childScope = childScopes->item(i);
                Q_ASSERT(childScope);

                if (childScope->name() == textOf((*namespaceList)[nestingCounter]->name())) {
                    nestedScope = childScope;
                    break;
                }
            }
            ++nestingCounter;
        } while (nestedScope != 0 && nestingCounter <  namespaceList->count());

        targetScope =  nestedScope;
        if(!targetScope)
            currentScope = currentScope->parent();
    }

    return targetScope;
}

TypeMember *Semantic::typeLookup(CodeModel::Scope *baseScope, const NameAST* name)
{
    QList<CodeModel::Member *> memberList = nameLookup(baseScope, name);

    foreach(Member *member, memberList) {
        if(TypeMember *typeMember = member->toTypeMember())
            return typeMember;
    }
    return 0;
}

FunctionMember *Semantic::functionLookup(CodeModel::Scope *baseScope,
                                          const DeclaratorAST *functionDeclarator)
{

    QList<CodeModel::Member*> candidateList =
                nameLookup(baseScope, functionDeclarator->declaratorId());
    return selectFunction(candidateList, functionDeclarator);
}

/*
    This is a simplified function lookup routine, for matching member function
    definitions with member function declarations. It does not implement
    the general C++ function overload resolution rules.
*/
FunctionMember *Semantic::selectFunction(QList<CodeModel::Member*> candidatateList, const DeclaratorAST *functionDeclarator)
{
    // get arguments for funciton we are looking for
    FunctionMember testFunction;
    parseFunctionArguments(functionDeclarator, &testFunction);
    ArgumentCollection *testArgumentCollection = testFunction.arguments();
    if(!testArgumentCollection)
        return 0;

    //test againts functions in overload list.
    foreach(Member* member, candidatateList) {
        FunctionMember *function = member->toFunctionMember();
        if (!function)
            continue;
        ArgumentCollection *argumentCollection = function->arguments();
        if(!argumentCollection)
            continue;
        //test number of arguments
        if(testArgumentCollection->count() != argumentCollection->count())
            continue;
        //test argument types
        bool mismatch = false;
        for(int a = 0; a < testArgumentCollection->count(); ++a) {
            //cout <<testArgumentCollection->item(a)->type()->name().constData()
            // << " | " << argumentCollection->item(a)->type()->name().constData() << endl;
            // So.. here we get different type objects with the same name.. hm
            if (testArgumentCollection->item(a)->type()->name() != argumentCollection->item(a)->type()->name())
                mismatch = true;
        }
        if(!mismatch)
            return function;
    }
    return 0;
}



/// =======================================================

Semantic::Semantic()
{

}

Semantic::~Semantic()
{
}

QByteArray Semantic::textOf(AST *node) const
{
    if (!node)
        return QByteArray();
    QByteArray text;
    for (int i = node->startToken(); i < node->endToken(); ++i) {
        if (!m_tokenStream->isHidden(i)) {
            if (i != node->startToken())
                text += " ";
            text += m_tokenStream->tokenText(i);
        }
    }
    return text;
}

CodeModel::NamespaceScope *Semantic::parseTranslationUnit(TranslationUnitAST *node,
            TokenStreamAdapter::TokenStream *tokenStream, TypedPool<CodeModel::Item> *storage)
{
    m_storage = storage;
    m_tokenStream =tokenStream;
    m_currentAccess = CodeModel::Member::Public;
    m_inSlots = false;
    m_inSignals = false;
    m_inStorageSpec = false;
    m_inTypedef = false;
    m_anon = 0;
    m_imports.clear();
    m_nameUses.clear();

    NamespaceScope *globalScope = CodeModel::Create<NamespaceScope>(m_storage);
    globalScope->setName("::");
    currentScope.push(globalScope);

    m_imports << QList<QByteArray>();
    TreeWalker::parseTranslationUnit(node);
    m_imports.pop_back();
    return globalScope;
}


void Semantic::parseLinkageSpecification(LinkageSpecificationAST *ast)
{
    if(!ast)
        return;
    int inStorageSpec = m_inStorageSpec;
    m_inStorageSpec = true;
    TreeWalker::parseLinkageSpecification(ast);
    m_inStorageSpec = inStorageSpec;
}


#if 0
void Semantic::parseDeclaration(DeclarationAST *ast)
{
    TreeWalker::parseDeclaration(ast);
}
#endif

void Semantic::parseNamespace(NamespaceAST *ast)
{
    if(currentScope.isEmpty()) {
        emit error("Error in Semantic::parseNamespace: no parent scope");
        return;
    }

    CodeModel::NamespaceScope *parent = currentScope.top()->toNamespaceScope();
    if(!parent->toNamespaceScope()) {
        emit error("Error in Semantic::parseNamespace: parent scope was not a namespace");
        return;
    }

    QByteArray nsName;
    if (!ast->namespaceName() || textOf(ast->namespaceName()).isEmpty()){
        nsName = "(__QT_ANON_NAMESPACE)";
    } else {
        nsName = textOf(ast->namespaceName());
    }

    CodeModel::NamespaceScope *ns = parent->findNamespace(nsName);
    if (!ns) {
        ns = CodeModel::Create<CodeModel::NamespaceScope>(m_storage);
        parent->addScope(ns);
        ns->setName(nsName);
    }

    currentScope.push(ns);
    TreeWalker::parseNamespace(ast);
    currentScope.pop();
}

void Semantic::parseClassSpecifier(ClassSpecifierAST *ast)
{
    if (!ast->name()){
        //emit error("Error in Semantic::parseClassSpecifier: class has no name");
        return;
    }

    if(currentScope.isEmpty()) {
        QByteArray errorText = QByteArray("Error in Semantic::parseClassSpecifier parsing class ")
                            +textOf(ast->name()->unqualifiedName())
                            +QByteArray(": no parent scope");
        emit error(errorText);
        return;
    }

    QByteArray kind = textOf(ast->classKey());
    if (kind == "class")
        m_currentAccess = CodeModel::Member::Private;
    else // kind =="struct"
        m_currentAccess = CodeModel::Member::Public;

    QByteArray className = textOf(ast->name()->unqualifiedName());

    //create ClassScope
    CodeModel::ClassScope *klass = CodeModel::Create<CodeModel::ClassScope>(m_storage);
    klass->setName(className);
    currentScope.top()->addScope(klass);

    //create ClassType
    CodeModel::ClassType *type = CodeModel::Create<CodeModel::ClassType>(m_storage);
    type->setScope(klass);
    currentScope.top()->addType(type);
    type->setParent(currentScope.top());

    //create TypeMember
    CodeModel::TypeMember *typeMember = CodeModel::Create<CodeModel::TypeMember>(m_storage);
    typeMember->setNameAST(ast->name()->unqualifiedName());
    typeMember->setName(className);
    typeMember->setType(type);
    currentScope.top()->addMember(typeMember);
    typeMember->setParent(currentScope.top());

    currentScope.push(klass);
    if (ast->baseClause())
        parseBaseClause(ast->baseClause(), klass);

    //TreeWalker::parseClassSpecifier(ast);
    parseNode(ast->winDeclSpec());
    parseNode(ast->classKey());
    parseNode(ast->baseClause());

    // Here's the trick for parsing c++ classes:
    // All inline function definitions must be interpreted as if they were
    // written after any other declarations in the class.
    QList<DeclarationAST *> functionDefinitions;
    if (ast->declarationList())
        foreach(DeclarationAST *decl, *ast->declarationList()) {
            if(decl->nodeType() == NodeType_FunctionDefinition)
                functionDefinitions.append(decl);
            else
                parseNode(decl);
        }
    foreach(DeclarationAST *decl, functionDefinitions)
        parseNode(decl);

    currentScope.pop();
}


void Semantic::parseSimpleDeclaration(SimpleDeclarationAST *ast)
{
    if(currentScope.isEmpty()) {
        emit error("Error in Semantic::parseSimpleDeclaration: no current Scope");
        return;
    }

    TypeSpecifierAST *typeSpec = ast->typeSpec();
    InitDeclaratorListAST *declarators = ast->initDeclaratorList();

    if (typeSpec)
        parseTypeSpecifier(typeSpec);

    if (declarators){
        List<InitDeclaratorAST*> l = *declarators->initDeclaratorList();

        foreach (InitDeclaratorAST *current, l) {
            parseDeclaration(ast->functionSpecifier(), ast->storageSpecifier(), typeSpec, current);
        }
    }
}

void Semantic::parseDeclaration(AST *funSpec, AST *storageSpec, TypeSpecifierAST *typeSpec, InitDeclaratorAST *decl)
{
    if (m_inStorageSpec)
            return;

    if(!decl)
        return;

    DeclaratorAST *d = decl->declarator();
    if (!d)
        return;

    if (!d->subDeclarator() && d->parameterDeclarationClause())
        return parseFunctionDeclaration(funSpec, storageSpec, typeSpec, decl);

    if(!typeSpec || !typeSpec->name())
        return;

    DeclaratorAST *t = d;
    while (t && t->subDeclarator())
        t = t->subDeclarator();

    QByteArray id;
    if (t && t->declaratorId() && t->declaratorId()->unqualifiedName())
        id = textOf(t->declaratorId()->unqualifiedName());

    if (!t || !t->declaratorId() || !t->declaratorId()->unqualifiedName())
        return;
    AST *nameAST = t->declaratorId()->unqualifiedName();
    QByteArray name = textOf(nameAST);


    if (!scopeOfDeclarator(d, QList<QByteArray>()).isEmpty()){
        return;
    }

    //create VariableMember
    CodeModel::VariableMember *variableMember = CodeModel::Create<CodeModel::VariableMember>(m_storage);
    variableMember->setNameAST(nameAST);
    variableMember->setName(name);
    variableMember->setAccess(m_currentAccess);
    variableMember->setParent(currentScope.top());
    currentScope.top()->addMember(variableMember);

    //look up type of variableMember,

    TypeMember *typeMember = typeLookup(currentScope.top(), typeSpec->name());
    if(typeMember) {
        variableMember->setType(typeMember->type());
  //      createNameUse(typeMember, typeSpec->name());
    } else {
        QByteArray text = typeOfDeclaration(typeSpec, d);
        CodeModel::UnknownType *type = CodeModel::Create<CodeModel::UnknownType>(m_storage);
        type->setName(text);
        variableMember->setType(type);
    }

    if (decl)
        parseNode(decl->initializer());

}

void Semantic::parseFunctionDeclaration(AST *funSpec, AST *storageSpec,
                                        TypeSpecifierAST * typeSpec, InitDeclaratorAST * initDeclarator)
{
    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;
    bool isPure = initDeclarator->initializer() != 0;

    if (funSpec){
        List<AST*> l = *funSpec->children();
        foreach (AST *current, l) {
            QByteArray text = textOf(current);
            if (text == "virtual") isVirtual = true;
            else if (text == "inline") isInline = true;
        }
    }

    if (storageSpec){
        List<AST*> l = *storageSpec->children();
        foreach (AST *current, l) {
            QByteArray text = textOf(current);
            if (text == "friend") isFriend = true;
            else if (text == "static") isStatic = true;
        }
    }
    DeclaratorAST *declarator = initDeclarator->declarator();
    if(!declarator || !declarator->declaratorId())
        return;
    AST *nameAST = declarator->declaratorId()->unqualifiedName();
    QByteArray name = textOf(nameAST);

    CodeModel::FunctionMember *method = CodeModel::Create<CodeModel::FunctionMember>(m_storage);
    method->setNameAST(nameAST);
    method->setName(name);
    method->setAccess(m_currentAccess);
    method->setStatic(isStatic);
    method->setVirtual(isVirtual);
    method->setAbstract(isPure);

    parseFunctionArguments(declarator, method);

    if (m_inSignals)
        method->setSignal(true);

    if (m_inSlots)
        method->setSlot(true);

    method->setConstant(declarator->constant() != 0);

    QByteArray text = typeOfDeclaration(typeSpec, declarator);
    if (!text.isEmpty()) {
        CodeModel::UnknownType *type = CodeModel::Create<CodeModel::UnknownType>(m_storage);
        type->setName(text);
        method->setReturnType(type);
    }

    method->setParent(currentScope.top());
    currentScope.top()->addMember(method);
}


void Semantic::parseBaseClause(BaseClauseAST * baseClause, CodeModel::ClassScope *klass)
{
    if(!baseClause)
        return;
    if(!klass)
        return;
    List<BaseSpecifierAST*> l = *baseClause->baseSpecifierList();
    foreach (BaseSpecifierAST *baseSpecifier, l) {
        QByteArray baseName;
        if (baseSpecifier->name())
            baseName = textOf(baseSpecifier->name());

        QList<CodeModel::Scope*> candidates = klass->findScope(baseName);
        if (!candidates.isEmpty()) {
            CodeModel::ClassType *type = CodeModel::Create<CodeModel::ClassType>(m_storage);
            type->setScope(candidates.first()->toClassScope());
            klass->addBaseClass(type);
        } else {
            CodeModel::UnknownType *type = CodeModel::Create<CodeModel::UnknownType>(m_storage);
            type->setName(baseName);
            klass->addBaseClass(type);
        }
    }
}
void Semantic::parseFunctionArguments(const DeclaratorAST *declarator, CodeModel::FunctionMember *method)
{
    if(!declarator || !method)
        return;

    ParameterDeclarationClauseAST *clause = declarator->parameterDeclarationClause();

    if (clause && clause->parameterDeclarationList()){
        ParameterDeclarationListAST *params = clause->parameterDeclarationList();
        List<ParameterDeclarationAST*> l = *params->parameterList();
        foreach (ParameterDeclarationAST *param, l) {
            CodeModel::Argument *arg = CodeModel::Create<CodeModel::Argument>(m_storage);
            arg->setParent(method);

            if (param->declarator()){
                QByteArray text = declaratorToString(param->declarator(), QByteArray(), true);
                if(param->declarator()->declaratorId())
                    arg->setNameAST(param->declarator()->declaratorId()->unqualifiedName());
                if (!text.isEmpty())
                    arg->setName(text);
            }

            QByteArray tp = typeOfDeclaration(param->typeSpec(), param->declarator());
            if (!tp.isEmpty()) {
                CodeModel::UnknownType *type = CodeModel::Create<CodeModel::UnknownType>(m_storage);
                type->setName(tp);
                arg->setType(type);
            }

            method->addArgument(arg);
        }
    }
}

QByteArray Semantic::typeOfDeclaration(TypeSpecifierAST *typeSpec, DeclaratorAST *declarator)
{
    if (!typeSpec)
        return QByteArray();

    QByteArray text;

    if (typeSpec->cvQualify()) {
        List<AST*> cv = *typeSpec->cvQualify()->children();
        foreach (AST *current, cv) {
            text += " " + textOf(current);
        }
        text += " ";
    }


    text += textOf(typeSpec);

    if (typeSpec->cv2Qualify()) {
        List<AST*> cv = *typeSpec->cv2Qualify()->children();
        foreach (AST *current, cv) {
            text += textOf(current) + " ";
        }
    }

    if (declarator && declarator->ptrOpList()) {
        List<AST*> ptrOpList = *declarator->ptrOpList();
        foreach (AST *current, ptrOpList) {
            text += " " + textOf(current);
        }
        text += " ";
    }

    return text.trimmed().simplified();
}



QList<QByteArray> Semantic::scopeOfName(NameAST *id, const QList<QByteArray>& startScope)
{
    QList<QByteArray> scope = startScope;
    if (id && id->classOrNamespaceNameList()){
        if (id->isGlobal())
            scope.clear();

        List<ClassOrNamespaceNameAST*> l = *id->classOrNamespaceNameList();
        foreach (ClassOrNamespaceNameAST *current, l) {
            if (current->name())
               scope << textOf(current->name());
        }
    }

    return scope;
}

QList<QByteArray> Semantic::scopeOfDeclarator(DeclaratorAST *d, const QList<QByteArray>& startScope)
{
    if(!d)
        return QList<QByteArray>();
    return scopeOfName(d->declaratorId(), startScope);
}

void Semantic::parseUsing(UsingAST *ast)
{
    if(currentScope.isEmpty()) {
        emit error("Error in Semantic::parseUsing: no current Scope");
        return;
    }

    //CodeModel::Scope *s = lookUpScope(currentScope.top(), ast->name());
    QList<CodeModel::Member *> members = nameLookup(currentScope.top(), ast->name());
    if(members.isEmpty()) {
        emit error("Error in Semantic::parseUsing: could not look up using target");
        return;
    }
    //TODO: handle multiple members (when nameLookup returns a set of overloded functions)
    CodeModel::Member *member = members[0];
    CodeModel::Scope *targetScope = member->parent();
    if(!targetScope) {
        emit error("Error in Semantic::parseUsing: target has no parent scope");
        return;
    }

    if(!ast->name())
        return;
    AST *nameAST = ast->name()->unqualifiedName();
    if(!nameAST)
        return;
    QByteArray name = textOf(nameAST);
    CodeModel::UsingDirectiveMember *usingMember = CodeModel::Create<CodeModel::UsingDirectiveMember>(m_storage);
    usingMember->setNameAST(nameAST);
    usingMember->setName(name);
    usingMember->setParent(currentScope.top());
    usingMember->setTargetScope(targetScope);
    currentScope.top()->addMember(usingMember);
}

void Semantic::parseUsingDirective(UsingDirectiveAST *ast)
{
  //  cout << "parse using directive" << endl;

    QByteArray name = textOf(ast->name()->unqualifiedName());
 //   cout << "I'm going to use: " << name.constData() << endl;
}



void Semantic::parseFunctionDefinition(FunctionDefinitionAST *ast)
{
    if(!currentScope.top()){
        emit error("Error in Semantic::parseFunctionDefinition: No current scope");
        return;
    }

    AST *funSpec = ast->functionSpecifier();
    AST *storageSpec = ast->storageSpecifier();
    TypeSpecifierAST *typeSpec = ast->typeSpec();
    InitDeclaratorAST *initDeclarator = ast->initDeclarator();
    if (!ast->initDeclarator())
        return;

    DeclaratorAST *d = initDeclarator->declarator();

    if (!d->declaratorId())
        return;


    // Check if function already has been declared, if not then this is also a declaration.
    CodeModel::FunctionMember *method = functionLookup(currentScope.top(), d);
    if (!method)
        parseFunctionDeclaration(funSpec, storageSpec, typeSpec, initDeclarator);
    method = functionLookup(currentScope.top(), d);

    if(!method) {
        emit error("Error in Semantic::parseFunctionDefinition: Could not find declaration for function definition");
        return;
    }

    CodeModel::Scope *parent = method->parent();

    if(!ast->functionBody()) {
        emit error("Error in Semantic::parseFunctionDefinition: no function body in function definition");
        return;
    }

    //create child function scope
    QByteArray id = textOf(d->declaratorId()->unqualifiedName());
    CodeModel::BlockScope *functionScope = CodeModel::Create<CodeModel::BlockScope>(m_storage);
    functionScope->setName(QByteArray("__QT_ANON_BLOCK_SCOPE(Function: ") + id + QByteArray(")"));
    functionScope->setParent(parent);
    method->setFunctionBodyScope(functionScope);

    //add arguments to child scope
     CodeModel::ArgumentCollection *arguments = method->arguments();
     for(int i=0; i<arguments->count(); ++i) {
         CodeModel::Argument *argument = arguments->item(i);
         CodeModel::VariableMember *variableMember = CodeModel::Create<CodeModel::VariableMember>(m_storage);
         variableMember->setNameAST(argument->nameAST());
         variableMember->setType(argument->type());
         variableMember->setName(argument->name());
         variableMember->setParent(functionScope);
         functionScope->addMember(variableMember);
     }

    //push function scope and parse function body
    currentScope.push(functionScope);
    parseStatementList(ast->functionBody());
    currentScope.pop();
}

void Semantic::parseStatementList(StatementListAST *list)
{
    if(!list)
        return;
    CodeModel::BlockScope *blockScope = CodeModel::Create<CodeModel::BlockScope>(m_storage);
    blockScope->setName("__QT_ANON_BLOCK_SCOPE");
    blockScope->setParent(currentScope.top());
    currentScope.top()->addScope(blockScope);

    currentScope.push(blockScope);
    TreeWalker::parseStatementList(list);
    currentScope.pop();
}

void Semantic::parseExpression(AbstractExpressionAST* node)
{
    if(!node)
        return;
    if(node->nodeType() == NodeType_ClassMemberAccess)
        parseClassMemberAccess(static_cast<ClassMemberAccessAST *>(node));
    else
        TreeWalker::parseExpression(node);
}

/*
    Pretty hardwired code for handling class member access of the types:
    object.member and objectPtr->member.

    This function creates a name use for object to its declaration, and a
    name use from member to its declaration in the class.
*/
void Semantic::parseClassMemberAccess(ClassMemberAccessAST *node)
{
    if(!node)
        return;
    parseExpression(node->expression());
    NameUse *nameUse = findNameUse(node->expression());
    if(!nameUse)
        return;
    if(    nameUse
        && nameUse->declaration()
        && nameUse->declaration()->toVariableMember()
        && nameUse->declaration()->toVariableMember()->type()
        && nameUse->declaration()->toVariableMember()->type()->toClassType()
        && nameUse->declaration()->toVariableMember()->type()->toClassType()->scope())   {

        CodeModel::Scope *scope = nameUse->declaration()->toVariableMember()->type()->toClassType()->scope();
        QList<CodeModel::Member *> members = lookupNameInScope(scope, node->name());
            if(members.count() != 0)
                createNameUse(members.at(0), node->name());
    }
}

void Semantic::parseExpressionStatement(ExpressionStatementAST *node)
{
    TreeWalker::parseExpressionStatement(node);
}


/*
    Parses a name: looks up name, creates name use.
*/
void Semantic::parseNameUse(NameAST* name)
{
    if(!name)
        return;
    if(currentScope.isEmpty()) {
        emit error("Error in Semantic::parseUsing: no current Scope");
        return;
    }

    // Look up name
    QList<CodeModel::Member *> members = nameLookup(currentScope.top(), name);
    if(members.isEmpty()) {
        emit error("Error in Semantic::parseUsing: could not look up using target");
        return;
    }
    //TODO: handle multiple members (when nameLookup returns a set of overloaded functions)
    CodeModel::Member *member = members[0];
    if(!member->parent()) {
        emit error("Error in Semantic::parseUsing: target has no parent scope");
        return;
    }

    createNameUse(member, name);
}

void Semantic::createNameUse(Member *member, NameAST *name)
{
    AST *unqualifedName = name->unqualifiedName()->name();

    if(!unqualifedName || !member || !name)
        return;

    CodeModel::NameUse *nameUse = CodeModel::Create<CodeModel::NameUse>(m_storage);
    nameUse->setParent(currentScope.top());
    nameUse->setNameAST(unqualifedName);
    nameUse->setName(textOf(unqualifedName));
    nameUse->setDeclaration(member);

    currentScope.top()->addNameUse(nameUse);
    addNameUse(unqualifedName, nameUse);
}

void Semantic::addNameUse(AST *node, NameUse *nameUse)
{
    const int tokenIndex = node->startToken();
    m_nameUses.insert(tokenIndex, nameUse);
}


NameUse *Semantic::findNameUse(AST *node)
{
    if(!node)
        return 0;

    List<AST*> *children = node->children();
    if(children) {
        NameUse *nameUse = 0;
        foreach(AST* child , *children) {
            nameUse = findNameUse(child);
            if(nameUse)
                break;
        }
        if (nameUse)
            return nameUse;
    }

    for (int t = node->startToken(); t < node->endToken(); ++t) {
 //       cout << t <<" |" <<m_tokenStream->tokenText(t).constData() << "|" << endl;
        if (m_nameUses.contains(t))
            return m_nameUses.value(t);
    }
    return 0;
}


void Semantic::parseEnumSpecifier(EnumSpecifierAST *ast)
{
    if(!currentScope.top()){
        emit error("Error in Semantic::parseEnumSpecifier: No current scope");
        return;
    }

    if (!ast->name())
         return;

    QByteArray name = textOf(ast->name());

    //create a Type
    CodeModel::EnumType *type = CodeModel::Create<CodeModel::EnumType>(m_storage);
    type->setName(name);
    currentScope.top()->addType(type);
    type->setParent(currentScope.top());



    //create a TypeMember
    CodeModel::TypeMember *typeMember = CodeModel::Create<CodeModel::TypeMember>(m_storage);
    if(ast->name())
        typeMember->setNameAST(ast->name()->unqualifiedName());
    typeMember->setName(name);
    typeMember->setType(type);
    currentScope.top()->addMember(typeMember);
    typeMember->setParent(currentScope.top());

    //parse the eneumerators
    List<EnumeratorAST*> l = *ast->enumeratorList();
    foreach (EnumeratorAST *current, l) {
        CodeModel::VariableMember *attr = CodeModel::Create<CodeModel::VariableMember>(m_storage);
        typeMember->setNameAST(current->id());
        attr->setName(textOf(current->id()));
        attr->setAccess(m_currentAccess);
        attr->setStatic(true);

        attr->setType(&CodeModel::BuiltinType::Int);
        currentScope.top()->addMember(attr);
    }
}

void Semantic::parseTypedef(TypedefAST *ast)
{
    TypeSpecifierAST *typeSpec = ast->typeSpec();
    InitDeclaratorListAST *declarators = ast->initDeclaratorList();

    if (typeSpec && declarators){
        QByteArray typeId;

        if (typeSpec->name())
            typeId = textOf(typeSpec->name());

        List<InitDeclaratorAST*> l = *declarators->initDeclaratorList();
        foreach (InitDeclaratorAST *initDecl, l) {
            QByteArray type, id;
            if (initDecl->declarator()){
               type = typeOfDeclaration(typeSpec, initDecl->declarator());

               DeclaratorAST *d = initDecl->declarator();
               while (d->subDeclarator()){
                   d = d->subDeclarator();
               }

               if (d->declaratorId())
                  id = textOf(d->declaratorId());
            }

            //create a type
            CodeModel::Scope *scope = currentScope.top();
            CodeModel::AliasType *typeAlias = CodeModel::Create<CodeModel::AliasType>(m_storage);
            //typeAlias->setName(id);
            //typeAlias->setParent(scope);
            scope->addType(typeAlias);

            //create a TypeMember
            CodeModel::TypeMember *typeMember = CodeModel::Create<CodeModel::TypeMember>(m_storage);
            if(typeSpec->name())
                typeMember->setNameAST(typeSpec->name()->unqualifiedName());
            typeMember->setName(id);
            typeMember->setType(typeAlias);
            currentScope.top()->addMember(typeMember);
            typeMember->setParent(currentScope.top());

        }

    }
}

void Semantic::parseTypeSpecifier(TypeSpecifierAST *ast)
{
    parseNameUse(ast->name());
    TreeWalker::parseTypeSpecifier(ast);
}


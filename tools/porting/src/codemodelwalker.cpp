#include "codemodelwalker.h"


void CodeModelWalker::parseScope(CodeModel::Scope *scope)
{
    if(!scope) {
        puts("Error in CodeModelWalker: parseScope() called with null pointer");
        return;
    }
    if(scope->toClassScope())
        parseClassScope(scope->toClassScope());
    if(scope->toNamespaceScope())
        parseNamespaceScope(scope->toNamespaceScope());
    if(scope->toBlockScope())
        parseBlockScope(scope->toBlockScope());
/*
    Not used any more, types accosiated with a scope
    is now stored as a TypeMember
    CodeModel::TypeCollection *t = scope->types();
    if(t)
    for (int i=0; i<t->count(); ++i)
        parseType(t->item(i));
*/
    CodeModel::MemberCollection *m = scope->members();
    if(m)
    for (int i=0; i<m->count(); ++i)
        parseMember(m->item(i));

    CodeModel::ScopeCollection *c = scope->scopes();
    if(c)
    for (int i=0; i<c->count(); ++i)
        parseScope(c->item(i));

    CodeModel::NameUseCollection *n = scope->nameUses();
    if(n)
    for (int i=0; i<n->count(); ++i)
        parseNameUse(n->item(i));
}

void CodeModelWalker::parseType(CodeModel::Type *type)
{
    if(!type) {
        puts("Error in CodeModelWalker: parseType() called with null pointer");
        return;
    }
    if (type->toEnumType())
        parseEnumType(type->toEnumType());
    else if (type->toClassType())
        parseClassType(type->toClassType());
    else if (type->toBuiltinType())
        parseBuiltinType(type->toBuiltinType());
    else if (type->toPointerType())
        parsePointerType(type->toPointerType());
    else if (type->toReferenceType())
        parseReferenceType(type->toReferenceType());
    else if (type->toGenericType())
        parseGenericType(type->toGenericType());
    else if (type->toAliasType())
        parseAliasType(type->toAliasType());
    else if (type->toUnknownType())
        parseUnknownType(type->toUnknownType());
}

void CodeModelWalker::parseMember(CodeModel::Member *member)
{
    if(!member) {
        puts("Error in CodeModelWalker: parseMember() called with null pointer");
        return;
    }
    if (member->toFunctionMember())
        parseFunctionMember(member->toFunctionMember());
    else if (member->toVariableMember())
        parseVariableMember(member->toVariableMember());
    else if (member->toUsingDeclarationMember())
        parseUsingDeclarationMember(member->toUsingDeclarationMember());
    else if (member->toUsingDirectiveMember())
        parseUsingDirectiveMember(member->toUsingDirectiveMember());
    else if (member->toTypeMember())
        parseTypeMember(member->toTypeMember());
}

